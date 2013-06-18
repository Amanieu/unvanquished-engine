//@@COPYRIGHT@@

// Memory pool of fixed-length objects. All objects allocated from a pool must
// be freed in order to avoid memory leaks.

#ifndef BUILD_MODULE

// Implementation details
namespace MemPoolImpl {

// Size of a MemPool block
static const int BLOCK_SIZE = 65536;

// Empty item containing only a hook for a list
struct freeItem_t: public SList<freeItem_t>::Hook {};

// Memory block descriptor. This is always located at the end of a block. The
// block's next pointer is set to THREAD_BLOCK when it belongs to a thread.
struct memBlock_t: public List<memBlock_t>::Hook {
	SList<freeItem_t> freeList;
	int numFree;
	int objSize;
	Mutex lock;
};

// Block lists, protected by a lock. The partial list contains blocks which
// are partially filled.
struct blockLists_t {
	List<memBlock_t> partial;
	Mutex lock;
};

// Per-thread data for fast lock-less allocation
struct threadData_t {
	memBlock_t *threadBlock;
	SList<freeItem_t>::node_ptr freeList;
	void *reapStart;
};

// Actual alloc and free functions
EXPORT __malloc void *Alloc(size_t objSize, blockLists_t &blockLists, threadData_t &threadData);
EXPORT void Free(void *ptr, int maxObjs, blockLists_t &blockLists, threadData_t &threadData);

// Helper function for freeing a whole block, for module unloading
EXPORT void FreeBlock(memBlock_t *block);

// Helper function for the general allocator, retrieves objSize from a pointer
size_t GetObjSize(void *ptr);

}

// MemPool class
template<size_t objSize> class MemPool {
public:
	// Allocate a single object from the pool
	static __malloc void *Alloc()
	{
#if BUILD_MODULE && _WIN32
		MemPoolImpl::threadData_t *threadDataPtr = threadDataTLS.Get();
		if (!threadDataPtr) {
			threadDataPtr = new(malloc(sizeof(MemPoolImpl::threadData_t))) MemPoolImpl::threadData_t;
			threadDataPtr->threadBlock = NULL;
			threadDataPtr->freeList = NULL;
			threadDataTLS.Set(threadDataPtr);
			moduleData.data[AtomicIncrement(&moduleData.count)] = threadDataPtr;
		}

		MemPoolImpl::threadData_t &threadData = *threadDataPtr;
#elif BUILD_MODULE
		// Register thread data. Check threadBlock since it can only be NULL on
		// the first allocation.
		if (!threadData.threadBlock)
			moduleData.data[AtomicIncrement(&moduleData.count)] = &threadData;
#endif

		return MemPoolImpl::Alloc(objSize, blockLists, threadData);
	}

	// Free a single object back to the pool
	static void Free(void *ptr)
	{
		// Handle NULL pointers
		if (!ptr)
			return;

#if BUILD_MODULE && _WIN32
		MemPoolImpl::threadData_t *threadDataPtr = threadDataTLS.Get();
		if (!threadDataPtr) {
			threadDataPtr = new(malloc(sizeof(MemPoolImpl::threadData_t))) MemPoolImpl::threadData_t;
			threadDataPtr->threadBlock = NULL;
			threadDataPtr->freeList = NULL;
			threadDataPtr->reapStart = NULL;
			threadDataTLS.Set(threadDataPtr);
			moduleData.data[AtomicIncrement(&moduleData.count)] = threadDataPtr;
		}

		MemPoolImpl::threadData_t &threadData = *threadDataPtr;
#endif

		MemPoolImpl::Free(ptr, maxObjs, blockLists, threadData);
	}

private:
	static MemPoolImpl::blockLists_t blockLists;
#if BUILD_MODULE && _WIN32
	static ThreadLocal<MemPoolImpl::threadData_t *> threadDataTLS;
#else
	static __thread MemPoolImpl::threadData_t threadData;
#endif

	// Number of objects that can fit in one block
	static const int maxObjs = (MemPoolImpl::BLOCK_SIZE - sizeof(MemPoolImpl::memBlock_t)) / objSize;

	// Make sure objSize is large enough to contain at least one pointer
	StaticAssert(objSize >= sizeof(void *));

	// Make sure we can fit at least 3 objects in a block
	StaticAssert(maxObjs >= 3);

#ifdef BUILD_MODULE
	// For modules, we register all threadData so that thread blocks can be
	// freed when the module is unloaded. Note that this means that memory pools
	// can't be used in static object destructors.
	struct moduleData_t {
		// Initialized to 0 by static initialization
		MemPoolImpl::threadData_t *data[MAX_REAL_THREADS];
		atomic_t count;

		// Release all thread blocks when the module is unloaded
		~moduleData_t()
		{
			for (int i = 0; i < count; i++) {
				if (data[i]) {
					FreeBlock(data[i]->threadBlock);
#ifdef _WIN32
					free(data[i]);
#endif
				}
			}
		}
	};
	static moduleData_t moduleData;
#endif
};

// Static member definitions
template<size_t objSize> MemPoolImpl::blockLists_t MemPool<objSize>::blockLists;
#ifdef BUILD_MODULE
template<size_t objSize> typename MemPool<objSize>::moduleData_t MemPool<objSize>::moduleData;
#endif
#if BUILD_MODULE && _WIN32
template<size_t objSize> ThreadLocal<MemPoolImpl::threadData_t *> MemPool<objSize>::threadDataTLS;
#else
template<size_t objSize> __thread MemPoolImpl::threadData_t MemPool<objSize>::threadData = {NULL, NULL, NULL};
#endif

#else

// In modules, use the general allocator instead if the object size matches one
// of the general allocator size classes. This is not needed in the main binary
// because the linker will merge the templates with those used by the general
// allocator.
template<size_t objSize> class MemPool {
public:
	static __malloc void *Alloc()
	{
		return MemAlloc(objSize);
	}
	static void Free(void *ptr)
	{
		MemFree(ptr);
	}
};

#endif

// Derive from this class to use a memory pool
template<typename T> class UseMemPool {
public:
	__malloc void *operator new(size_t size)
	{
		UNUSED(size);
		Assert(size == sizeof(T));
		return MemPool<sizeof(T)>::Alloc();
	}

	void operator delete(void *ptr)
	{
		MemPool<sizeof(T)>::Free(ptr);
	}
};
