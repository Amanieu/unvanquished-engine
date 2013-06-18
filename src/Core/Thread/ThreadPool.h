//@@COPYRIGHT@@

// Thread pool framework

namespace threadpool {

// Submit a task for execution in the thread pool as a child of the
// current task.
EXPORT void spawn(std::function<void()>&& func);
template<typename T> inline void spawn(T&& func)
{
	spawn(std::function<void()>(std::forward<T>(func)));
}
template<typename T, typename... Args>
inline void spawn(T&& obj, Args&&... args)
{
	spawn(std::bind(std::forward<T>(obj), std::forward<Args>(args)...));
}

// Spawn a single task and wait for it to complete.
EXPORT void spawn_and_wait(std::function<void()>&& func);
template<typename T> inline void spawn_and_wait(T&& func)
{
	spawn_and_wait(std::function<void()>(std::forward<T>(func)));
}
template<typename T, typename... Args>
inline void spawn_and_wait(T&& obj, Args&&... args)
{
	spawn_and_wait(std::bind(std::forward<T>(obj), std::forward<Args>(args)...));
}

// Once all children of the current task have completed, continue with
// the given task. Using continuations is generally prefered to calling
// wait_for_all at the end of a task.
EXPORT void continue_with(std::function<void()>&& func);
template<typename T> inline void continue_with(T&& func)
{
	continue_with(std::function<void()>(std::forward<T>(func)));
}
template<typename T, typename... Args>
inline void continue_with(T&& obj, Args&&... args)
{
	continue_with(std::bind(std::forward<T>(obj), std::forward<Args>(args)...));
}

// Wait for all children of the current task to complete. This is
// implicitly called at the end of a task function if no continuation is
// specified.
EXPORT void wait_for_all();

// Do some useful work from the task pool while waiting for an event.
// If no work is available then the thread yields to the operating system.
EXPORT void yield();

// Initialize the thread pool for running jobs using the given number
// of threads.
void init(int num_threads);

// Functions below are to allow external tasks such as asynchronous I/O to
// integrate into the thread pool framework and act as normal tasks.

// Internal task structure
struct task;

// Add a child to the current task, returns a handle to the parent task.
task* add_child();

// Notify a parent task that a child has finished working. If a continuation is
// given then run it as a child of the given parent task.
void child_finished(task* parent, std::function<void()>&& continuation = nullptr);
template<typename T> inline void child_finished(task* parent, T&& continuation)
{
	child_finished(parent, std::function<void()>(std::forward<T>(continuation)));
}
template<typename T, typename... Args>
inline void child_finished(task* parent, T&& obj, Args&&... args)
{
	child_finished(parent, std::bind(std::forward<T>(obj), std::forward<Args>(args)...));
}

}
