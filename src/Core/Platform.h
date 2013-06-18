//@@COPYRIGHT@@

// Fix platform defines
#if defined(_M_IX86) && !defined(__i386__)
#define __i386__
#elif defined(_M_X64) && !defined(__x86_64__)
#define __x86_64__
#elif defined(_M_IA64) && !defined(__ia64__)
#define __ia64__
#elif defined(__powerpc__) && !defined(__ppc__)
#define __ppc__
#elif defined(__powerpc64__) && !defined(__ppc64__)
#define __ppc64__
#elif defined(__sparc) && !defined(__sparc__)
#define __sparc__
#endif

// MSVC SSE support
#if _M_IX86_FP >= 1
#define __SSE__
#endif
#if _M_IX86_FP >= 2
#define __SSE2__
#endif

// Platform-specific configuration

#ifdef _WIN32

#define SYS_LITTLE_ENDIAN
#define DLL_EXT ".dll"

#elif defined(__APPLE__)

#if defined(__powerpc__) || defined(__powerpc64__)
#define SYS_BIG_ENDIAN
#elif defined(__i386__) || defined(__x86_64__)
#define SYS_LITTLE_ENDIAN
#endif

#define DLL_EXT ".dylib"

#elif defined(__linux)

#include <endian.h>

#if __FLOAT_WORD_ORDER == __BIG_ENDIAN
#define SYS_BIG_ENDIAN
#else
#define SYS_LITTLE_ENDIAN
#endif

#define DLL_EXT ".so"

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)

#ifndef __BSD__
#define __BSD__
#endif

#include <machine/endian.h>

#if BYTE_ORDER == BIG_ENDIAN
#define SYS_BIG_ENDIAN
#else
#define SYS_LITTLE_ENDIAN
#endif

#define DLL_EXT ".so"

#elif defined(__sun)

#include <sys/byteorder.h>

#ifdef _BIG_ENDIAN
#define SYS_BIG_ENDIAN
#elif defined(_LITTLE_ENDIAN)
#define SYS_LITTLE_ENDIAN
#endif

#define DLL_EXT ".so"

#elif defined(__sgi)

#define SYS_BIG_ENDIAN

#define DLL_EXT ".so"

#else

#error "Platform not supported"

#endif


// Endianness handling

inline int16_t Swap16(int16_t x)
{
	return (x >> 8) | (x << 8);
}

inline int32_t Swap32(int32_t x)
{
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 3
	return __builtin_bswap32(x);
#elif _MSC_VER
	return _byteswap_ulong(x);
#else
	return (x << 24) | ((x << 8) & 0xff0000) | ((x >> 8) & 0xff00) | (x >> 24);
#endif
}

inline int64_t Swap64(int64_t x)
{
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 3
	return __builtin_bswap64(x);
#elif _MSC_VER
	return _byteswap_uint64(x);
#else
	return  (x << 56) |
	       ((x << 40) & 0xff000000000000ULL) |
	       ((x << 24) & 0xff0000000000ULL) |
	       ((x <<  8) & 0xff00000000ULL) |
	       ((x >>  8) & 0xff000000ULL) |
	       ((x >> 24) & 0xff0000ULL) |
	       ((x >> 40) & 0xff00ULL) |
	        (x >> 56);
#endif
}

inline float SwapFloat(float x)
{
	floatInt_t fi;
	fi.f = x;
	fi.i = Swap32(fi.i);
	return fi.f;
}

#ifdef SYS_LITTLE_ENDIAN

#define Little16(x) (x)
#define Little32(x) (x)
#define Little64(x) (x)
#define LittleFloat(x) (x)
#define Big16(x) Swap16(x)
#define Big32(x) Swap32(x)
#define Big64(x) Swap64(x)
#define BigFloat(x) SwapFloat(x)

#else

#define Little16(x) Swap16(x)
#define Little32(x) Swap32(x)
#define Little64(x) Swap64(x)
#define LittleFloat(x) SwapFloat(x)
#define Big16(x) (x)
#define Big32(x) (x)
#define Big64(x) (x)
#define BigFloat(x) (x)

#endif
