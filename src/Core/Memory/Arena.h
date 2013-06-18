//@@COPYRIGHT@@

// Fast allocators that can't free individual allocations. All allocations can
// be freed at once though.

// Default alignment for allocations
#define DEFAULT_MEMORY_ALIGNMENT 16

// Arena allocator that allocates from a fixed block of memory. It will allocate
// extra memory blocks dynamically if the initial block becomes full.
template<size_t blockSize> class MemArena {
public:
	// Constructor
	MemArena()
	{
		blockList.push_front(internal_block);
		offset = internal_block.data;
	}

	// Destructor
	~MemArena()
	{
		FreeAll();
	}

	// Allocate an object. Alignment must be a power of 2.
	__malloc void *Alloc(size_t size, size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
	{
		// Align the offset
		offset = reinterpret_cast<char *>(PAD(reinterpret_cast<intptr_t>(offset), alignment));

		// If the current block is full, get a new one.
		blockHeader_t *block = &blockList.front();
		if (offset + size > reinterpret_cast<char *>(block) + blockSize)
			block = NewBlock(size, alignment);

		// Increment offset and return
		void *ptr = offset;
		offset += size;
		return ptr;
	}

	// Copy a string
	__malloc const char *CopyString(const char *string)
	{
		char *newString = Alloc(strlen(string) + 1, 1);
		strcpy(newString, string);
		return newString;
	}

	// Free all allocated memory
	void FreeAll()
	{
		while (boost::next(blockList.begin()) != blockList.end())
			blockList.pop_front_and_dispose(MemFree);
		offset = blockList.front().data;
	}

private:
	// Header at the begining of each block
	struct blockHeader_t: public SList<blockHeader_t>::Hook {
		size_t size;
		char data[0];
	};

	// Internal block
	struct: public blockHeader_t {
		char data[blockSize - sizeof(blockHeader_t)];
	} internal_block;

	// Make sure blockSize is reasonable
	StaticAssert(blockSize > sizeof(blockHeader_t));

	// Linked list of blocks
	SList<blockHeader_t> blockList;

	// Allocation offset in the current block
	char *offset;

	// Allocate a new block, which needs to be big enough to include the new
	// element.
	blockHeader_t *NewBlock(size_t size, size_t alignment)
	{
		// Calculate the block size
		size_t newSize = std::max(blockSize, size + PAD(sizeof(blockHeader_t), alignment));

		// Get a new block
		blockHeader_t *newBlock = static_cast<blockHeader_t *>(MemAlloc(newSize));

		// Update block list. We want to continue using the old block if we
		// allocated a block for a large item.
		if (newSize == blockSize || blockList.empty())
			blockList.push_front(*newBlock);
		else
			blockList.insert_after(blockList.begin(), *newBlock);

		// Align the offset
		offset = reinterpret_cast<char *>(PAD(reinterpret_cast<intptr_t>(newBlock->data), alignment));

		return newBlock;
	}
};

// Arena allocator that allocates from a fixed block of memory. It will return
// NULL if the block becomes full.
template<size_t blockSize> class MemArenaFixed {
public:
	// Constructor
	MemArenaFixed()
	{
		offset = internal_block;
	}

	// Allocate an object. Alignment must be a power of 2.
	__malloc void *Alloc(size_t size, size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
	{
		// Align the offset
		offset = reinterpret_cast<char *>(PAD(reinterpret_cast<intptr_t>(offset), alignment));

		// Check if the block is full
		if (offset + size > internal_block + blockSize)
			return NULL;

		// Increment offset and return
		void *ptr = offset;
		offset += size;
		return ptr;
	}

	// Copy a string
	__malloc const char *CopyString(const char *string)
	{
		char *newString = Alloc(strlen(string) + 1, 1);
		strcpy(newString, string);
		return newString;
	}

	// Free all allocated memory
	void FreeAll()
	{
		offset = internal_block;
	}

private:
	char internal_block[blockSize];
	char *offset;
};

// Wrap a MemArena with a mutex to allow access from multiple threads. Note that
// FreeAll is not made thread-safe.
template<typename Arena> class MemArenaThreadWrapper: public Arena {
public:
	// Allocate an object, with locking
	__malloc void *Alloc(size_t size, size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
	{
		lock.Lock();
		void *ptr = Arena::Alloc(size, alignment);
		lock.Unlock();
		return ptr;
	}

	// Copy a string
	__malloc const char *CopyString(const char *string)
	{
		char *newString = Alloc(strlen(string) + 1, 1);
		strcpy(newString, string);
		return newString;
	}

private:
	// Lock protecting the arena
	Mutex lock;
};

// Operator new overload that uses a MemArena. Use it like this:
// T *myPtr = new(arena, [alignment]) T(...);
// Note that you will then have to destroy the objects manually where you would
// normally use delete: myPtr->~T();
template<size_t blockSize> inline __malloc void *operator new(size_t size, MemArena<blockSize> &arena, size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
{
	return arena.Alloc(size, alignment);
}
template<size_t blockSize> inline __malloc void *operator new(size_t size, MemArenaFixed<blockSize> &arena, size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
{
	return arena.Alloc(size, alignment);
}
template<typename Arena> inline __malloc void *operator new(size_t size, MemArenaThreadWrapper<Arena> &arena, size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
{
	return arena.Alloc(size, alignment);
}
template<size_t blockSize> inline __malloc void *operator new[](size_t size, MemArena<blockSize> &arena, size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
{
	return arena.Alloc(size, alignment);
}
template<size_t blockSize> inline __malloc void *operator new[](size_t size, MemArenaFixed<blockSize> &arena, size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
{
	return arena.Alloc(size, alignment);
}
template<typename Arena> inline __malloc void *operator new[](size_t size, MemArenaThreadWrapper<Arena> &arena, size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
{
	return arena.Alloc(size, alignment);
}
