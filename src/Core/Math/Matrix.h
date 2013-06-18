//@@COPYRIGHT@@

// 4x4 Matrix, in column major order.

class Plane;
class Matrix {
private:
	// 4x4 element vectors
	Vector4 data[4];

public:
	// Default constructor
	Matrix() = default;

	// Construct from elements
	Matrix(Vector4 col0, Vector4 col1, Vector4 col2, Vector4 col3)
	{
		data[0] = col0;
		data[1] = col1;
		data[2] = col2;
		data[3] = col3;
	}

	// Matrix multiplication
	const Matrix operator*(Matrix other) const
	{
		Matrix out;
		for (int i = 0; i < 4; i++) {
			Vector4 x = other[i].xxxx() * data[0];
			Vector4 y = other[i].yyyy() * data[1];
			Vector4 z = other[i].zzzz() * data[2];
			Vector4 w = other[i].wwww() * data[3];
			out[i] = x + y + z + w;
		}
		return out;
	}
	Matrix& operator*=(Matrix other)
	{
		*this = *this * other;
		return *this;
	}

	// Comparaison operators
	bool operator==(Matrix other) const
	{
		for (int i = 0; i < 4; i++) {
			if (data[i] != other[i])
				return false;
		}
		return true;
	}
	bool operator!=(Matrix other) const
	{
		return !operator==(other);
	}

	// Subscript operator
	Vector4& operator[](int index)
	{
		return data[index];
	}
	const Vector4 operator[](int index) const
	{
		return data[index];
	}

	// Matrix transpose. Use this instead of Inverse() if the matrix only
	// consists of a rotation or reflection.
	const Matrix Transpose() const
	{
		Matrix out;
#ifdef __SSE__
		__m128 tmp0 = _mm_unpacklo_ps(data[0], data[1]);
		__m128 tmp1 = _mm_unpacklo_ps(data[2], data[3]);
		__m128 tmp2 = _mm_unpackhi_ps(data[0], data[1]);
		__m128 tmp3 = _mm_unpackhi_ps(data[2], data[3]);
		out[0] = _mm_movelh_ps(tmp0, tmp1);
		out[1] = _mm_movehl_ps(tmp1, tmp0);
		out[2] = _mm_movelh_ps(tmp2, tmp3);
		out[3] = _mm_movehl_ps(tmp3, tmp2);
#else
		out[0] = Vector4(data[0].x(), data[1].x(), data[2].x(), data[3].x());
		out[1] = Vector4(data[0].y(), data[1].y(), data[2].y(), data[3].y());
		out[2] = Vector4(data[0].z(), data[1].z(), data[2].z(), data[3].z());
		out[3] = Vector4(data[0].w(), data[1].w(), data[2].w(), data[3].w());
#endif
		return out;
	}

	// Matrix inversion code by Intel
	// http://software.intel.com/en-us/articles/optimized-matrix-library-for-use-with-the-intel-pentiumr-4-processors-sse2-instructions/
	//
	//   Copyright (c) 2001 Intel Corporation.
	//
	// Permition is granted to use, copy, distribute and prepare derivative works
	// of this library for any purpose and without fee, provided, that the above
	// copyright notice and this statement appear in all copies.
	// Intel makes no representations about the suitability of this software for
	// any purpose, and specifically disclaims all warranties.
	// See LEGAL.TXT for all the legal information.
	const Matrix Inverse() const
	{
		Matrix out;
		Vector4 tmp;
		Vector4 A, B, C, D;
		Vector4 iA, iB, iC, iD, DC, AB;
		float dA, dB, dC, dD;
		float det, d, d1, d2;
		Vector4 rd;

#ifdef __SSE__
		A = _mm_movelh_ps(data[0], data[1]);
		B = _mm_movehl_ps(data[1], data[0]);
		C = _mm_movelh_ps(data[2], data[3]);
		D = _mm_movehl_ps(data[3], data[2]);
#else
		A = Vector4(data[0].xy(), data[1].xy());
		B = Vector4(data[0].zw(), data[1].zw());
		C = Vector4(data[2].xy(), data[3].xy());
		D = Vector4(data[2].zw(), data[3].zw());
#endif

		AB = A.wwxx() * B;
		AB -= A.yyzz() * B.zwxy();
		DC = D.wwxx() * C;
		DC -= D.yyzz() * C.zwxy();

		tmp = A.wwyy() * A;
		dA = tmp.x() - tmp.z();
		tmp = B.wwyy() * B;
		dB = tmp.x() - tmp.z();
		tmp = C.wwyy() * C;
		dC = tmp.x() - tmp.z();
		tmp = D.wwyy() * D;
		dD = tmp.x() - tmp.z();

		tmp = AB * DC.xzyw();

		iD = C.xxzz() * AB.xyxy();
		iD += C.yyww() * AB.zwzw();
		iA = B.xxzz() * DC.xyxy();
		iA += B.yyww() * DC.zwzw();

		Vector2 tmp2 = tmp.xy() + tmp.zw();
		d = tmp2.x() + tmp2.y();
		d1 = dA * dD;
		d2 = dB * dC;

		iD = D * dA - iD;
		iA = A * dD - iA;

		det = 1 / (d1 + d2 - d);

		iB = D * AB.wxwx();
		iB -= D.yxwz() * AB.zyzy();
		iC = A * DC.wxwx();
		iC -= A.yxwz() * DC.zyzy();

		rd = Vector4(det);
		rd.FlipSigns(0, 1, 1, 0);

		iB = C * dB - iB;
		iC = B * dC - iC;

		iA *= rd;
		iB *= rd;
		iC *= rd;
		iD *= rd;

#ifdef __SSE__
		out[0] = _mm_shuffle_ps(iA, iB, 0x77);
		out[1] = _mm_shuffle_ps(iA, iB, 0x22);
		out[2] = _mm_shuffle_ps(iC, iD, 0x77);
		out[3] = _mm_shuffle_ps(iC, iD, 0x22);
#else
		out[0] = Vector4(iA.wy(), iB.wy());
		out[1] = Vector4(iA.zx(), iB.zx());
		out[2] = Vector4(iC.wy(), iD.wy());
		out[3] = Vector4(iC.zx(), iD.zx());
#endif

		return out;
	}

	// Cheaper inverse if the matrix is an affine transformation
	// (Rotation/reflection + translation)
	const Matrix AffineInverse() const
	{
		Matrix out;

		// Transpose the 3x3 inner matrix
#ifdef __SSE__
		__m128 tmp0 = _mm_unpacklo_ps(data[0], data[1]);
		__m128 tmp1 = _mm_unpacklo_ps(data[2], _mm_setzero_ps());
		__m128 tmp2 = _mm_unpackhi_ps(data[0], data[1]);
		__m128 tmp3 = _mm_unpackhi_ps(data[2], _mm_setzero_ps());
		out[0] = _mm_movelh_ps(tmp0, tmp1);
		out[1] = _mm_movehl_ps(tmp1, tmp0);
		out[2] = _mm_movelh_ps(tmp2, tmp3);
#else
		out[0] = Vector4(data[0].x(), data[1].x(), data[2].x(), 0);
		out[1] = Vector4(data[0].y(), data[1].y(), data[2].y(), 0);
		out[2] = Vector4(data[0].z(), data[1].z(), data[2].z(), 0);
#endif

		// Transform the translation vector with the transposed matrix
		Vector3 x = data[3].xxx() * out[0].xyz();
		Vector3 y = data[3].yyy() * out[1].xyz();
		Vector3 z = data[3].zzz() * out[2].xyz();
		out[3] = Vector4(-(x + y + z), 1);

		return out;
	}

	// Transform a 4 element vector with the matrix
	const Vector4 Transform4(Vector4 vec) const
	{
		Vector4 x = vec.xxxx() * data[0];
		Vector4 y = vec.yyyy() * data[1];
		Vector4 z = vec.zzzz() * data[2];
		Vector4 w = vec.wwww() * data[3];
		return x + y + z + w;
	}

	// Transform a normal with the matrix (ignores the translation part)
	const Vector3 TransformNormal(Vector3 vec) const
	{
		Vector3 x = vec.xxx() * data[0].xyz();
		Vector3 y = vec.yyy() * data[1].xyz();
		Vector3 z = vec.zzz() * data[2].xyz();
		return x + y + z;
	}

	// Transform a point with the matrix
	const Vector3 TransformPoint(Vector3 vec) const
	{
		Vector3 x = vec.xxx() * data[0].xyz();
		Vector3 y = vec.yyy() * data[1].xyz();
		Vector3 z = vec.zzz() * data[2].xyz();
		return x + y + z + data[3].xyz();
	}

	// Transform a plane with the matrix
	const Plane TransformPlane(Plane plane) const;
};

// Identity matrix
inline const Matrix MatrixIdentity()
{
	Matrix out;
	out[0] = Vector4(1, 0, 0, 0);
	out[1] = Vector4(0, 1, 0, 0);
	out[2] = Vector4(0, 0, 1, 0);
	out[3] = Vector4(0, 0, 0, 1);
	return out;
}

// Scale matrix
inline const Matrix MatrixScale(float x, float y, float z)
{
	Matrix out;
	out[0] = Vector4(x, 0, 0, 0);
	out[1] = Vector4(0, y, 0, 0);
	out[2] = Vector4(0, 0, z, 0);
	out[3] = Vector4(0, 0, 0, 1);
	return out;
}
inline const Matrix MatrixScale(Vector3 vec)
{
	Matrix out;
	Vector4 vec4 = Vector4(vec, 0);
	out[0] = vec4.xwww();
	out[1] = vec4.wyww();
	out[2] = vec4.wwzw();
	out[3] = Vector4(0, 0, 0, 1);
	return out;
}

// Build a matrix from a set of axis
inline const Matrix MatrixFromAxis(Vector3 x, Vector3 y, Vector3 z)
{
	Matrix out;
	out[0] = Vector4(x, 0);
	out[1] = Vector4(y, 0);
	out[2] = Vector4(z, 0);
	out[3] = Vector4(0, 0, 0, 1);
	return out;
}

// Combine a rotation matrix and a translation
inline const Matrix MatrixMakeTransform(Matrix rot, Vector3 trans)
{
	Matrix out;
	out[0] = rot[0];
	out[1] = rot[1];
	out[2] = rot[2];
	out[3] = Vector4(trans, 1);
	return out;
}

// Make a matrix from Euler angles
inline const Matrix MatrixFromAngles(float yaw, float pitch, float roll)
{
	Matrix out;
#ifdef __SSE__
	Vector4 s, c;
	std::tie(s, c) = sincos_ps(Vector3(roll, pitch, yaw));
	Vector4 Z0 = _mm_unpackhi_ps(c, s);
	Vector4 Z1 = _mm_unpackhi_ps(-s, c);
	Z1 = _mm_and_ps(Z1, SSE_MASK(1, 1, 1, 0));
	Vector4 X0 = s.xxxx();
	Vector4 X1 = c.xxxx();
	Vector4 Y1 = _mm_shuffle_ps(s, c, _MM_SHUFFLE(0, 1, 1, 1));
	Vector4 Y0 = _mm_shuffle_ps(c, -s, _MM_SHUFFLE(0, 1, 1, 1));
	Vector4 tmp = Z0 * Y1;

	out[0] = Z0 * Y0;
	out[1] = tmp * X0 + Z1 * X1;
	out[2] = tmp * X1 - Z1 * X0;
	out[3] = Vector4(0, 0, 0, 1);
#else
	float sr, sp, sy, cr, cp, cy;

	sy = sin(yaw);
	sp = sin(pitch);
	sr = sin(roll);
	cy = cos(yaw);
	cp = cos(pitch);
	cr = cos(roll);

	out[0] = Vector4(cp*cy, cp*sy, -sp, 0);
	out[1] = Vector4(sr*sp*cy - cr*sy, sr*sp*sy + cr*cy, sr*cp, 0);
	out[2] = Vector4(cr*sp*cy + sr*sy, cr*sp*sy - sr*cy, cr*cp, 0);
	out[3] = Vector4(0, 0, 0, 1);
#endif
	return out;
}
