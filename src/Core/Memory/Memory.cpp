//@@COPYRIGHT@@

// Various memory management utilities

// Shortcuts for some common strings
static const char nullString[] = "";
static const char numberString[][2] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};

const char *CopyString(const char *string)
{
	// See if we can use one of the static strings
	if (!string[0])
		return nullString;
	if (isdigit(string[0]) && !string[1])
		return numberString[string[0] - '0'];

	// Allocate space and copy the string
	char *newString = new char[strlen(string) + 1];
	strcpy(newString, string);
	return newString;
}

void FreeString(const char *string)
{
	// NULL string
	if (!string)
		return;

	// Static strings
	if (!string[0])
		return;
	if (isdigit(string[0]) && !string[1])
		return;

	// Normal string
	delete[] string;
}

// General allocator size classes
static const unsigned int sizeClasses[] = {8, 16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512, 768, 1024};
static const int numSizeClasses = sizeof(sizeClasses) / sizeof(*sizeClasses);

// Whether all global constructors have been run yet
static bool memInit = false;

void Memory::Init()
{
	// We just set a flag now that we are sure that all MemPools have been
	// constructed.
	memInit = true;
}

// Aligned system malloc
static inline void *aligned_malloc(size_t size)
{
#ifdef __SSE__
	return _mm_malloc(size, 16);
#elif _WIN32
	return _aligned_malloc(size, 16);
#else
	void *ptr;
	if (posix_memalign(&ptr, 16, size))
		return NULL;
	else
		return ptr;
#endif
}
static inline void aligned_free(void *ptr)
{
#ifdef __SSE__
	_mm_free(ptr);
#elif _WIN32
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}

// Find a size class for a size
static inline int FindSizeClass(size_t size)
{
	const unsigned int *ptr = std::lower_bound(sizeClasses, sizeClasses + numSizeClasses, size);
	return ptr - sizeClasses;
}

void *MemAlloc(size_t size)
{
	if (size > sizeClasses[numSizeClasses - 1] || !memInit) {
		void *ptr = aligned_malloc(size);
		if (!ptr)
			Error("Out of memory (Tried to allocate %d bytes)", static_cast<int>(size));
		return ptr;
	}

	int sizeClass = FindSizeClass(size);
	switch (sizeClass) {
	case 0:
		return MemPool<8>::Alloc();
	case 1:
		return MemPool<16>::Alloc();
	case 2:
		return MemPool<24>::Alloc();
	case 3:
		return MemPool<32>::Alloc();
	case 4:
		return MemPool<48>::Alloc();
	case 5:
		return MemPool<64>::Alloc();
	case 6:
		return MemPool<96>::Alloc();
	case 7:
		return MemPool<128>::Alloc();
	case 8:
		return MemPool<192>::Alloc();
	case 9:
		return MemPool<256>::Alloc();
	case 10:
		return MemPool<384>::Alloc();
	case 11:
		return MemPool<512>::Alloc();
	case 12:
		return MemPool<768>::Alloc();
	case 13:
		return MemPool<1024>::Alloc();
	NO_DEFAULT
	}

	// Unreachable
	while (true) {}
}

void MemFree(void *ptr)
{
	if (!MemVirtual::IsBlockPtr(ptr)) {
		aligned_free(ptr);
		return;
	}

	int sizeClass = FindSizeClass(MemPoolImpl::GetObjSize(ptr));
	switch (sizeClass) {
	case 0:
		MemPool<8>::Free(ptr);
		return;
	case 1:
		MemPool<16>::Free(ptr);
		return;
	case 2:
		MemPool<24>::Free(ptr);
		return;
	case 3:
		MemPool<32>::Free(ptr);
		return;
	case 4:
		MemPool<48>::Free(ptr);
		return;
	case 5:
		MemPool<64>::Free(ptr);
		return;
	case 6:
		MemPool<96>::Free(ptr);
		return;
	case 7:
		MemPool<128>::Free(ptr);
		return;
	case 8:
		MemPool<192>::Free(ptr);
		return;
	case 9:
		MemPool<256>::Free(ptr);
		return;
	case 10:
		MemPool<384>::Free(ptr);
		return;
	case 11:
		MemPool<512>::Free(ptr);
		return;
	case 12:
		MemPool<768>::Free(ptr);
		return;
	case 13:
		MemPool<1024>::Free(ptr);
		return;
	NO_DEFAULT
	}

	// Unreachable
	while (true) {}
}
