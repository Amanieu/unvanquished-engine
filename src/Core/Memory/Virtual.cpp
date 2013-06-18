//@@COPYRIGHT@@

#ifndef _WIN32
// Data structure representing a mmap allocation
struct mmapInfo_t: public HashTable<mmapInfo_t>::Hook, UseMemPool<mmapInfo_t> {
	void *address;
	size_t size;

	// Constructor
	mmapInfo_t(void *address_, size_t size_)
	{
		address = address_;
		size = size_;
	}
};

// Hashtable operations
static inline size_t hash_value(const mmapInfo_t &info)
{
	boost::hash<intptr_t> hasher;
	return hasher(reinterpret_cast<intptr_t>(info.address) >> 12);
}
static inline bool operator==(const mmapInfo_t &a, const mmapInfo_t &b)
{
	return a.address == b.address;
}

// Hash table to track allocations
static HashTable<mmapInfo_t> mmapTable;

void MemVirtual::RegisterMmap(void *addr, size_t size)
{
	mmapTable.insert_equal(*new mmapInfo_t(addr, size));
}

size_t MemVirtual::ReleaseMmap(void *addr)
{
	HashTable<mmapInfo_t>::const_iterator i = mmapTable.find(mmapInfo_t(addr, 0));
	Assert(i != mmapTable.end());
	size_t size = i->size;
	mmapTable.erase_and_dispose(i, DeleteFunctor<mmapInfo_t>());
	return size;
}
#endif

// If the operating system supports memory overcommit, we can avoid a system
// call on block allocation.
#ifdef MAP_NORESERVE
#define HAVE_MEM_OVERCOMMIT
#endif

// Size of the memory region used for block allocation
#define ALLOC_SIZE (512 * 1024 * 1024)

// Bit set of all blocks in the allocation region
#define BITSET_SIZE (ALLOC_SIZE / (MemPoolImpl::BLOCK_SIZE * 32))
static uint32_t bitset[BITSET_SIZE];

// Pointer to the first word of the bitset containing at least one clear bit
static uint32_t *bitsetStart = bitset;

// Pointer to the memory region containing all blocks
static void *blockMemory = NULL;

// Mutex protecting the bitset
static Mutex bitsetLock;

// Initialize the block allocator by reserving a large range of memory, but
// without commiting any pages in it.
static inline void InitBlock()
{
#ifdef _WIN32
	void *addr = VirtualAlloc(NULL, ALLOC_SIZE, MEM_RESERVE, PAGE_READWRITE);
	if (!addr)
		Error("Failed to reserve %d bytes of memory", ALLOC_SIZE);
	blockMemory = addr;
#else
	// We are only guaranteed an address aligned to page size, so we need to
	// align it manually to MemPoolImpl::BLOCK_SIZE.
#ifdef HAVE_MEM_OVERCOMMIT
	char *base = static_cast<char *>(anonymous_mmap(NULL, ALLOC_SIZE + MemPoolImpl::BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_NORESERVE));
#else
	char *base = static_cast<char *>(anonymous_mmap(NULL, ALLOC_SIZE + MemPoolImpl::BLOCK_SIZE, PROT_READ, 0));
#endif
	if (base == MAP_FAILED)
		Error("Failed to reserve %d bytes of memory", ALLOC_SIZE);

	// Get aligned address
	char *aligned = reinterpret_cast<char *>(PAD(reinterpret_cast<intptr_t>(base), MemPoolImpl::BLOCK_SIZE));

	// Unmap prolog and epilog
	if (base != aligned)
		munmap(base, aligned - base);
	munmap(aligned + ALLOC_SIZE, MemPoolImpl::BLOCK_SIZE + base - aligned);

	blockMemory = aligned;
#endif
}

void *MemVirtual::AllocBlock()
{
	// Make sure the block allocation system has been initialized
	if (!blockMemory)
		InitBlock();

	// Find a word in the bitset with an unset bit, and set it
	int index = 0;
	bitsetLock.Lock();
	for (; bitsetStart != bitset + BITSET_SIZE; bitsetStart++) {
		uint32_t &value = *bitsetStart;
		if (~value == 0)
			continue;
		index = IntFFS(~value);
		value |= 1 << index;
		break;
	}
	bitsetLock.Unlock();

	// Check if block memory is full
	if (bitsetStart == bitset + BITSET_SIZE)
		Error("Failed to allocate %d bytes of memory", MemPoolImpl::BLOCK_SIZE);

	// Get a pointer to the memory block
	void *block = static_cast<char *>(blockMemory) + ((bitsetStart - bitset) * 32 + index) * MemPoolImpl::BLOCK_SIZE;

	// Commit the memory pages for this block
#ifdef _WIN32
	void *addr = VirtualAlloc(block, MemPoolImpl::BLOCK_SIZE, MEM_COMMIT, PAGE_READWRITE);
	if (!addr)
		Error("Failed to allocate %d bytes of memory", MemPoolImpl::BLOCK_SIZE);
#elif !defined(HAVE_MEM_OVERCOMMIT)
	void *ret = anonymous_mmap(block, MemPoolImpl::BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_FIXED);
	if (ret == MAP_FAILED)
		Error("Failed to allocate %d bytes of memory", MemPoolImpl::BLOCK_SIZE);
#endif

	// Return pointer
	return block;
}

void MemVirtual::FreeBlock(void *block)
{
	int index = (static_cast<char *>(block) - static_cast<char *>(blockMemory)) / MemPoolImpl::BLOCK_SIZE;
	bitsetLock.Lock();
	bitsetStart = std::min(bitsetStart, bitset + index / 32);
	bitset[index / 32] &= ~(1 << (index % 32));
	bitsetLock.Unlock();

	// Uncommit pages for this block
#ifdef _WIN32
	if (!VirtualFree(block, MemPoolImpl::BLOCK_SIZE, MEM_DECOMMIT))
		Error("Failed to VirtualFree %d bytes of memory", MemPoolImpl::BLOCK_SIZE);
#else
#ifdef HAVE_MEM_OVERCOMMIT
	void *ret = anonymous_mmap(block, MemPoolImpl::BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_NORESERVE);
#else
	void *ret = anonymous_mmap(block, MemPoolImpl::BLOCK_SIZE, PROT_READ, MAP_FIXED);
#endif
	if (ret == MAP_FAILED)
		Error("Failed to decommit %d bytes of memory", MemPoolImpl::BLOCK_SIZE);
#endif
}

bool MemVirtual::IsBlockPtr(void *ptr)
{
	return blockMemory && ptr >= blockMemory && ptr < static_cast<char *>(blockMemory) + ALLOC_SIZE;
}
