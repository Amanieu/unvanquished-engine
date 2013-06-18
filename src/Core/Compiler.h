//@@COPYRIGHT@@

// Abstract away compiler-specific stuff

#if __GNUC__ >= 4

// Emit a nice warning when a function is used
#define __deprecated __attribute__((deprecated))

// Indicates that a function does not return
#define __noreturn __attribute__((noreturn))

// Pure functions depend only on their arguments, do not modify any global
// state, and have no effect other than their return value
#define __pure __attribute__((pure))

// Expect printf-style arguments for a function: a is the index of the format
// string, and b is the index of the first variable argument
#define __printf(a, b) __attribute__((format(printf, a, b)))

// Allow this function to be used from other modules
#ifdef _WIN32
#define __dllexport __attribute__((dllexport))
#else
#define __dllexport __attribute__((visibility("default")))
#endif

// Mark that this function is imported from another module
#ifdef _WIN32
#define __dllimport __attribute__((dllimport))
#else
#define __dllimport __attribute__((visibility("default")))
#endif

// Marks that this function will return a pointer that is not aliased to any
// other pointer
#define __malloc __attribute__((malloc))

// Various x86 calling conventions
#ifdef __i386__
// Already defined in MinGW, so just leave them as they are
#ifndef _WIN32
#define __cdecl __attribute__((cdecl))
#define __stdcall __attribute__((stdcall))
#define __fastcall __attribute__((fastcall, sseregparm))
#endif
#else
#define __cdecl
#define __stdcall
#define __fastcall
#endif

// Instructs the CPU to load memory into the cache to avoid expensive cache
// misses. Issue a prefetch a few lines before the data is actually accessed.
#define __prefetch(addr) __builtin_prefetch(addr, 1)

// Returns whether the variable is a known constant at compile time.
#define __constant(x) __builtin_constant_p(x)

// Ensure a C++ object is constructed before any other objects. Note that these
// objects must not allocate any memory, since the memory system may not have
// been initialized yet.
#define __init_early(x) x __attribute__((init_priority(101)))

#if __GNUC_MINOR__ >= 3 || __GNUC__ > 4

// A cold function is rarely called, so branches that lead to one are assumed
// to be unlikely
#define __cold __attribute__((__cold__))

#else
#define __cold
#endif

#if __GNUC_MINOR__ >= 5 || __GNUC__ > 4

// Marks this code location as unreachable
#define __unreachable() __builtin_unreachable()

// Gives the compiler a hint that x will always be true. The expression x should
// not have any side effects.
#define __assume(x) do {if (!(x)) __builtin_unreachable();} while (false)

#else
#define __unreachable() do {} while (false)
#define __assume(x) do {} while (false)
#endif

// Remove this once compiler properly supports C++11
#define thread_local __thread
#define alignas(x) __attribute__((aligned(x)))

#elif _MSC_VER >= 1500

#define __deprecated __declspec(deprecated)
#define __noreturn __declspec(noreturn)
#define __pure
#define __printf(a, b)
#define __dllexport __declspec(dllexport)
#define __dllimport __declspec(dllimport)
#define __malloc __declspec(restrict)
#ifdef __SSE__
#define __prefetch(addr) _mm_prefetch(addr, _MM_HINT_T0)
#else
#define __prefetch(addr) do {} while (false)
#endif
#define __constant(x) false
// DO NOT USE THIS IN A HEADER FILE
#define __init_early(x) __pragma(init_seg(lib)) x
#define __cold
#define __unreachable() __assume(false)

// Fix up C99 support
#define __func__ __FUNCTION__

// Remove this once compiler properly supports C++11
#define thread_local __declspec(thread)
#define alignas(x) __declspec(align(x))

#else

#error "Unsupported compiler"

// To add support for a new compiler, you must define the following macros
#define __dllexport
#define __dllimport
#define __cdecl
#define __stdcall
#define __fastcall
#define __init_early(x)
#define __thread

// You can also define these if they are available for your compiler
#define __deprecated
#define __noreturn
#define __pure
#define __printf(a, b)
#define __malloc
#define __prefetch(addr) 0
#define __constant(x) false
#define __cold
#define __unreachable() do {} while (false)
#define __assume(x) do {} while (false)

#endif
