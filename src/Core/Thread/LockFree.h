//@@COPYRIGHT@@

// Lock-free list and lock-free queue
// Based on the wine implementation of the Interlocked*SList functions

namespace lockfree {

// Intrusive lock-free stack
// T is the type of objects in the list
// Hook is a intrusive hook definition (base_hook<>/member_hook<>/value_traits<>)
// Supported hooks are boost intrusive slist hooks in normal_link and safe_link modes.
// Note: Stateful value_traits are not supported
template<typename T, typename Hook> class intrusive_stack: boost::noncopyable {
public:
	// Intrusive slist type used internally
	typedef intrusive::slist<
		T,
		Hook,
		intrusive::constant_time_size<false>,
		intrusive::linear<true>,
		intrusive::cache_last<false>
	> slist;

	// Inherited typedefs
	typedef typename slist::value_traits value_traits;
	typedef typename slist::pointer pointer;
	typedef typename slist::const_pointer const_pointer;
	typedef typename slist::value_type value_type;
	typedef typename slist::reference reference;
	typedef typename slist::const_reference const_reference;
	typedef typename slist::difference_type difference_type;
	typedef typename slist::size_type size_type;
	typedef typename slist::iterator iterator;
	typedef typename slist::const_iterator const_iterator;
	typedef typename slist::node_traits node_traits;
	typedef typename slist::node node;
	typedef typename slist::node_ptr node_ptr;
	typedef typename slist::const_node_ptr const_node_ptr;
	typedef typename slist::node_algorithms node_algorithms;

	// Check if safe-link hooks are used
	static const bool use_safe_link = (int)value_traits::link_mode == (int)intrusive::safe_link;

private:
	// Structure containing list head and ABA counter, aligned for use
	// with atomic operations.
	struct alignas(sizeof(void*) * 2) list_head {
		node_ptr list;
		uintptr_t counter;
	};

	// Atomic list head
	std::atomic<list_head> head;

public:
	// Initialize the list to be empty
	intrusive_stack()
	{
		head.store({nullptr, 0}, std::memory_order_relaxed);
	}

	// If safe link mode is used, unlink all nodes if safe_link is used
	~intrusive_stack()
	{
		node_ptr current = unlocked_list();
		if (use_safe_link) {
			while (current) {
				node_ptr next = node_traits::get_next(current);
				node_algorithms::init(current);
				current = next;
			}
		}
	}

	// Check if the stack is empty. The result should not be relied on
	// as another thread may have pushed an item in the meantime.
	bool empty() const
	{
		return unlocked_list() == nullptr;
	}

	// Push an item onto the stack
	void push(reference item)
	{
		list_head old_head, new_head;
		node_ptr item_node = value_traits::to_node_ptr(item);
		if (use_safe_link)
			BOOST_INTRUSIVE_SAFE_HOOK_DEFAULT_ASSERT(node_algorithms::inited(item_node));

		old_head = head.load(std::memory_order_relaxed);
		do {
			node_traits::set_next(item_node, old_head.list);
			node_traits::set_next(item_node, old_head.list);
			new_head.list = item_node;
			new_head.counter = old_head.counter + 1;
		} while (!head.compare_exchange_weak(old_head, new_head, std::memory_order_release, std::memory_order_relaxed));
	}

	// Push a set of linked nodes. The first node will become
	// the top of the stack.
	void splice(node_ptr first, node_ptr last)
	{
		list_head old_head, new_head;

		old_head = head.load(std::memory_order_relaxed);
		do {
			node_traits::set_next(last, old_head.list);
			new_head.list = first;
			new_head.counter = old_head.counter + 1;
		} while (!head.compare_exchange_weak(old_head, new_head, std::memory_order_release, std::memory_order_relaxed));
	}

	// Pop an item from the stack. Returns NULL if the stack is empty.
	// A popped item must not be freed until there are no more threads trying to pop.
	pointer pop()
	{
		list_head old_head, new_head;
		node_ptr item;

		// WARNING: There is a chance that the memory referenced by
		// item becomes invalid if another thread pops and frees it.
		old_head = head.load(std::memory_order_relaxed);
		do {
			item = old_head.list;
			if (item == nullptr)
				return nullptr;
			new_head.list = node_traits::get_next(item); // Here
			new_head.counter = old_head.counter + 1;
		} while (!head.compare_exchange_weak(old_head, new_head, std::memory_order_acquire, std::memory_order_relaxed));

		if (use_safe_link)
			node_algorithms::init(item);
		return value_traits::to_value_ptr(item);
	}

	// Remove all items from the stack. Returns a linear list of linked nodes.
	node_ptr flush()
	{
		list_head old_head, new_head;
		node_ptr first;

		old_head = head.load(std::memory_order_relaxed);
		do {
			first = old_head.list;
			new_head.list = nullptr;
			new_head.counter = old_head.counter + 1;
		} while (!head.compare_exchange_weak(old_head, new_head, std::memory_order_acquire, std::memory_order_relaxed));

		return first;
	}

	// Get a handle to the internal slist. Note that any operations done
	// using it are not atomic.
	node_ptr unlocked_list()
	{
		return head.load(std::memory_order_relaxed).list;
	}
	const_node_ptr unlocked_list() const
	{
		return head.load(std::memory_order_relaxed).list;
	}
};

// Lock-free freelist allocator, based on intrusive_stack.
// This allocator does not free memory, instead it keeps a list of free items
// which it reuses. This is necessary for some lock-free algorithms where
// memory cannot be freed until all threads have stopped using it.
template<typename T, typename Alloc = std::allocator<T>> class freelist_allocator {
public:
	// Inherit allocator types
	typedef typename Alloc::size_type size_type;
	typedef typename Alloc::difference_type difference_type;
	typedef typename Alloc::pointer pointer;
	typedef typename Alloc::const_pointer const_pointer;
	typedef typename Alloc::reference reference;
	typedef typename Alloc::const_reference const_reference;
	typedef typename Alloc::value_type value_type;

	// Rebind to another type
	template<typename U, typename Alloc2 = Alloc> struct rebind {
		typedef freelist_allocator<U, Alloc2> other;
	};

private:
	// Internal freelist node and hook
	typedef intrusive::slist_base_hook<
		intrusive::link_mode<intrusive::normal_link>
	> hook;
	struct node: public hook {};

	// Allocator and freelist
	struct alloc_and_freelist_t: public Alloc {
		intrusive_stack<node, intrusive::base_hook<hook>> freelist;
	} alloc_and_freelist;

public:
	// Constructors, don't copy freelist to avoid double-free
	freelist_allocator() = default;
	freelist_allocator(const freelist_allocator& other) {};
	template<typename U> freelist_allocator(const freelist_allocator<U>& other) {};

	// Release freelist on destruction
	~freelist_allocator()
	{
		typedef typename intrusive_stack<node, intrusive::base_hook<hook>>::node_ptr node_ptr;
		typedef typename intrusive_stack<node, intrusive::base_hook<hook>>::node_traits node_traits;
		typedef typename intrusive_stack<node, intrusive::base_hook<hook>>::value_traits value_traits;
		node_ptr current = alloc_and_freelist.freelist.unlocked_list();
		while (current) {
			node_ptr next = node_traits::get_next(current);
			alloc_and_freelist.deallocate(value_traits::to_value_ptr(current), 1);
			current = next;
		}
	}

	pointer address(reference x) const noexcept
	{
		return alloc_and_freelist.address(x);
	}
	const_pointer address(const_reference x) const noexcept
	{
		return alloc_and_freelist.address(x);
	}

	pointer allocate(size_type n, const void* hint = nullptr)
	{
		static_assert(sizeof(node) <= sizeof(T), "Size of T for lockfree::freelist_allocator<T> must be at least 1 pointer");
		node* item = alloc_and_freelist.freelist.pop();
		if (item == nullptr)
			return alloc_and_freelist.allocate(n, hint);
		else
			return reinterpret_cast<pointer>(item);
	}

	void deallocate(pointer p, size_type n)
	{
		static_assert(sizeof(node) <= sizeof(T), "Size of T for lockfree::freelist_allocator<T> must be at least 1 pointer");
		node* item = reinterpret_cast<node*>(p);
		alloc_and_freelist.freelist.push(*item);
	}

	size_type max_size() const noexcept
	{
		// We can only handle single element allocations
		return 1;
	}

	template<typename U, typename... Args> void construct(U* p, Args&&... args)
	{
		alloc_and_freelist.construct(p, std::forward<Args>(args)...);
	}

	template<typename U> void destroy(U* p)
	{
		alloc_and_freelist.destroy(p);
	}
};

// Allocators can free into another allocator's freelist if they have the same type
template<typename T> inline bool operator==(const freelist_allocator<T>&, const freelist_allocator<T>&)
{
	return true;
}
template<typename T> inline bool operator!=(const freelist_allocator<T>&, const freelist_allocator<T>&)
{
	return false;
}

// Lock-free stack
// T is the type of objects in the list
// Alloc is the allocator to use for allocating nodes. Note that nodes
// must not be freed if there are other threads trying to pop them. The
// default allocator (freelist_allocator) handles this by never freeing
// nodes.
template<typename T, typename Alloc = freelist_allocator<T>> class stack: boost::noncopyable {
public:
	// Container typedefs
	typedef T value_type;
	typedef typename Alloc::pointer pointer;
	typedef typename Alloc::const_pointer const_pointer;
	typedef typename Alloc::reference reference;
	typedef typename Alloc::const_reference const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef Alloc allocator_type;

private:
	// Internal node type
	typedef intrusive::slist_base_hook<
		intrusive::link_mode<intrusive::normal_link>
	> hook;
	struct node: public hook {
		T obj;

		// Forward constructor arguments to object
		template<typename... Args> node(Args&&... args)
			: obj(std::forward<Args>(args)...) {}
	};

	// Node allocator
	typedef typename Alloc::template rebind<node>::other node_allocator;

	// Stack containing data and the allocator
	struct data_and_alloc_t: public node_allocator {
		intrusive_stack<node, hook> data;

		// Initialize allocator
		data_and_alloc_t(const Alloc& alloc)
			: node_allocator(alloc) {}
	} data_and_alloc;

public:
	// Constructor
	explicit stack(const Alloc& alloc = Alloc())
		: data_and_alloc(alloc) {}

	// Check if the stack is empty. The result should not be relied on
	// as another thread may have pushed an item in the meantime.
	bool empty() const
	{
		return data_and_alloc.data.empty();
	}

	// Push an item onto the stack.
	void push(const T& item)
	{
		// Emplace using copy constructor
		emplace(item);
	}
	void push(T&& item)
	{
		// Emplace using move constructor
		emplace(std::move(item));
	}

	// Construct an item and push it onto the stack.
	template<typename... Args> void emplace(Args&&... args)
	{
		// Allocate a node
		node* item = data_and_alloc.allocate(1);

		// Construct the node
		try {
			data_and_alloc.construct(item, std::forward<Args>(args)...);
		} catch (...) {
			// Free it if constructor threw an exception
			data_and_alloc.deallocate(item, 1);
			throw;
		}

		// Push it onto the stack
		data_and_alloc.data.push(*item);
	}

	// Try to pop an item from the stack
	boost::optional<value_type> pop()
	{
		// Optional return value
		boost::optional<value_type> ret;

		// Try to pop an item off the stack
		node* item = data_and_alloc.data.dequeue();
		if (item != nullptr) {
			// Return the result and free the node
			ret = std::move(item->obj);
			data_and_alloc.destroy(item);
			data_and_alloc.deallocate(item, 1);
		}

		return ret;
	}
};

// Intrusive lock-free FIFO single-consumer, multiple-producer queue
// T is the type of objects in the list
// Hook is a intrusive hook definition (base_hook<>/member_hook<>/value_traits<>)
// Supported hook are boost intrusive slist hook in normal_link or safe_link modes.
// Note: Stateful value_traits are not supported
template<typename T, typename Hook> class intrusive_queue_sc: boost::noncopyable {
public:
	// Intrusive slist type used internally
	typedef typename intrusive_stack<T, Hook>::slist slist;

	// Inherited typedefs
	typedef typename slist::value_traits value_traits;
	typedef typename slist::pointer pointer;
	typedef typename slist::const_pointer const_pointer;
	typedef typename slist::value_type value_type;
	typedef typename slist::reference reference;
	typedef typename slist::const_reference const_reference;
	typedef typename slist::difference_type difference_type;
	typedef typename slist::size_type size_type;
	typedef typename slist::iterator iterator;
	typedef typename slist::const_iterator const_iterator;
	typedef typename slist::node_traits node_traits;
	typedef typename slist::node node;
	typedef typename slist::node_ptr node_ptr;
	typedef typename slist::const_node_ptr const_node_ptr;
	typedef typename slist::node_algorithms node_algorithms;

	// Check if safe-link hooks are used
	static const bool use_safe_link = (int)value_traits::link_mode == (int)intrusive::safe_link;

private:
	// Intrusive stack pushed to by multiple producers
	intrusive_stack<T, Hook> public_list;

	// Private unlocked stack for the single consumer
	slist private_list;

public:
	// Check if the queue is empty. The result should not be relied on
	// as another thread may have enqueued an item in the meantime.
	bool empty() const
	{
		return private_list.empty() && public_list.empty();
	}

	// Add an item to the queue
	void enqueue(reference item)
	{
		public_list.push(item);
	}

	// Pop an item from the queue. Returns NULL if the queue is empty.
	// This function can only be called from one thread at a time.
	pointer dequeue()
	{
		// Try to get an item from the private list
		if (!private_list.empty()) {
			reference item = private_list.front();
			private_list.pop_front();
			return &item;
		}

		// Flush the public list
		node_ptr list = public_list.flush();
		if (list == nullptr)
			return nullptr;

		// Insert items into the private list in reverse order
		while (node_traits::get_next(list) != nullptr) {
			if (use_safe_link)
				node_algorithms::init(list);
			private_list.push_front(*node_traits::to_value_ptr(list));
			list = node_traits::get_next(list);
		}

		// Return the last item
		if (use_safe_link)
			node_algorithms::init(list);
		return value_traits::to_value_ptr(list);
	}
};

// Lock-free FIFO single-consumer, multiple-producer queue
// Note that a freelist allocator is not required here since there is
// only one thread popping.
template<typename T, typename Alloc = std::allocator<T>> class queue_sc: boost::noncopyable {
public:
	// Container typedefs
	typedef T value_type;
	typedef typename Alloc::pointer pointer;
	typedef typename Alloc::const_pointer const_pointer;
	typedef typename Alloc::reference reference;
	typedef typename Alloc::const_reference const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef Alloc allocator_type;

private:
	// Internal node type
	typedef intrusive::slist_base_hook<
		intrusive::link_mode<intrusive::normal_link>
	> hook;
	struct node: public hook {
		T obj;

		// Forward constructor arguments to object
		template<typename... Args> node(Args&&... args)
			: obj(std::forward<Args>(args)...) {}
	};

	// Node allocator
	typedef typename Alloc::template rebind<node>::other node_allocator;

	// Queue containing data and the allocator
	struct data_and_alloc_t: public node_allocator {
		intrusive_queue_sc<node, hook> data;

		// Initialize allocator
		data_and_alloc_t(const Alloc& alloc)
			: node_allocator(alloc) {}
	} data_and_alloc;

public:
	// Constructor
	explicit queue_sc(const Alloc& alloc = Alloc())
		: data_and_alloc(alloc) {}

	// Check if the queue is empty. The result should not be relied on
	// as another thread may have pushed an item in the meantime.
	bool empty() const
	{
		return data_and_alloc.data.empty();
	}

	// Add an item to the queue
	void enqueue(const T& item)
	{
		// Emplace using copy constructor
		emplace(item);
	}
	void enqueue(T&& item)
	{
		// Emplace using move constructor
		emplace(std::move(item));
	}

	// Construct an item and add it to the queue.
	template<typename... Args> void emplace(Args&&... args)
	{
		// Allocate a node
		node* item = data_and_alloc.allocate(1);

		// Construct the node
		try {
			data_and_alloc.construct(item, std::forward<Args>(args)...);
		} catch (...) {
			// Free it if constructor threw an exception
			data_and_alloc.deallocate(item, 1);
			throw;
		}

		// Push it onto the queue
		data_and_alloc.data.enqueue(*item);
	}

	// Try to dequeue an item
	boost::optional<value_type> dequeue()
	{
		// Optional return value
		boost::optional<value_type> ret;

		// Try to dequeue an item
		node* item = data_and_alloc.data.dequeue();
		if (item != nullptr) {
			// Return the result and free the node
			ret = std::move(item->obj);
			data_and_alloc.destroy(item);
			data_and_alloc.deallocate(item, 1);
		}

		return ret;
	}
};

}
