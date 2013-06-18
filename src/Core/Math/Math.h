//@@COPYRIGHT@@

// General math functions

#ifdef __SSE__
#include "SSE.h"
#endif

// Pi constant
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Use float versions of all math functions
#define cos(x) cosf(x)
#define sin(x) sinf(x)
#define tan(x) tanf(x)
#define asin(x) asinf(x)
#define acos(x) acosf(x)
#define atan(x) atanf(x)
#define atan2(x, y) atan2f(x, y)
#define sqrt(x) sqrtf(x)
#define fabs(x) fabsf(x)
#define copysign(x, y) copysignf(x, y)
#define ceil(x) ceilf(x)
#define floor(x) floorf(x)
#define exp(x) expf(x)
#define fmod(x, y) fmodf(x, y)
#define frexp(x, y) frexpf(x, y)
#define ldexp(x, y) ldexpf(x, y)
#define log(x) logf(x)
#define log10(x) log10f(x)
#define pow(x, y) powf(x, y)

// Degrees <-> Radians conversions
inline float DegToRad(float x)
{
	return (x * M_PI) / 180;
}
inline float RadToDeg(float x)
{
	return (x * 180) / M_PI;
}

// Determine whether an integer is a power of 2. Returns true for 0.
inline bool IsPowerOf2(unsigned x)
{
	return (x & (x - 1)) == 0;
}

// Get the next highest power of 2 greater than or equal to x. Result for 0 is
// 0.
inline int NextPowerOf2(uint32_t x)
{
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return x;
}

// Calculates the log base 2 of an integer, aka the index of the last set bit.
// Result for 0 is undefined.
inline int IntLog2(uint32_t x)
{
	__assume(x != 0);
#ifdef _MSC_VER
	unsigned long index;
	_BitScanReverse(&index, x);
	return index;
#elif __GNUC__
	return __builtin_clz(x) ^ 31;
#else
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	x >>= 1;
	x -= (x >> 1) & 0x55555555;
	x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
	x = ((x >> 4) + x) & 0x0f0f0f0f;
	x += (x >> 8);
	x += (x >> 16);
	return x & 63;
#endif
}

// Find the index of the first set bit. Result for 0 is undefined.
inline int IntFFS(uint32_t x)
{
	__assume(x != 0);
#ifdef _MSC_VER
	unsigned long index;
	_BitScanForward(&index, x);
	return index;
#elif __GNUC__
	return __builtin_ffs(x) - 1;
#else
	return IntLog2(x & -x);
#endif
}

// Conversions between 32-bit float and 16-bit half float. NaN, infinity and
// denormals are clamped to normal values.
inline int16_t FloatToHalf(float value)
{
	floatInt_t fi;
	int out;
	fi.f = std::min(fabs(value), 65504.0f);
	out = fi.i + 0x48000000;
	out += ((out >> 13) & 1) + 0x0FFF;
	out >>= 13;
	fi.f = value;
	out |= (fi.i & 0x80000000) >> 16;
	return (fabs(value) >= 6.10351562e-05) ? out : 0;
}
inline float HalfToFloat(int16_t value)
{
	floatInt_t fi;
	fi.i = (value & 0x8000) << 16;
	fi.i |= ((value & 0x7C00) + 0x1C000) << 13;
	fi.i |= (value & 0x03FF) << 13;
	return (value & 0x7C00) ? fi.f : 0;
}

// Inverse square root
inline float rsqrt(float number)
{
	// Just trust the compiler to optimize this properly. If it doesn't produce
	// optimal code, then replace this with some platform-specific code.
	return 1 / sqrt(number);
}

// Fast sin() function, but only valid in the range [-PI/2, PI/2]
inline float fastsin(float a)
{
	float s, t;

	s = a * a;
	t = -2.50521083854417187751e-8;
	t *= s;
	t += 2.75573192239858906526e-6;
	t *= s;
	t += -1.98412698412698412698e-4;
	t *= s;
	t += 8.33333333333333333333e-3;
	t *= s;
	t += -1.66666666666666666667e-1;
	t *= s;
	t *= a;
	t += a;

	return t;
}

// Fast cos() function, but only valid in the range [-PI/2, PI/2]
inline float fastcos(float a)
{
	float s, t;

	s = a * a;
	t = 2.08767569878680989792e-9;
	t *= s;
	t += -2.75573192239858906526e-7;
	t *= s;
	t += 2.48015873015873015873e-5;
	t *= s;
	t += -1.38888888888888888889e-3;
	t *= s;
	t += 4.16666666666666666667e-2;
	t *= s;
	t -= 0.5;
	t *= s;
	t += 1;

	return t;
}

#ifdef _MSC_VER
// MSVC only has copysign for doubles
inline float copysignf(float x, float y)
{
	if (y < 0)
		return -fabs(x);
	else
		return fabs(x);
}
#endif

// Initialize the math library
namespace Math {
inline void Init()
{
#ifdef __SSE__
#ifdef __SSE3__
	int dazFlag = _MM_DENORMALS_ZERO_ON;
#else
	int dazFlag = 0;
#endif

	// Round to nearest, all exceptions masked, denormals are zero, flush to zero
	_mm_setcsr(_MM_MASK_MASK | _MM_FLUSH_ZERO_ON | dazFlag | _MM_ROUND_NEAREST);
#endif
}
}
