//@@COPYRIGHT@@

// Various memory management utilities

// Initialize the memory subsystem
namespace Memory {
void Init();
}

// Replacements for the standard malloc() and free()
EXPORT __malloc void *MemAlloc(size_t size);
EXPORT void MemFree(void *ptr);

// Helper functions to allocate space for a read-only copy of a string.
EXPORT __malloc const char *CopyString(const char *string);
EXPORT void FreeString(const char *string);

#ifndef BUILD_TEST
// Overloads for the standard C++ new and delete opertors to use the main heap.
// Note that these don't throw bad_alloc, but instead crash if out of memory.
inline __malloc void *operator new(size_t size) throw(std::bad_alloc)
{
	return MemAlloc(size);
}
inline __malloc void *operator new[](size_t size) throw(std::bad_alloc)
{
	return MemAlloc(size);
}
inline void operator delete(void *ptr) throw()
{
	return MemFree(ptr);
}
inline void operator delete[](void *ptr) throw()
{
	return MemFree(ptr);
}
#endif

// STL allocator class that uses our memory functions instead
template<typename T> class StlAllocator {
public:
	// STL typedefs
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T *pointer;
	typedef const T *const_pointer;
	typedef T &reference;
	typedef const T &const_reference;
	typedef T value_type;

	// Constructors
	StlAllocator() {}
	StlAllocator(const StlAllocator &) {}

	// Allow assignment from allocators of different types
	template<typename U> StlAllocator(const StlAllocator<U> &) {}
	template<typename U> StlAllocator &operator=(const StlAllocator<U> &)
	{
		return *this;
	}

	// Allocate and free memory
	pointer allocate(size_type n, const void * = NULL)
	{
		return static_cast<pointer>(MemAlloc(n * sizeof(T)));
	}
	void deallocate(void *ptr, size_type)
	{
		MemFree(ptr);
	}

	// Get an address from a reference
	pointer address(reference x) const
	{
		return &x;
	}
	const_pointer address(const_reference x) const
	{
		return &x;
	}

	// Construct and destruct an object
	void construct(pointer ptr, const T &val)
	{
		new(ptr) T(val);
	}
	void destroy(pointer ptr)
	{
		ptr->~T();
	}

	// Maximum allocation size
	size_type max_size() const
	{
		return std::numeric_limits<size_type>::max();
	}

	// Get an allocator for a different type
	template <typename U> struct rebind {
		typedef StlAllocator<U> other;
	};
};
