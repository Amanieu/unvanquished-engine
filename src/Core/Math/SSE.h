//@@COPYRIGHT@@

// SSE helper functions

#include <xmmintrin.h>

#ifdef __SSE2__
#include <emmintrin.h>

#ifdef __SSE3__
#include <pmmintrin.h>
#endif
#endif

// Vector masks
inline __m128 SSE_IntVec(int x, int y, int z, int w)
{
#ifdef __SSE2__
	return _mm_castsi128_ps(_mm_setr_epi32(x, y, z, w));
#else
	union {
		int data[4];
		__m128 vec;
	} u;
	u.data[0] = x;
	u.data[1] = y;
	u.data[2] = z;
	u.data[3] = w;
	return vec;
#endif
}
#define SSE_SIGNMASK(x, y, z, w) SSE_IntVec(x ? 0x80000000 : 0, y ? 0x80000000 : 0, z ? 0x80000000 : 0, w ? 0x80000000 : 0)
#define SSE_NSIGNMASK(x, y, z, w) SSE_IntVec(x ? ~0x80000000 : ~0, y ? ~0x80000000 : ~0, z ? ~0x80000000 : ~0, w ? ~0x80000000 : ~0)
#define SSE_MASK(x, y, z, w) SSE_IntVec(x ? ~0 : 0, y ? ~0 : 0, z ? ~0 : 0, w ? ~0 : 0)

// Reorder the contents of a SSE register. Use _mm_shuffle_ps instead if the
// original register won't be used again.
#ifdef __SSE2__
// Using pshufd instead of shufps can help avoid a register move, but causes
// a reformatting delay in some CPUs because of the cast.
#define shuffle_ps(m, shuffle) (_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(m), shuffle)))
#else
#define shuffle_ps(m, shuffle) (_mm_shuffle_ps(m, m, shuffle))
#endif

// Dot product for 4 element vectors, result is in the first element
inline __m128 dot4_ps(__m128 a, __m128 b)
{
	__m128 xyzw = _mm_mul_ps(a, b);
	__m128 yxwz = shuffle_ps(xyzw, _MM_SHUFFLE(2, 3, 0, 1));
	__m128 xy2zw2 = _mm_add_ps(xyzw, yxwz);
#ifdef __SSE2__
	__m128 zw4 = shuffle_ps(xy2zw2, _MM_SHUFFLE(2, 2, 2, 2));
#else
	__m128 zw4 = _mm_movehl_ps(xy2zw2, xy2zw2);
#endif
	return _mm_add_ss(xy2zw2, zw4);
}

// Same as above but ignoring the 4th element
inline __m128 dot3_ps(__m128 a, __m128 b)
{
	__m128 xyzw = _mm_mul_ps(a, b);
	__m128 yxwz = shuffle_ps(xyzw, _MM_SHUFFLE(2, 3, 0, 1));
	__m128 xy = _mm_add_ss(xyzw, yxwz);
	__m128 zzww = _mm_movehl_ps(xyzw, xyzw);
	return _mm_add_ss(xy, zzww);
}

// Same as above but ignoring the 3rd and 4th elements
inline __m128 dot2_ps(__m128 a, __m128 b)
{
	__m128 xyzw = _mm_mul_ps(a, b);
	__m128 yxwz = shuffle_ps(xyzw, _MM_SHUFFLE(3, 2, 0, 1));
	return _mm_add_ss(xyzw, yxwz);
}

// Inverse function, operates on all 4 elements
inline __m128 rcp_ps(__m128 x)
{
	// On newer CPUs, _mm_div_ps is faster
	return _mm_div_ps(_mm_set1_ps(1), x);
	//__m128 r = _mm_rcp_ps(x);
	//return _mm_sub_ps(_mm_add_ps(r, r), _mm_mul_ps(_mm_mul_ps(r, x), r));
}

// Division, operates on all 4 elements
inline __m128 div_ps(__m128 x, __m128 y)
{
	// On newer CPUs, _mm_div_ps is faster
	return _mm_div_ps(x, y);
	//return _mm_mul_ps(x, rcp_ps(y));
}

// Inverse square root function, operates on all 4 elements
inline __m128 rsqrt_ps(__m128 x)
{
	__m128 r = _mm_rsqrt_ps(x);
	r = _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5), r),
	               _mm_sub_ps(_mm_set1_ps(3), _mm_mul_ps(_mm_mul_ps(x, r), r)));
	return r;
}

// Square root function, operates on all 4 elements
inline __m128 sqrt_ps(__m128 x)
{
	return _mm_mul_ps(x, rsqrt_ps(x));
}

#include "SSEMath.h"
