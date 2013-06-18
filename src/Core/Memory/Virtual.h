//@@COPYRIGHT@@

// Virtual memory management

namespace MemVirtual {

#ifndef _WIN32
// Store the size of a mmap allocation in a hash table. The allocation size can
// then be retrieved using ReleaseMmap, which also removes it from the table.
void RegisterMmap(void *addr, size_t size);
size_t ReleaseMmap(void *addr);
#endif

// Allocate a 64KB block of memory. This is more efficient than allocating
// another size.
__malloc void *AllocBlock();

// Free a 64KB block of memory allocated with AllocBlock
void FreeBlock(void *block);

// Check if a pointer is inside a block
bool IsBlockPtr(void *ptr);

}

#ifndef _WIN32
// mmap wrapper to allocate anonymous memory
inline void *anonymous_mmap(void *addr, size_t size, int prot, int flags)
{
#if defined(MAP_ANONYMOUS) || defined(MAP_ANON)
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif
	return mmap(addr, size, prot, flags | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#else
	static int fd = open("/dev/zero", O_RDWR);
	return mmap(addr, size, prot, flags | MAP_PRIVATE, fd, 0);
#endif
}
#endif
