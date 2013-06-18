//@@COPYRIGHT@@

namespace threadpool {

// A task to be executed.
// FIXME: MemPool
struct task {
	// Function to call to run the task
	std::function<void()> function;

	// Parent task whose reference count is decremented when this one
	// completes.
	task* parent;

	// Reference count. This starts off with a value of 1 and is used
	// to count the number of active children + 1. If a continuation is
	// set then it is decremented and the continuation is run when the
	// value reaches 0.
	std::atomic<int> ref_count;
};

// Initial size for work stealing queue and global queue
static const int INITIAL_JOBQUEUE_SIZE = 32;

// Work-stealing queue
template<typename T> class work_steal_queue {
public:
	work_steal_queue(int length_)
		: length{length_}, head_index{0}, tail_index{0}
	{
		items = new T[length];
	}

	~work_steal_queue()
	{
		delete[] items;
	}

	// Push a job to the tail of this thread's queue
	void push(T job)
	{
		int tail = tail_index.load(std::memory_order_relaxed);

		// Check if we have space to insert an element
		if (tail == length) {
			// Lock the queue
			std::lock_guard<thread::spinlock> locked(lock);
			int head = head_index.load(std::memory_order_relaxed);

			// Resize the queue if it is more than 75% full
			if (head <= length / 4) {
				T* old = items;
				length *= 2;
				items = new T[length];
				std::move(old + head, old + tail, items);
				delete[] old;
			} else {
				// Simply shift the items to free up space at the end
				std::move(items + head, items + tail, items);
			}
			tail -= head;
			head_index.store(0, std::memory_order_relaxed);
		}

		// Now add the job
		items[tail] = job;
		tail_index.store(tail + 1, std::memory_order_release);
	}

	// Pop a job from the tail of this thread's queue
	T pop()
	{
		int tail = tail_index.load(std::memory_order_relaxed);

		// Early exit if our queue is empty
		if (head_index.load(std::memory_order_relaxed) >= tail)
			return nullptr;

		// Make sure tail is stored before we read head
		tail--;
		tail_index.store(tail, std::memory_order_relaxed);
		std::atomic_thread_fence(std::memory_order_seq_cst);

		// Race to the queue
		if (head_index.load(std::memory_order_relaxed) <= tail)
			return items[tail];

		// There is a concurrent steal, lock the queue and try again
		std::lock_guard<thread::spinlock> locked(lock);

		// Check if the item is still available
		if (head_index.load(std::memory_order_relaxed) <= tail)
			return items[tail];

		// Otherwise restore the tail and fail
		tail_index.store(tail + 1, std::memory_order_relaxed);
		return nullptr;
	}

	// Steal a job from the head of this thread's queue
	T steal()
	{
		// Lock the queue to prevent concurrent steals
		std::lock_guard<thread::spinlock> locked(lock);

		// Make sure head is stored before we read tail
		int head = head_index.load(std::memory_order_relaxed);
		head_index.store(head + 1, std::memory_order_relaxed);
		std::atomic_thread_fence(std::memory_order_seq_cst);

		// Check if there is a job to steal
		if (head < tail_index.load(std::memory_order_relaxed)) {
			// Need acquire fence to synchronise with concurrent push
			std::atomic_thread_fence(std::memory_order_acquire);
			return items[head];
		}

		// Otherwise restore the head and fail
		head_index.store(head, std::memory_order_relaxed);
		return nullptr;
	}

private:
	T* items;
	int length;
	thread::spinlock lock;
	std::atomic<int> head_index, tail_index;
};

// Public job queue, which is used to queue jobs from outside the thread pool
// FIXME: multiple lanes
template<typename T> class locked_queue {
public:
	locked_queue(size_t length)
		: queue{length} {}

	// Push a job to the end of the queue
	void push(T job)
	{
		std::lock_guard<thread::spinlock> locked(lock);

		// Resize queue if it is full
		if (queue.full())
			queue.set_capacity(queue.capacity() * 2);

		// Push the item
		queue.push_back(job);
	}

	// Pop a job from the front of the queue
	T pop()
	{
		std::lock_guard<thread::spinlock> locked(lock);

		// See if an item is available
		if (queue.empty())
			return nullptr;
		else {
			T job = queue.front();
			queue.pop_front();
			return job;
		}
	}

private:
	boost::circular_buffer<T> queue;
	thread::spinlock lock;
};

// Currently active task for a thread.
static thread_local task* current_task = nullptr;

// Whether the current task is to be recycled for continuation
static thread_local bool current_task_continue;

// Current thread's work stealing queue, NULL if not in thread pool
static thread_local work_steal_queue<task*>* current_wsqueue = nullptr;

// Number of threads in the pool, including the master thread
static int num_threads;

// Array of work stealing queues for each thread
static work_steal_queue<task*>** wsqueues;

// Global queue for tasks from outside the pool
static locked_queue<task*> public_queue{INITIAL_JOBQUEUE_SIZE};

void spawn(std::function<void()>&& func)
{
	// Allocate new task
	task* new_task = new task;
	new_task->function = std::move(func);
	new_task->parent = current_task;

	// Check if we are in the thread pool
	if (current_wsqueue) {
		// Increment reference count on parent task
		current_task->ref_count.fetch_add(1, std::memory_order_relaxed);

		// Push task onto our task queue
		current_wsqueue->push(new_task);
	} else {
		// Push task onto public queue
		public_queue.push(new_task);
	}
}

void spawn_and_wait(std::function<void()>&& func)
{
	// Create dummy task to hold a reference count
	task dummy_task;
	task* old = current_task;
	current_task = &dummy_task;
	dummy_task.ref_count.store(1, std::memory_order_relaxed);

	// Spawn the task and wait for it to complete
	spawn(std::move(func));
	wait_for_all();

	// Restore current task
	current_task = old;
}

void continue_with(std::function<void()>&& func)
{
	// Replace function in current task
	current_task->function = std::move(func);

	// Mark task for continuation
	current_task_continue = true;
}

void wait_for_all()
{
	// Wait for the current task to return to its original ref count of 1
	while (current_task->ref_count.load(std::memory_order_relaxed) != 1)
		yield();
}

static void run_task(task* job)
{
	// Set current task
	task* old = current_task;
	bool old_continue = current_task_continue;
	current_task = job;
	current_task_continue = false;

	// Initialize reference count
	job->ref_count.store(1, std::memory_order_relaxed);

	// Get task function and run it. We make a local copy because
	// continue_with may overwrite job->function.
	// FIXME: Exception handling
	std::function<void()> func = std::move(job->function);
	func();

	// Handle continuations and parents
	if (current_task_continue) {
		// Decrement reference count of job
		if (job->ref_count.fetch_sub(1, std::memory_order_release) == 1) {
			// If the refcount is now 0, run the continuation
			std::atomic_thread_fence(std::memory_order_acquire);
			current_wsqueue->push(job);
		}
	} else {
		// Make sure all sub-tasks have completed
		wait_for_all();

		// Decrement reference count of parent
		if (job->parent && job->parent->ref_count.fetch_sub(1, std::memory_order_release) == 1) {
			// If the refcount is now 0, run the parent
			std::atomic_thread_fence(std::memory_order_acquire);
			current_wsqueue->push(job->parent);
		}

		// Free the task
		delete job;
	}

	// Restore current_task
	current_task = old;
	current_task_continue = old_continue;
}

void yield()
{
	task* job;

	// Try to fetch from local queue
	job = current_wsqueue->pop();
	if (job) {
		run_task(job);
		return;
	}

	// Try to fetch from global queue
	job = public_queue.pop();
	if (job) {
		run_task(job);
		return;
	}

	// Try to steal from another thread
	// FIXME: ordering
	for (int i = 0; i < num_threads; i++) {
		if (wsqueues[i] != current_wsqueue) {
			job = wsqueues[i]->steal();
			if (job) {
				run_task(job);
				return;
			}
		}
	}

	// No work was found, yield to operating system
	// FIXME: Sleep if yielding too much to avoid 100% CPU usage
	std::this_thread::yield();
}

// Worker thread main loop
static void worker_thread(work_steal_queue<task*>* wsqueue)
{
	current_wsqueue = wsqueue;

	// FIXME: shutdown
	while (true)
		yield();
}

void init(int num_threads_)
{
	// Bound check thread count
	num_threads = std::max(num_threads_, 1);

	// Print thread count
	Printf("Using %d threads", num_threads);

	// Set up master thread
	static task root_task;
	root_task.ref_count.store(1, std::memory_order_relaxed);
	current_task = &root_task;
	wsqueues = new work_steal_queue<task*>*[num_threads];
	current_wsqueue = wsqueues[0] = new work_steal_queue<task*>(INITIAL_JOBQUEUE_SIZE);

	// Start worker threads
	for (int i = 1; i < num_threads; i++) {
		wsqueues[i] = new work_steal_queue<task*>(INITIAL_JOBQUEUE_SIZE);
		std::thread(worker_thread, wsqueues[i]).detach();
	}
}

task* add_child()
{
	current_task->ref_count.fetch_add(1, std::memory_order_relaxed);
	return current_task;
}

void child_finished(task* parent, std::function<void()>&& continuation)
{
	if (continuation) {
		// Allocate new task
		task* new_task = new task;
		new_task->function = std::move(continuation);
		new_task->parent = parent;

		// Check if we are in the thread pool
		if (current_wsqueue) {
			// Push task onto our task queue
			current_wsqueue->push(new_task);
		} else {
			// Push task onto public queue
			public_queue.push(new_task);
		}
	} else {
		// Decrement reference count of parent
		if (parent->ref_count.fetch_sub(1, std::memory_order_release) == 1) {
			// If the refcount is now 0, run the parent
			std::atomic_thread_fence(std::memory_order_acquire);

			// Check if we are in the thread pool
			if (current_wsqueue) {
				// Push task onto our task queue
				current_wsqueue->push(parent);
			} else {
				// Push task onto public queue
				public_queue.push(parent);
			}
		}
	}
}

}
