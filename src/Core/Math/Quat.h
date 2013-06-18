//@@COPYRIGHT@@

// Quaternion

class Quat: public Vector4 {
public:
	// Constructors
	Quat() = default;
	template<typename... T> Quat(T... x): Vector4(x...) {}

	// Quaternion cross product
	const Quat operator*(Quat other) const
	{
		Vector4 yz_zx_xy_nzz = yzxz() * other.zxyz();
		yz_zx_xy_nzz.FlipSigns(0, 0, 0, 1);

		Vector4 wx_wy_wz_nyy = wwwy() * other.zxyz();
		wx_wy_wz_nyy.FlipSigns(0, 0, 0, 1);

		Vector4 xw_yw_zw_ww = xyzw() * other.wwww();

		Vector4 zy_xz_yz_xx = zxyx() * other.yzxx();

		return xw_yw_zw_ww - zy_xz_yz_xx + yz_zx_xy_nzz + wx_wy_wz_nyy;
	}
	Quat& operator*=(Quat other)
	{
		*this = *this * other;
		return *this;
	}

	// Derive the W element from the other 3
	void CalcW()
	{
		float w = sqrt(fabs(1 - DotProduct(xyz(), xyz())));
		*this = Vector4(xyz(), w);
	}

	// Convert to a matrix
	// http://cache-www.intel.com/cd/00/00/29/37/293748_293748.pdf
	const Matrix ToMatrix() const
	{
		Matrix out;
#ifdef __SSE__
		Vector4 x2y2z2w2 = *this + *this;

		Vector4 y2y2z2z2 = x2y2z2w2.yyzz();
		Vector4 yy2xy2xz2yz2 = yxxy() * y2y2z2z2;

		Vector4 z2z2y2x2 = x2y2z2w2.zzyx();
		Vector4 zz2wz2wy2wx2 = zwww() * z2z2y2x2;

		Vector4 xx2 = _mm_mul_ss(data, x2y2z2w2);
		Vector4 _xx2_1 = _mm_sub_ss(_mm_set_ss(1), xx2);
		Vector4 _xx2_yy2_1 = _mm_sub_ss(_xx2_1, yy2xy2xz2yz2);

		// First row
		Vector4 tmp0 = _mm_xor_ps(SSE_SIGNMASK(0, 1, 1, 1), yy2xy2xz2yz2);
		Vector4 tmp1 = _mm_xor_ps(SSE_SIGNMASK(1, 0, 1, 1), zz2wz2wy2wx2);
		tmp1 = _mm_add_ss(tmp1, _mm_set_ss(1));
		Vector4 tmp2 = tmp1 - tmp0;
		out[0] = _mm_and_ps(tmp2, SSE_MASK(1, 1, 1, 0));

		// Second row
		tmp0 = _mm_move_ss(tmp0, xx2);
		tmp1 = _mm_xor_ps(SSE_SIGNMASK(0, 1, 1, 1), tmp1);
		tmp1 = _mm_sub_ps(tmp1, tmp0);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1));
		out[1] = _mm_and_ps(tmp1, SSE_MASK(1, 1, 1, 0));

		// Third row
		tmp2 = _mm_movehl_ps(tmp2, tmp1);
		out[2] = _mm_shuffle_ps(tmp2, _xx2_yy2_1, _MM_SHUFFLE(2, 0, 3, 1));

		// Fourth row
		out[3] = Vector4(0, 0, 0, 1);
#else
		float x2 = data[0] + data[0];
		float y2 = data[1] + data[1];
		float z2 = data[2] + data[2];

		float xx2 = data[0] * x2;
		float yy2 = data[1] * y2;
		float zz2 = data[2] * z2;
		float yz2 = data[1] * z2;
		float wx2 = data[3] * x2;
		float xy2 = data[0] * y2;
		float wz2 = data[3] * z2;
		float xz2 = data[0] * z2;
		float wy2 = data[3] * y2;

		out[0] = Vector4(1 - yy2 - zz2, xy2 + wz2, xz2 - wy2, 0);
		out[1] = Vector4(xy2 - wz2, 1 - xx2 - zz2, yz2 + wx2, 0);
		out[2] = Vector4(xz2 + wy2, yz2 - wx2, 1 - xx2 - yy2, 0);
		out[3] = Vector4(0, 0, 0, 1);
#endif
		return out;
	}
};

// Normalized linear interpolation. This is a cheap lerp, but it doesn't
// keep the angular velocity constant.
inline const Quat QuatLerp(Quat from, Quat to, float t)
{
	Quat out = VectorLerp(from, to, t);
	out.Normalize();
	return out;
}

// Spherical linear interpolation. More expensive but smoother lerp.
// http://www.intel.com/cd/ids/developer/asmo-na/eng/293747.htm
inline const Quat QuatSlerp(Quat from, Quat to, float t)
{
	float cosom = DotProduct(from, to);
	float absCosom = fabs(cosom);
	float scale0 = 1 - t;
	float scale1 = t;

	if (1 - absCosom > 1e-6) {
		float sinSqr = 1 - absCosom * absCosom;
		float sinom = rsqrt(sinSqr);

		// Fast atan2 implementation, which only works in a limited range.
		// t = atan2(sinSqr * sinom, absCosom);
		float y = sinSqr * sinom;
		float x = absCosom;
		float a, d, s, t;

		if (y > x) {
			a = -x / y;
			d = M_PI / 2;
		} else {
			a = y / x;
			d = 0;
		}

		s = a * a;
		t = 0.0028662257f;
		t *= s;
		t += -0.0161657367f;
		t *= s;
		t += 0.0429096138f;
		t *= s;
		t += -0.0752896400f;
		t *= s;
		t += 0.1065626393f;
		t *= s;
		t += -0.1420889944f;
		t *= s;
		t += 0.1999355085f;
		t *= s;
		t += -0.3333314528f;
		t *= s;
		t *= a;
		t += a;
		t += d;

		scale0 = fastsin(scale0 * t) * sinom;
		scale1 = fastsin(scale1 * t) * sinom;
	}

	scale1 = copysign(scale1, cosom);

	return scale0 * from + scale1 * to;
}

// Make a quat from Euler angles
inline const Quat QuatFromAngles(float yaw, float pitch, float roll)
{
	Vector4 srpyr, crpyr;
	roll /= 2;
	pitch /= 2;
	yaw /= 2;
#ifdef __SSE__
	std::tie(srpyr, crpyr) = sincos_ps(Vector4(roll, pitch, yaw, roll));
#else
	srpyr = Vector4(sin(roll), sin(pitch), sin(yaw), sin(roll));
	crpyr = Vector4(cos(roll), cos(pitch), cos(yaw), cos(roll));
#endif

	Vector4 spyrp = srpyr.yzxy();
	Vector4 cpyrp = crpyr.yzxy();
	Vector4 syrpy = srpyr.zxyz();
	Vector4 cyrpy = crpyr.zxyz();

	// Exchange low floats of yrpy
#ifdef __SSE__
	Vector4 cysrpy = _mm_move_ss(syrpy, cyrpy);
	Vector4 sycrpy = _mm_move_ss(cyrpy, syrpy);
#else
	Vector4 cysrpy = Vector4(cyrpy.x(), syrpy.yzw());
	Vector4 sycrpy = Vector4(syrpy.x(), cyrpy.yzw());
#endif

	// Multiply
	Vector4 half0 = srpyr * spyrp * sycrpy;
	Vector4 half1 = crpyr * cpyrp * cysrpy;

	// Add
#ifdef __SSE3__
	Vector4 result = _mm_addsub_ps(half1, half0);
#else
	half0.FlipSigns(0, 1, 0, 1);
	Vector4 result = half1 + half0;
#endif

	return result.yzwx();
}

// Identity quat
inline const Quat QuatIdentity()
{
	return Quat(0, 0, 0, 1);
}
