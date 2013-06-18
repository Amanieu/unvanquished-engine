//@@COPYRIGHT@@

// SSE math functions

// Based on these libraries:
// gmath SSE library: http://github.com/raedwulf/gmath/
// cephes math library: http://www.netlib.org/cephes/

inline __m128 fastsin_ps(__m128 a)
{
	__m128 s, t;

	s = _mm_mul_ps(a, a);
	t = _mm_set1_ps(-2.50521083854417187751e-8);
	t = _mm_mul_ps(t, s);
	t = _mm_add_ps(t, _mm_set1_ps(2.75573192239858906526e-6));
	t = _mm_mul_ps(t, s);
	t = _mm_add_ps(t, _mm_set1_ps(-1.98412698412698412698e-4));
	t = _mm_mul_ps(t, s);
	t = _mm_add_ps(t, _mm_set1_ps(8.33333333333333333333e-3));
	t = _mm_mul_ps(t, s);
	t = _mm_add_ps(t, _mm_set1_ps(-1.66666666666666666667e-1));
	t = _mm_mul_ps(t, s);
	t = _mm_mul_ps(t, a);
	t = _mm_add_ps(t, a);

	return t;
}

inline __m128 fastcos_ps(__m128 a)
{
	__m128 s, t;

	s = _mm_mul_ps(a, a);
	t = _mm_set1_ps(2.08767569878680989792e-9);
	t = _mm_mul_ps(t, s);
	t = _mm_add_ps(t, _mm_set1_ps(-2.75573192239858906526e-7));
	t = _mm_mul_ps(t, s);
	t = _mm_add_ps(t, _mm_set1_ps(2.48015873015873015873e-5));
	t = _mm_mul_ps(t, s);
	t = _mm_add_ps(t, _mm_set1_ps(-1.38888888888888888889e-3));
	t = _mm_mul_ps(t, s);
	t = _mm_add_ps(t, _mm_set1_ps(4.16666666666666666667e-2));
	t = _mm_mul_ps(t, s);
	t = _mm_sub_ps(t, _mm_set1_ps(0.5));
	t = _mm_mul_ps(t, s);
	t = _mm_add_ps(t, _mm_set1_ps(1));

	return t;
}

// Helper macros for SSE2 emulation using MMX
#ifndef __SSE2__

union xmm_mm_union {
	__m128 xmm;
	__m64 mm[2];
};

#define COPY_MM_TO_XMM(mm0_, mm1_, xmm_) \
	do { \
		xmm_mm_union u; \
		u.mm[0] = mm0_; \
		u.mm[1] = mm1_; \
		xmm_ = u.xmm; \
	} while (false)

#endif

inline __m128 sin_ps(__m128 x)
{
	__m128 xmm1, xmm2, xmm3, sign_bit, y;

#ifdef __SSE2__
	__m128i emm0, emm2;
#else
	__m64 mm0, mm1, mm2, mm3;
#endif

	sign_bit = x;

	// Take the absolute value
	x = _mm_and_ps(x, SSE_NSIGNMASK(1, 1, 1, 1));

	// Extract the sign bit (upper one)
	sign_bit = _mm_and_ps(sign_bit, SSE_SIGNMASK(1, 1, 1, 1));

	// Scale by 4/Pi
	y = _mm_mul_ps(x, _mm_set1_ps(4 / M_PI));

#ifdef __SSE2__
	// Store the integer part of y in emm2
	emm2 = _mm_cvttps_epi32(y);

	// j=(j+1) & (~1) (see the cephes sources)
	emm2 = _mm_add_epi32(emm2, _mm_set1_epi32(1));
	emm2 = _mm_and_si128(emm2, _mm_set1_epi32(~1));
	y = _mm_cvtepi32_ps(emm2);

	// Get the swap sign flag
	emm0 = _mm_and_si128(emm2, _mm_set1_epi32(4));
	emm0 = _mm_slli_epi32(emm0, 29);

	// Get the polynom selection mask
	// Both branches will be computed.
	emm2 = _mm_and_si128(emm2, _mm_set1_epi32(2));
	emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());

	__m128 swap_sign_bit = _mm_castsi128_ps(emm0);
	__m128 poly_mask = _mm_castsi128_ps(emm2);
	sign_bit = _mm_xor_ps(sign_bit, swap_sign_bit);
#else
	// Store the integer part of y in mm0:mm1
	xmm2 = _mm_movehl_ps(_mm_setzero_ps(), y);
	mm2 = _mm_cvttps_pi32(y);
	mm3 = _mm_cvttps_pi32(xmm2);

	// j=(j+1) & (~1) (see the cephes sources)
	mm2 = _mm_add_pi32(mm2, _mm_set1_pi32(1));
	mm3 = _mm_add_pi32(mm3, _mm_set1_pi32(1));
	mm2 = _mm_and_si64(mm2, _mm_set1_pi32(~1));
	mm3 = _mm_and_si64(mm3, _mm_set1_pi32(~1));
	y = _mm_cvtpi32x2_ps(mm2, mm3);

	// Get the swap sign flag
	mm0 = _mm_and_si64(mm2, _mm_set1_pi32(4));
	mm1 = _mm_and_si64(mm3, _mm_set1_pi32(4));
	mm0 = _mm_slli_pi32(mm0, 29);
	mm1 = _mm_slli_pi32(mm1, 29);

	// Get the polynom selection mask
	mm2 = _mm_and_si64(mm2, _mm_set1_pi32(2));
	mm3 = _mm_and_si64(mm3, _mm_set1_pi32(2));
	mm2 = _mm_cmpeq_pi32(mm2, _mm_setzero_si64());
	mm3 = _mm_cmpeq_pi32(mm3, _mm_setzero_si64());

	__m128 swap_sign_bit, poly_mask;
	COPY_MM_TO_XMM(mm0, mm1, swap_sign_bit);
	COPY_MM_TO_XMM(mm2, mm3, poly_mask);
	sign_bit = _mm_xor_ps(sign_bit, swap_sign_bit);

	_mm_empty();
#endif

	// The magic pass: "Extended precision modular arithmetic"
	// x = ((x - y * DP1) - y * DP2) - y * DP3;
	xmm1 = _mm_set1_ps(-0.78515625);
	xmm2 = _mm_set1_ps(-2.4187564849853515625e-4);
	xmm3 = _mm_set1_ps(-3.77489497744594108e-8);
	xmm1 = _mm_mul_ps(y, xmm1);
	xmm2 = _mm_mul_ps(y, xmm2);
	xmm3 = _mm_mul_ps(y, xmm3);
	x = _mm_add_ps(x, xmm1);
	x = _mm_add_ps(x, xmm2);
	x = _mm_add_ps(x, xmm3);

	__m128 z = _mm_mul_ps(x, x);

	// Evaluate the first polynom  (0 <= x <= Pi/4)
	y = _mm_set1_ps(2.443315711809948e-5);
	y = _mm_mul_ps(y, z);
	y = _mm_add_ps(y, _mm_set1_ps(-1.388731625493765e-3));
	y = _mm_mul_ps(y, z);
	y = _mm_add_ps(y, _mm_set1_ps(4.166664568298827e-2));
	y = _mm_mul_ps(y, z);
	y = _mm_mul_ps(y, z);
	y = _mm_sub_ps(y, _mm_mul_ps(z, _mm_set1_ps(0.5)));
	y = _mm_add_ps(y, _mm_set1_ps(1));

	// Evaluate the second polynom  (Pi/4 <= x <= 0)
	__m128 y2 = _mm_set1_ps(-1.9515295891e-4);
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_add_ps(y2, _mm_set1_ps(8.3321608736e-3));
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_add_ps(y2, _mm_set1_ps(-1.6666654611e-1));
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_mul_ps(y2, x);
	y2 = _mm_add_ps(y2, x);

	// Select the correct result from the two polynoms
	xmm3 = poly_mask;
	y2 = _mm_and_ps(xmm3, y2);
	y = _mm_andnot_ps(xmm3, y);
	y = _mm_add_ps(y,y2);

	// Update the sign
	y = _mm_xor_ps(y, sign_bit);

	return y;
}

inline __m128 cos_ps(__m128 x)
{
	__m128 xmm1, xmm2, xmm3, y;

#ifdef __SSE2__
	__m128i emm0, emm2;
#else
	__m64 mm0, mm1, mm2, mm3;
#endif

	// Take the absolute value
	x = _mm_and_ps(x, SSE_NSIGNMASK(1, 1, 1, 1));

	// Scale by 4/Pi
	y = _mm_mul_ps(x, _mm_set1_ps(4 / M_PI));

#ifdef __SSE2__
	// Store the integer part of y in emm2
	emm2 = _mm_cvttps_epi32(y);

	// j=(j+1) & (~1) (see the cephes sources)
	emm2 = _mm_add_epi32(emm2, _mm_set1_epi32(1));
	emm2 = _mm_and_si128(emm2, _mm_set1_epi32(~1));
	y = _mm_cvtepi32_ps(emm2);

	emm2 = _mm_sub_epi32(emm2, _mm_set1_epi32(2));

	// Get the swap sign flag
	emm0 = _mm_andnot_si128(emm2, _mm_set1_epi32(4));
	emm0 = _mm_slli_epi32(emm0, 29);

	// Get the polynom selection mask
	// Both branches will be computed.
	emm2 = _mm_and_si128(emm2, _mm_set1_epi32(2));
	emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());

	__m128 sign_bit = _mm_castsi128_ps(emm0);
	__m128 poly_mask = _mm_castsi128_ps(emm2);
#else
	// Store the integer part of y in mm0:mm1
	xmm2 = _mm_movehl_ps(_mm_setzero_ps(), y);
	mm2 = _mm_cvttps_pi32(y);
	mm3 = _mm_cvttps_pi32(xmm2);

	// j=(j+1) & (~1) (see the cephes sources)
	mm2 = _mm_add_pi32(mm2, _mm_set1_pi32(1));
	mm3 = _mm_add_pi32(mm3, _mm_set1_pi32(1));
	mm2 = _mm_and_si64(mm2, _mm_set1_pi32(~1));
	mm3 = _mm_and_si64(mm3, _mm_set1_pi32(~1));
	y = _mm_cvtpi32x2_ps(mm2, mm3);

	mm2 = _mm_sub_pi32(mm2, _mm_set1_pi32(2));
	mm3 = _mm_sub_pi32(mm3, _mm_set1_pi32(2));

	// Get the swap sign flag
	mm0 = _mm_andnot_si64(mm2, _mm_set1_pi32(4));
	mm1 = _mm_andnot_si64(mm3, _mm_set1_pi32(4));
	mm0 = _mm_slli_pi32(mm0, 29);
	mm1 = _mm_slli_pi32(mm1, 29);

	// Get the polynom selection mask
	mm2 = _mm_and_si64(mm2, _mm_set1_pi32(2));
	mm3 = _mm_and_si64(mm3, _mm_set1_pi32(2));
	mm2 = _mm_cmpeq_pi32(mm2, _mm_setzero_si64());
	mm3 = _mm_cmpeq_pi32(mm3, _mm_setzero_si64());

	__m128 sign_bit, poly_mask;
	COPY_MM_TO_XMM(mm0, mm1, sign_bit);
	COPY_MM_TO_XMM(mm2, mm3, poly_mask);

	_mm_empty();
#endif

	// The magic pass: "Extended precision modular arithmetic"
	// x = ((x - y * DP1) - y * DP2) - y * DP3;
	xmm1 = _mm_set1_ps(-0.78515625);
	xmm2 = _mm_set1_ps(-2.4187564849853515625e-4);
	xmm3 = _mm_set1_ps(-3.77489497744594108e-8);
	xmm1 = _mm_mul_ps(y, xmm1);
	xmm2 = _mm_mul_ps(y, xmm2);
	xmm3 = _mm_mul_ps(y, xmm3);
	x = _mm_add_ps(x, xmm1);
	x = _mm_add_ps(x, xmm2);
	x = _mm_add_ps(x, xmm3);

	__m128 z = _mm_mul_ps(x, x);

	// Evaluate the first polynom  (0 <= x <= Pi/4)
	y = _mm_set1_ps(2.443315711809948e-5);
	y = _mm_mul_ps(y, z);
	y = _mm_add_ps(y, _mm_set1_ps(-1.388731625493765e-3));
	y = _mm_mul_ps(y, z);
	y = _mm_add_ps(y, _mm_set1_ps(4.166664568298827e-2));
	y = _mm_mul_ps(y, z);
	y = _mm_mul_ps(y, z);
	y = _mm_sub_ps(y, _mm_mul_ps(z, _mm_set1_ps(0.5)));
	y = _mm_add_ps(y, _mm_set1_ps(1));

	// Evaluate the second polynom  (Pi/4 <= x <= 0)
	__m128 y2 = _mm_set1_ps(-1.9515295891e-4);
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_add_ps(y2, _mm_set1_ps(8.3321608736e-3));
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_add_ps(y2, _mm_set1_ps(-1.6666654611e-1));
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_mul_ps(y2, x);
	y2 = _mm_add_ps(y2, x);

	// Select the correct result from the two polynoms
	xmm3 = poly_mask;
	y2 = _mm_and_ps(xmm3, y2);
	y = _mm_andnot_ps(xmm3, y);
	y = _mm_add_ps(y,y2);

	// Update the sign
	y = _mm_xor_ps(y, sign_bit);

	return y;
}

// SSE sincos, gives you a free cosine for your sine
inline std::pair<__m128, __m128> sincos_ps(__m128 x)
{
	__m128 s, c;
	__m128 xmm1, xmm2, xmm3, sign_bit_sin, y;

#ifdef __SSE2__
	__m128i emm0, emm2, emm4;
#else
	__m64 mm0, mm1, mm2, mm3, mm4, mm5;
#endif

	sign_bit_sin = x;

	// Take the absolute value
	x = _mm_and_ps(x, SSE_NSIGNMASK(1, 1, 1, 1));

	// Extract the sign bit (upper one)
	sign_bit_sin = _mm_and_ps(sign_bit_sin, SSE_SIGNMASK(1, 1, 1, 1));

	// Scale by 4/Pi
	y = _mm_mul_ps(x, _mm_set1_ps(4 / M_PI));

#ifdef __SSE2__
	// Store the integer part of y in emm2
	emm2 = _mm_cvttps_epi32(y);

	// j=(j+1) & (~1) (see the cephes sources)
	emm2 = _mm_add_epi32(emm2, _mm_set1_epi32(1));
	emm2 = _mm_and_si128(emm2, _mm_set1_epi32(~1));
	y = _mm_cvtepi32_ps(emm2);

	emm4 = emm2;

	// Get the swap sign flag for the sine
	emm0 = _mm_and_si128(emm2, _mm_set1_epi32(4));
	emm0 = _mm_slli_epi32(emm0, 29);

	// Get the polynom selection mask
	// Both branches will be computed.
	emm2 = _mm_and_si128(emm2, _mm_set1_epi32(2));
	emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());

	__m128 swap_sign_bit_sin = _mm_castsi128_ps(emm0);
	__m128 poly_mask = _mm_castsi128_ps(emm2);
#else
	// Store the integer part of y in mm2:mm3
	xmm3 = _mm_movehl_ps(_mm_setzero_ps(), y);
	mm2 = _mm_cvttps_pi32(y);
	mm3 = _mm_cvttps_pi32(xmm3);

	// j=(j+1) & (~1) (see the cephes sources)
	mm2 = _mm_add_pi32(mm2, _mm_set1_pi32(1));
	mm3 = _mm_add_pi32(mm3, _mm_set1_pi32(1));
	mm2 = _mm_and_si64(mm2, _mm_set1_pi32(~1));
	mm3 = _mm_and_si64(mm3, _mm_set1_pi32(~1));
	y = _mm_cvtpi32x2_ps(mm2, mm3);

	mm4 = mm2;
	mm5 = mm3;

	// Get the swap sign flag for the sine
	mm0 = _mm_and_si64(mm2, _mm_set1_pi32(4));
	mm1 = _mm_and_si64(mm3, _mm_set1_pi32(4));
	mm0 = _mm_slli_pi32(mm0, 29);
	mm1 = _mm_slli_pi32(mm1, 29);

	// Get the polynom selection mask for the sine
	mm2 = _mm_and_si64(mm2, _mm_set1_pi32(2));
	mm3 = _mm_and_si64(mm3, _mm_set1_pi32(2));
	mm2 = _mm_cmpeq_pi32(mm2, _mm_setzero_si64());
	mm3 = _mm_cmpeq_pi32(mm3, _mm_setzero_si64());

	__m128 swap_sign_bit_sin, poly_mask;
	COPY_MM_TO_XMM(mm0, mm1, swap_sign_bit_sin);
	COPY_MM_TO_XMM(mm2, mm3, poly_mask);
#endif

	// The magic pass: "Extended precision modular arithmetic"
	// x = ((x - y * DP1) - y * DP2) - y * DP3;
	xmm1 = _mm_set1_ps(-0.78515625);
	xmm2 = _mm_set1_ps(-2.4187564849853515625e-4);
	xmm3 = _mm_set1_ps(-3.77489497744594108e-8);
	xmm1 = _mm_mul_ps(y, xmm1);
	xmm2 = _mm_mul_ps(y, xmm2);
	xmm3 = _mm_mul_ps(y, xmm3);
	x = _mm_add_ps(x, xmm1);
	x = _mm_add_ps(x, xmm2);
	x = _mm_add_ps(x, xmm3);

#ifdef __SSE2__
	emm4 = _mm_sub_epi32(emm4, _mm_set1_epi32(2));
	emm4 = _mm_andnot_si128(emm4, _mm_set1_epi32(4));
	emm4 = _mm_slli_epi32(emm4, 29);

	__m128 sign_bit_cos = _mm_castsi128_ps(emm4);
#else
	// Get the sign flag for the cosine
	mm4 = _mm_sub_pi32(mm4, _mm_set1_pi32(2));
	mm5 = _mm_sub_pi32(mm5, _mm_set1_pi32(2));
	mm4 = _mm_andnot_si64(mm4, _mm_set1_pi32(4));
	mm5 = _mm_andnot_si64(mm5, _mm_set1_pi32(4));
	mm4 = _mm_slli_pi32(mm4, 29);
	mm5 = _mm_slli_pi32(mm5, 29);

	__m128 sign_bit_cos;
	COPY_MM_TO_XMM(mm4, mm5, sign_bit_cos);

	_mm_empty();
#endif

	sign_bit_sin = _mm_xor_ps(sign_bit_sin, swap_sign_bit_sin);

	__m128 z = _mm_mul_ps(x, x);

	// Evaluate the first polynom  (0 <= x <= Pi/4)
	y = _mm_set1_ps(2.443315711809948e-5);
	y = _mm_mul_ps(y, z);
	y = _mm_add_ps(y, _mm_set1_ps(-1.388731625493765e-3));
	y = _mm_mul_ps(y, z);
	y = _mm_add_ps(y, _mm_set1_ps(4.166664568298827e-2));
	y = _mm_mul_ps(y, z);
	y = _mm_mul_ps(y, z);
	y = _mm_sub_ps(y, _mm_mul_ps(z, _mm_set1_ps(0.5)));
	y = _mm_add_ps(y, _mm_set1_ps(1));

	// Evaluate the second polynom  (Pi/4 <= x <= 0)
	__m128 y2 = _mm_set1_ps(-1.9515295891e-4);
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_add_ps(y2, _mm_set1_ps(8.3321608736e-3));
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_add_ps(y2, _mm_set1_ps(-1.6666654611e-1));
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_mul_ps(y2, x);
	y2 = _mm_add_ps(y2, x);

	// Select the correct result from the two polynoms
	xmm3 = poly_mask;
	__m128 ysin2 = _mm_and_ps(xmm3, y2);
	__m128 ysin1 = _mm_andnot_ps(xmm3, y);
	y2 = _mm_sub_ps(y2, ysin2);
	y = _mm_sub_ps(y, ysin1);

	xmm1 = _mm_add_ps(ysin1, ysin2);
	xmm2 = _mm_add_ps(y, y2);

	// Update the sign
	s = _mm_xor_ps(xmm1, sign_bit_sin);
	c = _mm_xor_ps(xmm2, sign_bit_cos);

	return std::make_pair(s, c);
}

inline __m128 tan_ps(__m128 x)
{
	__m128 xmm0, xmm1;
	__m128 y, z, zz;
	__m128 sign_bit;

#ifdef __SSE2__
	__m128i j, emm0;
#else
	__m64 j0, j1, mm0, mm1;
#endif

	sign_bit = x;

	// Take the absolute value
	x = _mm_and_ps(x, SSE_NSIGNMASK(1, 1, 1, 1));

	// Extract the sign bit (upper one)
	sign_bit = _mm_and_ps(sign_bit, SSE_SIGNMASK(1, 1, 1, 1));

	// Compute x mod PI/4
	y = _mm_mul_ps(_mm_set1_ps(4 / M_PI), x);

#ifdef __SSE2__
	// Integer part of x / (PI/4)
	j = _mm_cvttps_epi32(y);
	y = _mm_cvtepi32_ps(j);

	// Map zeros and singularities to origin
	emm0 = _mm_and_si128(j, _mm_set1_epi32(1));
	emm0 = _mm_cmpeq_epi32(emm0, _mm_set1_epi32(1));
	j = _mm_add_epi32(j, _mm_and_si128(emm0, _mm_set1_epi32(1)));
	xmm0 = _mm_cvtepi32_ps(emm0);
#else
	// Integer part of x / (PI/4)
	xmm0 = _mm_movehl_ps(_mm_setzero_ps(), y);
	j0 = _mm_cvttps_pi32(y);
	j1 = _mm_cvttps_pi32(xmm0);
	y = _mm_cvtpi32x2_ps(j0, j1);

	// Map zeros and singularities to origin
	mm0 = _mm_and_si64(j0, _mm_set1_pi32(1));
	mm1 = _mm_and_si64(j1, _mm_set1_pi32(1));
	mm0 = _mm_cmpeq_pi32(mm0, _mm_set1_pi32(1));
	mm1 = _mm_cmpeq_pi32(mm1, _mm_set1_pi32(1));
	j0 = _mm_add_pi32(j0, _mm_and_si64(mm0, _mm_set1_pi32(1)));
	j1 = _mm_add_pi32(j1, _mm_and_si64(mm1, _mm_set1_pi32(1)));
	xmm0 = _mm_cvtpi32x2_ps(mm0, mm1);
#endif
	y = _mm_add_ps(y, _mm_and_ps(xmm0, _mm_set1_ps(1)));

	// z = ((x - y * DP1) - y * DP2) - y * DP3;
	xmm0 = _mm_add_ps(x, _mm_mul_ps(y, _mm_set1_ps(-0.78515625)));
	xmm1 = _mm_add_ps(xmm0, _mm_mul_ps(y, _mm_set1_ps(-2.4187564849853515625e-4)));
	z = _mm_add_ps(xmm1, _mm_mul_ps(y, _mm_set1_ps(-3.77489497744594108e-8)));

	zz = _mm_mul_ps(z, z);

	// 1.7e-8 relative error in [-pi/4, +pi/4]
	xmm0 = _mm_mul_ps(_mm_set1_ps(9.38540185543e-3), zz);
	xmm1 = _mm_add_ps(xmm0, _mm_set1_ps(3.11992232697e-3));
	xmm0 = _mm_mul_ps(xmm1, zz);
	xmm1 = _mm_add_ps(xmm0, _mm_set1_ps(2.44301354525e-2));
	xmm0 = _mm_mul_ps(xmm1, zz);
	xmm1 = _mm_add_ps(xmm0, _mm_set1_ps(5.34112807005e-2));
	xmm0 = _mm_mul_ps(xmm1, zz);
	xmm1 = _mm_add_ps(xmm0, _mm_set1_ps(1.33387994085e-1));
	xmm0 = _mm_mul_ps(xmm1, zz);
	xmm1 = _mm_add_ps(xmm0, _mm_set1_ps(3.33331568548e-1));
	xmm0 = _mm_mul_ps(xmm1, zz);
	xmm1 = _mm_mul_ps(xmm0, z);
	y = _mm_add_ps(xmm1, z);

	// Calculate the negative reciprocal of y
	xmm1 = rcp_ps(y);
	xmm1 = _mm_xor_ps(xmm1, SSE_SIGNMASK(1, 1, 1, 1));

#ifdef __SSE2__
	emm0 = _mm_and_si128(j, _mm_set1_epi32(2));
	emm0 = _mm_cmpeq_epi32(emm0, _mm_set1_epi32(2));
	xmm0 = _mm_castsi128_ps(emm0);
#else
	mm0 = _mm_and_si64(j0, _mm_set1_pi32(2));
	mm1 = _mm_and_si64(j1, _mm_set1_pi32(2));
	mm0 = _mm_cmpeq_pi32(mm0, _mm_set1_pi32(2));
	mm1 = _mm_cmpeq_pi32(mm1, _mm_set1_pi32(2));
	COPY_MM_TO_XMM(mm0, mm1, xmm0);
#endif
	y = _mm_or_ps(_mm_andnot_ps(xmm0, y), _mm_and_ps(xmm0, xmm1));

	// Do the sign
	y = _mm_xor_ps(sign_bit, y);

	return y;
}

inline __m128 atan_ps(__m128 x)
{
	__m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;
	__m128 y, z;
	__m128 sign_bit;

	sign_bit = x;

	// Take the absolute value
	x = _mm_and_ps(x, SSE_NSIGNMASK(1, 1, 1, 1));

	// Extract the sign bit (upper one)
	sign_bit = _mm_and_ps(sign_bit, SSE_SIGNMASK(1, 1, 1, 1));

	// Range reduction
	xmm0 = _mm_cmpgt_ps(x, _mm_set1_ps(2.414213562373095)); // tan(3 * M_PI / 8)
	xmm4 = _mm_cmpgt_ps(x, _mm_set1_ps(0.414213562373095)); // tan(M_PI / 8)
	xmm1 = _mm_andnot_ps(xmm0, xmm4);
	y = _mm_and_ps(xmm0, _mm_set1_ps(M_PI / 2));
	y = _mm_or_ps(y, _mm_and_ps(xmm1, _mm_set1_ps(M_PI / 4)));
	xmm2 = _mm_add_ps(x, _mm_and_ps(xmm1, _mm_set1_ps(1)));
	xmm3 = _mm_add_ps(_mm_set1_ps(-1), _mm_and_ps(xmm1, x));
	xmm5 = div_ps(xmm3, xmm2);
	x = _mm_or_ps(_mm_andnot_ps(xmm4, x), _mm_and_ps(xmm4, xmm5));
	z = _mm_mul_ps(x, x);

	xmm0 = _mm_mul_ps(_mm_set1_ps(8.05374449538e-2), z);
	xmm1 = _mm_add_ps(_mm_set1_ps(-1.38776856032e-1), xmm0);
	xmm0 = _mm_mul_ps(xmm1, z);
	xmm1 = _mm_add_ps(_mm_set1_ps(1.99777106478e-1), xmm0);
	xmm0 = _mm_mul_ps(xmm1, z);
	xmm1 = _mm_add_ps(_mm_set1_ps(-3.33329491539e-1), xmm0);
	xmm0 = _mm_mul_ps(xmm1, z);
	xmm0 = _mm_mul_ps(xmm0, x);
	xmm0 = _mm_add_ps(xmm0, x);
	y = _mm_add_ps(y, xmm0);

	return _mm_xor_ps(sign_bit, y);
}

inline __m128 atan2_ps(__m128 y, __m128 x)
{
	__m128 xmm0, xmm1, xmm2, xmm3;
	__m128 xmm4, xmm5, xmm6, xmm7;
	__m128 sign_bit;
	__m128 z, w;

	xmm0 = _mm_cmplt_ps(x, _mm_setzero_ps());
	xmm1 = _mm_cmplt_ps(y, _mm_setzero_ps());
	xmm2 = _mm_cmpeq_ps(x, _mm_setzero_ps());
	xmm3 = _mm_cmpeq_ps(y, _mm_setzero_ps());

	sign_bit = _mm_and_ps(xmm1, SSE_SIGNMASK(1, 1, 1, 1));
	xmm5 = _mm_and_ps(xmm0, _mm_set1_ps(M_PI));
	w = _mm_or_ps(xmm5, sign_bit);
	z = atan_ps(div_ps(y, x));
	xmm4 = _mm_add_ps(w, z);

	// x == 0
	xmm6 = _mm_andnot_ps(xmm2, xmm4);
	xmm7 = _mm_andnot_ps(xmm1, _mm_set1_ps(M_PI / 2));
	xmm7 = _mm_or_ps(xmm7, sign_bit);
	xmm7 = _mm_andnot_ps(xmm3, xmm7);
	xmm7 = _mm_and_ps(xmm2, xmm7);
	xmm0 = _mm_or_ps(xmm6, xmm7);

	return xmm0;
}

inline __m128 asin_ps(__m128 x)
{
	__m128 xmm0, xmm1, xmm2;
	__m128 flag;
	__m128 z, z0;
	__m128 sign_bit;

	sign_bit = x;

	// Take the absolute value
	x = _mm_and_ps(x, SSE_NSIGNMASK(1, 1, 1, 1));

	// Extract the sign bit (upper one)
	sign_bit = _mm_and_ps(sign_bit, SSE_SIGNMASK(1, 1, 1, 1));

	flag = _mm_cmpgt_ps(x, _mm_set1_ps(0.5));
	xmm0 = _mm_mul_ps(_mm_set1_ps(0.5), _mm_sub_ps(_mm_set1_ps(1), x));
	xmm2 = sqrt_ps(xmm0);
	x = _mm_or_ps(_mm_and_ps(flag, xmm2), _mm_andnot_ps(flag, x));
	z0 = _mm_or_ps(_mm_and_ps(flag, xmm0), _mm_andnot_ps(flag, _mm_mul_ps(x, x)));

	z = _mm_mul_ps(z0, _mm_set1_ps(4.2163199048e-2));
	z = _mm_add_ps(z, _mm_set1_ps(2.4181311049e-2));
	z = _mm_mul_ps(z, z0);
	z = _mm_add_ps(z, _mm_set1_ps(4.5470025998e-2));
	z = _mm_mul_ps(z, z0);
	z = _mm_add_ps(z, _mm_set1_ps(7.4953002686e-2));
	z = _mm_mul_ps(z, z0);
	z = _mm_add_ps(z, _mm_set1_ps(1.6666752422e-1));
	z = _mm_mul_ps(z, z0);
	z = _mm_mul_ps(z, x);
	z = _mm_add_ps(z, x);

	xmm1 = _mm_sub_ps(_mm_set1_ps(M_PI / 2), _mm_add_ps(z, z));
	z = _mm_or_ps(_mm_and_ps(flag, xmm1), _mm_andnot_ps(flag, z));

	return _mm_or_ps(sign_bit, z);
}

inline __m128 acos_ps(__m128 x)
{
	__m128 xmm0, xmm1;
	__m128 flag;
	__m128 sign, sign_bit;

	// Extract the sign
	sign_bit = _mm_and_ps(x, SSE_SIGNMASK(1, 1, 1, 1));
	sign = _mm_cmplt_ps(x, _mm_setzero_ps());

	// Take the absolute value
	x = _mm_and_ps(x, SSE_NSIGNMASK(1, 1, 1, 1));

	flag = _mm_cmpgt_ps(x, _mm_set1_ps(0.5));
	xmm0 = _mm_mul_ps(_mm_set1_ps(0.5), _mm_sub_ps(_mm_set1_ps(1), x));
	xmm1 = sqrt_ps(xmm0);
	x = _mm_or_ps(_mm_and_ps(flag, xmm1), _mm_andnot_ps(flag, x));

	x = asin_ps(x);

	x = _mm_add_ps(x, _mm_and_ps(flag, x));
	xmm0 = _mm_and_ps(_mm_and_ps(flag, sign), _mm_set1_ps(M_PI / 2));
	xmm0 = _mm_add_ps(_mm_set1_ps(M_PI / 2), xmm0);

	sign_bit = _mm_andnot_ps(flag, sign_bit);
	xmm0 = _mm_sub_ps(xmm0, _mm_or_ps(x, sign_bit));
	xmm1 = _mm_andnot_ps(sign, flag);
	x = _mm_or_ps(_mm_andnot_ps(xmm1, xmm0), _mm_and_ps(xmm1, x));

	return x;
}
