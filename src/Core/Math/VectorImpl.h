//@@COPYRIGHT@@

// Implementation of vector classes

#ifdef __SSE__
template<int length> inline VectorBase<length>::operator __m128() const
{
	return data;
}
#endif

template<int length> template<int x> inline float VectorBase<length>::Swizzle() const
{
	static_assert(x >= 0 && x < length, "Invalid swizzle index");
#ifdef __SSE__
	if (x == 0)
		return _mm_cvtss_f32(data);
	else if (x == 1)
		return _mm_cvtss_f32(shuffle_ps(data, _MM_SHUFFLE(1, 1, 1, 1)));
	else if (x == 2)
#ifdef __SSE2__
		return _mm_cvtss_f32(shuffle_ps(data, _MM_SHUFFLE(2, 2, 2, 2)));
#else
		return _mm_cvtss_f32(_mm_movehl_ps(data, data));
#endif
	else
		return _mm_cvtss_f32(shuffle_ps(data, _MM_SHUFFLE(3, 3, 3, 3)));
#else
	return data[x];
#endif
}
template<int length> template<int x, int y> inline const Vector2 VectorBase<length>::Swizzle() const
{
	static_assert(x >= 0 && x < length, "Invalid swizzle index");
	static_assert(y >= 0 && y < length, "Invalid swizzle index");
#ifdef __SSE__
	if (x == 0 && y == 1)
		return data;
	else
		return shuffle_ps(data, _MM_SHUFFLE(3, 2, y, x));
#else
	return Vector2(data[x], data[y]);
#endif
}
template<int length> template<int x, int y, int z> inline const Vector3 VectorBase<length>::Swizzle() const
{
	static_assert(x >= 0 && x < length, "Invalid swizzle index");
	static_assert(y >= 0 && y < length, "Invalid swizzle index");
	static_assert(z >= 0 && z < length, "Invalid swizzle index");
#ifdef __SSE__
	if (x == 0 && y == 1 && z == 2)
		return data;
	else
		return shuffle_ps(data, _MM_SHUFFLE(3, z, y, x));
#else
	return Vector3(data[x], data[y], data[z]);
#endif
}
template<int length> template<int x, int y, int z, int w> inline const Vector4 VectorBase<length>::Swizzle() const
{
	static_assert(x >= 0 && x < length, "Invalid swizzle index");
	static_assert(y >= 0 && y < length, "Invalid swizzle index");
	static_assert(z >= 0 && z < length, "Invalid swizzle index");
	static_assert(w >= 0 && w < length, "Invalid swizzle index");
#ifdef __SSE__
	if (x == 0 && y == 1 && z == 2 && w == 3)
		return data;
	else
		return shuffle_ps(data, _MM_SHUFFLE(w, z, y, x));
#else
	return Vector4(data[x], data[y], data[z], data[w]);
#endif
}

template<int length> inline float VectorBase<length>::operator[](int index) const
{
	__assume(index >= 0 && index <= length);
	if (index == 0)
		return Swizzle<0>();
	else if (index == 1)
		return Swizzle<1>();
	// Need to check length because Swizzle is not defined for x > length
	else if (index == 2)
		return Swizzle<length >= 3 ? 2 : 0>();
	else
		return Swizzle<length >= 4 ? 3 : 0>();
}

template<int length> inline void VectorBase<length>::Set(int index, float x)
{
	__assume(index >= 0 && index <= length);
#ifdef __SSE__
	if (index == 0)
		data = _mm_move_ss(data, _mm_set_ss(x));
	else if (index == 1)
		data = BVector4(1, 0, 1, 1).Mix(data, _mm_set1_ps(x));
	else if (index == 2)
		data = BVector4(1, 1, 0, 1).Mix(data, _mm_set1_ps(x));
	else
		data = BVector4(1, 1, 1, 0).Mix(data, _mm_set1_ps(x));
#else
	data[index] = x;
#endif
}

template<int length> inline const BVectorEq<length> VectorBase<length>::operator==(Vector<length> other) const
{
#ifdef __SSE__
	return BVector<length>(_mm_cmpeq_ps(data, other));
#else
	BVector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, data[i] == other[i]);
	return out;
#endif
}
template<int length> inline const BVectorNe<length> VectorBase<length>::operator!=(Vector<length> other) const
{
#ifdef __SSE__
	return BVector<length>(_mm_cmpneq_ps(data, other));
#else
	BVector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, data[i] != other[i]);
	return out;
#endif
}
template<int length> inline const BVector<length> VectorBase<length>::operator>=(Vector<length> other) const
{
#ifdef __SSE__
	return _mm_cmpge_ps(data, other);
#else
	BVector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, data[i] >= other[i]);
	return out;
#endif
}
template<int length> inline const BVector<length> VectorBase<length>::operator<=(Vector<length> other) const
{
#ifdef __SSE__
	return _mm_cmple_ps(data, other);
#else
	BVector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, data[i] <= other[i]);
	return out;
#endif
}
template<int length> inline const BVector<length> VectorBase<length>::operator>(Vector<length> other) const
{
#ifdef __SSE__
	return _mm_cmpgt_ps(data, other);
#else
	BVector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, data[i] > other[i]);
	return out;
#endif
}
template<int length> inline const BVector<length> VectorBase<length>::operator<(Vector<length> other) const
{
#ifdef __SSE__
	return _mm_cmplt_ps(data, other);
#else
	BVector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, data[i] < other[i]);
	return out;
#endif
}

template<int length> inline const Vector<length> VectorBase<length>::operator-() const
{
#ifdef __SSE__
	return _mm_sub_ps(_mm_setzero_ps(), data);
#else
	Vector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, -data[i]);
	return out;
#endif
}
template<int length> inline const Vector<length> VectorBase<length>::operator+(Vector<length> other) const
{
#ifdef __SSE__
	return _mm_add_ps(data, other);
#else
	Vector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, data[i] + other[i]);
	return out;
#endif
}
template<int length> inline const Vector<length> VectorBase<length>::operator-(Vector<length> other) const
{
#ifdef __SSE__
	return _mm_sub_ps(data, other);
#else
	Vector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, data[i] - other[i]);
	return out;
#endif
}
template<int length> inline const Vector<length> VectorBase<length>::operator*(float x) const
{
	return *this * Vector<length>(x);
}
template<int length> inline const Vector<length> operator*(float x, Vector<length> other)
{
	return Vector<length>(x) * other;
}
template<int length> inline const Vector<length> VectorBase<length>::operator*(Vector<length> other) const
{
#ifdef __SSE__
	return _mm_mul_ps(data, other);
#else
	Vector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, data[i] * other[i]);
	return out;
#endif
}
template<int length> inline const Vector<length> VectorBase<length>::operator/(float x) const
{
	return *this / Vector<length>(x);
}
template<int length> inline const Vector<length> operator/(float x, Vector<length> other)
{
	return Vector<length>(x) / other;
}
template<int length> inline const Vector<length> VectorBase<length>::operator/(Vector<length> other) const
{
#ifdef __SSE__
	return div_ps(data, other);
#else
	Vector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, data[i] / other[i]);
	return out;
#endif
}

template<int length> inline Vector<length>& VectorBase<length>::operator+=(Vector<length> other)
{
	*this = *this + other;
	return static_cast<Vector<length>&>(*this);
}
template<int length> inline Vector<length>& VectorBase<length>::operator-=(Vector<length> other)
{
	*this = *this - other;
	return static_cast<Vector<length>&>(*this);
}
template<int length> inline Vector<length>& VectorBase<length>::operator*=(float x)
{
	*this = *this * x;
	return static_cast<Vector<length>&>(*this);
}
template<int length> inline Vector<length>& VectorBase<length>::operator*=(Vector<length> other)
{
	*this = *this * other;
	return static_cast<Vector<length>&>(*this);
}
template<int length> inline Vector<length>& VectorBase<length>::operator/=(float x)
{
	*this = *this / x;
	return static_cast<Vector<length>&>(*this);
}
template<int length> inline Vector<length>& VectorBase<length>::operator/=(Vector<length> other)
{
	*this = *this / other;
	return static_cast<Vector<length>&>(*this);
}

template<int length> inline float VectorBase<length>::LengthSq() const
{
	return DotProduct(static_cast<Vector<length>>(*this), static_cast<Vector<length>>(*this));
}
template<int length> inline float VectorBase<length>::Length() const
{
	return sqrt(LengthSq());
}

template<int length> inline const Vector<length> VectorBase<length>::Normalized() const
{
	return *this * rsqrt(LengthSq());
}
template<int length> inline void VectorBase<length>::Normalize()
{
	*this = Normalized();
}

template<int length> inline void VectorBase<length>::FlipSigns(BVector<length> signs)
{
#ifdef __SSE__
	data = _mm_xor_ps(data, _mm_and_ps(signs, SSE_SIGNMASK(1, 1, 1, 1)));
#else
	for (int i = 0; i < length; i++)
		data[i] = signs[i] ? -data[i] : data[i];
#endif
}
template<int length> template<typename... T>
inline void VectorBase<length>::FlipSigns(T&&... signs)
{
	FlipSigns(BVector<length>(signs...));
}

#ifdef __SSE__
template<int length> inline BVectorBase<length>::operator __m128() const
{
	return data;
}
#endif

template<int length> template<int x> inline bool BVectorBase<length>::Swizzle() const
{
	static_assert(x >= 0 && x < length, "Invalid swizzle index");
#ifdef __SSE__
	return _mm_movemask_ps(data) & (1 << x);
#else
	return data[x];
#endif
}
template<int length> template<int x, int y> inline const BVector2 BVectorBase<length>::Swizzle() const
{
	static_assert(x >= 0 && x < length, "Invalid swizzle index");
	static_assert(y >= 0 && y < length, "Invalid swizzle index");
#ifdef __SSE__
	if (x == 0 && y == 1)
		return data;
	else
		return shuffle_ps(data, _MM_SHUFFLE(3, 2, y, x));
#else
	return BVector2(data[x], data[y]);
#endif
}
template<int length> template<int x, int y, int z> inline const BVector3 BVectorBase<length>::Swizzle() const
{
	static_assert(x >= 0 && x < length, "Invalid swizzle index");
	static_assert(y >= 0 && y < length, "Invalid swizzle index");
	static_assert(z >= 0 && z < length, "Invalid swizzle index");
#ifdef __SSE__
	if (x == 0 && y == 1 && z == 2)
		return data;
	else
		return shuffle_ps(data, _MM_SHUFFLE(3, z, y, x));
#else
	return BVector3(data[x], data[y], data[z]);
#endif
}
template<int length> template<int x, int y, int z, int w> inline const BVector4 BVectorBase<length>::Swizzle() const
{
	static_assert(x >= 0 && x < length, "Invalid swizzle index");
	static_assert(y >= 0 && y < length, "Invalid swizzle index");
	static_assert(z >= 0 && z < length, "Invalid swizzle index");
	static_assert(w >= 0 && w < length, "Invalid swizzle index");
#ifdef __SSE__
	if (x == 0 && y == 1 && z == 2 && w == 3)
		return data;
	else
		return shuffle_ps(data, _MM_SHUFFLE(w, z, y, x));
#else
	return BVector4(data[x], data[y], data[z], data[w]);
#endif
}

template<int length> inline bool BVectorBase<length>::operator[](int index) const
{
	__assume(index >= 0 && index <= length);
	if (index == 0)
		return Swizzle<0>();
	else if (index == 1)
		return Swizzle<1>();
	// Need to check length because Swizzle is not defined for x > length
	else if (index == 2)
		return Swizzle<length >= 3 ? 2 : 0>();
	else
		return Swizzle<length >= 4 ? 3 : 0>();
}

template<int length> inline void BVectorBase<length>::Set(int index, bool x)
{
	__assume(index >= 0 && index <= length);
#ifdef __SSE__
	if (index == 0) {
		if (x)
			data = _mm_move_ss(data, SSE_MASK(1, 1, 1, 1));
		else
			data = _mm_move_ss(data, _mm_setzero_ps());
	} else if (index == 1)
		data = _mm_or_ps(_mm_andnot_ps(data, SSE_MASK(0, 1, 0, 0)), SSE_MASK(0, x, 0, 0));
	else if (index == 2)
		data = _mm_or_ps(_mm_andnot_ps(data, SSE_MASK(0, 0, 1, 0)), SSE_MASK(0, 0, x, 0));
	else
		data = _mm_or_ps(_mm_andnot_ps(data, SSE_MASK(0, 0, 0, 1)), SSE_MASK(0, 0, 0, x));
#else
	data[index] = x;
#endif
}

template<int length> inline const BVectorEq<length> BVectorBase<length>::operator==(BVector<length> other) const
{
	return ~(*this ^ other);
}
template<int length> inline const BVectorNe<length> BVectorBase<length>::operator!=(BVector<length> other) const
{
	return *this ^ other;
}

template<int length> inline const BVector<length> BVectorBase<length>::operator~() const
{
#ifdef __SSE__
	return _mm_xor_ps(data, SSE_MASK(1, 1, 1, 1));
#else
	Vector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, !data[i]);
	return out;
#endif
}
template<int length> inline const BVector<length> BVectorBase<length>::operator|(BVector<length> other) const
{
#ifdef __SSE__
	return _mm_or_ps(data, other);
#else
	Vector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, data[i] || other[i]);
	return out;
#endif
}
template<int length> inline const BVector<length> BVectorBase<length>::operator&(BVector<length> other) const
{
#ifdef __SSE__
	return _mm_and_ps(data, other);
#else
	Vector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, data[i] && other[i]);
	return out;
#endif
}
template<int length> inline const BVector<length> BVectorBase<length>::operator^(BVector<length> other) const
{
#ifdef __SSE__
	return _mm_xor_ps(data, other);
#else
	Vector<length> out;
	for (int i = 0; i < length; i++)
		out.Set(i, data[i] != other[i]);
	return out;
#endif
}

template<int length> inline BVector<length>& BVectorBase<length>::operator|=(BVector<length> other)
{
	*this = *this | other;
	return static_cast<Vector<length>&>(*this);
}
template<int length> inline BVector<length>& BVectorBase<length>::operator&=(BVector<length> other)
{
	*this = *this & other;
	return static_cast<Vector<length>&>(*this);
}
template<int length> inline BVector<length>& BVectorBase<length>::operator^=(BVector<length> other)
{
	*this = *this ^ other;
	return static_cast<Vector<length>&>(*this);
}

template<int length> inline bool BVectorBase<length>::Any() const
{
#ifdef __SSE__
	int mask = 0xf >> (4 - length);
	return (_mm_movemask_ps(data) & mask) != 0;
#else
	for (int i = 0; i < length; i++) {
		if (data[i])
			return true;
	}
	return false;
#endif
}
template<int length> inline bool BVectorBase<length>::All() const
{
#ifdef __SSE__
	int mask = 0xf >> (4 - length);
	return (_mm_movemask_ps(data) & mask) == mask;
#else
	for (int i = 0; i < length; i++) {
		if (!data[i])
			return false;
	}
	return true;
#endif
}
template<int length> inline bool BVectorBase<length>::None() const
{
	return !Any();
}
template<int length> inline bool BVectorBase<length>::NotAll() const
{
	return !All();
}

template<int length> inline const Vector<length> BVectorBase<length>::Mix(Vector<length> valueTrue, Vector<length> valueFalse) const
{
#ifdef __SSE__
	return _mm_or_ps(_mm_and_ps(data, valueTrue), _mm_andnot_ps(data, valueFalse));
#else
	Vector<length> out;
	for (int i = 0; i < length; i++)
		out.data[i] = data[i] ? valueTrue[i] : valueFalse[i];
	return out;
#endif
}

template<int length> inline const Vector<length> BVectorBase<length>::Mask(Vector<length> value) const
{
#ifdef __SSE__
	return _mm_and_ps(data, value);
#else
	Vector<length> out;
	for (int i = 0; i < length; i++)
		out.data[i] = data[i] ? value[i] : 0;
	return out;
#endif
}

template<int length> inline int BVectorBase<length>::Bits() const
{
#ifdef __SSE__
	return _mm_movemask_ps(data);
#else
	int out = 0;
	for (int i = 0; i < length; i++)
		out |= data[i] << i;
	return out;
#endif
}

inline Vector2::Vector(float x)
{
#ifdef __SSE__
	data = _mm_set1_ps(x);
#else
	data[0] = x;
	data[1] = x;
#endif
}
inline Vector2::Vector(float x, float y)
{
#ifdef __SSE__
	data = _mm_setr_ps(x, y, 0, 0);
#else
	data[0] = x;
	data[1] = y;
#endif
}

#ifdef __SSE__
inline Vector2::Vector(__m128 x)
{
	data = x;
}
inline Vector2& Vector2::operator=(__m128 x)
{
	data = x;
	return *this;
}
#endif

inline float Vector2::x() const {return Swizzle<0>();}
inline float Vector2::y() const {return Swizzle<1>();}
inline const Vector2 Vector2::xx() const {return Swizzle<0, 0>();}
inline const Vector2 Vector2::xy() const {return Swizzle<0, 1>();}
inline const Vector2 Vector2::yx() const {return Swizzle<1, 0>();}
inline const Vector2 Vector2::yy() const {return Swizzle<1, 1>();}
inline const Vector3 Vector2::xxx() const {return Swizzle<0, 0, 0>();}
inline const Vector3 Vector2::xxy() const {return Swizzle<0, 0, 1>();}
inline const Vector3 Vector2::xyx() const {return Swizzle<0, 1, 0>();}
inline const Vector3 Vector2::xyy() const {return Swizzle<0, 1, 1>();}
inline const Vector3 Vector2::yxx() const {return Swizzle<1, 0, 0>();}
inline const Vector3 Vector2::yxy() const {return Swizzle<1, 0, 1>();}
inline const Vector3 Vector2::yyx() const {return Swizzle<1, 1, 0>();}
inline const Vector3 Vector2::yyy() const {return Swizzle<1, 1, 1>();}
inline const Vector4 Vector2::xxxx() const {return Swizzle<0, 0, 0, 0>();}
inline const Vector4 Vector2::xxxy() const {return Swizzle<0, 0, 0, 1>();}
inline const Vector4 Vector2::xxyx() const {return Swizzle<0, 0, 1, 0>();}
inline const Vector4 Vector2::xxyy() const {return Swizzle<0, 0, 1, 1>();}
inline const Vector4 Vector2::xyxx() const {return Swizzle<0, 1, 0, 0>();}
inline const Vector4 Vector2::xyxy() const {return Swizzle<0, 1, 0, 1>();}
inline const Vector4 Vector2::xyyx() const {return Swizzle<0, 1, 1, 0>();}
inline const Vector4 Vector2::xyyy() const {return Swizzle<0, 1, 1, 1>();}
inline const Vector4 Vector2::yxxx() const {return Swizzle<1, 0, 0, 0>();}
inline const Vector4 Vector2::yxxy() const {return Swizzle<1, 0, 0, 1>();}
inline const Vector4 Vector2::yxyx() const {return Swizzle<1, 0, 1, 0>();}
inline const Vector4 Vector2::yxyy() const {return Swizzle<1, 0, 1, 1>();}
inline const Vector4 Vector2::yyxx() const {return Swizzle<1, 1, 0, 0>();}
inline const Vector4 Vector2::yyxy() const {return Swizzle<1, 1, 0, 1>();}
inline const Vector4 Vector2::yyyx() const {return Swizzle<1, 1, 1, 0>();}
inline const Vector4 Vector2::yyyy() const {return Swizzle<1, 1, 1, 1>();}

inline Vector3::Vector(float x)
{
#ifdef __SSE__
	data = _mm_set1_ps(x);
#else
	data[0] = x;
	data[1] = x;
	data[2] = x;
#endif
}
inline Vector3::Vector(float x, float y, float z)
{
#ifdef __SSE__
	data = _mm_setr_ps(x, y, z, 0);
#else
	data[0] = x;
	data[1] = y;
	data[2] = z;
#endif
}
inline Vector3::Vector(Vector2 vec, float z)
{
#ifdef __SSE__
	data = _mm_movelh_ps(vec, _mm_set_ss(z));
#else
	data[0] = vec[0];
	data[1] = vec[1];
	data[2] = z;
#endif
}
inline Vector3::Vector(float x, Vector2 vec)
{
#ifdef __SSE__
	data = _mm_move_ss(_mm_unpacklo_ps(vec, vec), _mm_set_ss(x));
#else
	data[0] = x;
	data[1] = vec[0];
	data[2] = vec[1];
#endif
}

#ifdef __SSE__
inline Vector3::Vector(__m128 x)
{
	data = x;
}
inline Vector3& Vector3::operator=(__m128 x)
{
	data = x;
	return *this;
}
#endif

inline float Vector3::x() const {return Swizzle<0>();}
inline float Vector3::y() const {return Swizzle<1>();}
inline float Vector3::z() const {return Swizzle<2>();}
inline const Vector2 Vector3::xx() const {return Swizzle<0, 0>();}
inline const Vector2 Vector3::xy() const {return Swizzle<0, 1>();}
inline const Vector2 Vector3::xz() const {return Swizzle<0, 2>();}
inline const Vector2 Vector3::yx() const {return Swizzle<1, 0>();}
inline const Vector2 Vector3::yy() const {return Swizzle<1, 1>();}
inline const Vector2 Vector3::yz() const {return Swizzle<1, 2>();}
inline const Vector2 Vector3::zx() const {return Swizzle<2, 0>();}
inline const Vector2 Vector3::zy() const {return Swizzle<2, 1>();}
inline const Vector2 Vector3::zz() const {return Swizzle<2, 2>();}
inline const Vector3 Vector3::xxx() const {return Swizzle<0, 0, 0>();}
inline const Vector3 Vector3::xxy() const {return Swizzle<0, 0, 1>();}
inline const Vector3 Vector3::xxz() const {return Swizzle<0, 0, 2>();}
inline const Vector3 Vector3::xyx() const {return Swizzle<0, 1, 0>();}
inline const Vector3 Vector3::xyy() const {return Swizzle<0, 1, 1>();}
inline const Vector3 Vector3::xyz() const {return Swizzle<0, 1, 2>();}
inline const Vector3 Vector3::xzx() const {return Swizzle<0, 2, 0>();}
inline const Vector3 Vector3::xzy() const {return Swizzle<0, 2, 1>();}
inline const Vector3 Vector3::xzz() const {return Swizzle<0, 2, 2>();}
inline const Vector3 Vector3::yxx() const {return Swizzle<1, 0, 0>();}
inline const Vector3 Vector3::yxy() const {return Swizzle<1, 0, 1>();}
inline const Vector3 Vector3::yxz() const {return Swizzle<1, 0, 2>();}
inline const Vector3 Vector3::yyx() const {return Swizzle<1, 1, 0>();}
inline const Vector3 Vector3::yyy() const {return Swizzle<1, 1, 1>();}
inline const Vector3 Vector3::yyz() const {return Swizzle<1, 1, 2>();}
inline const Vector3 Vector3::yzx() const {return Swizzle<1, 2, 0>();}
inline const Vector3 Vector3::yzy() const {return Swizzle<1, 2, 1>();}
inline const Vector3 Vector3::yzz() const {return Swizzle<1, 2, 2>();}
inline const Vector3 Vector3::zxx() const {return Swizzle<2, 0, 0>();}
inline const Vector3 Vector3::zxy() const {return Swizzle<2, 0, 1>();}
inline const Vector3 Vector3::zxz() const {return Swizzle<2, 0, 2>();}
inline const Vector3 Vector3::zyx() const {return Swizzle<2, 1, 0>();}
inline const Vector3 Vector3::zyy() const {return Swizzle<2, 1, 1>();}
inline const Vector3 Vector3::zyz() const {return Swizzle<2, 1, 2>();}
inline const Vector3 Vector3::zzx() const {return Swizzle<2, 2, 0>();}
inline const Vector3 Vector3::zzy() const {return Swizzle<2, 2, 1>();}
inline const Vector3 Vector3::zzz() const {return Swizzle<2, 2, 2>();}
inline const Vector4 Vector3::xxxx() const {return Swizzle<0, 0, 0, 0>();}
inline const Vector4 Vector3::xxxy() const {return Swizzle<0, 0, 0, 1>();}
inline const Vector4 Vector3::xxxz() const {return Swizzle<0, 0, 0, 2>();}
inline const Vector4 Vector3::xxyx() const {return Swizzle<0, 0, 1, 0>();}
inline const Vector4 Vector3::xxyy() const {return Swizzle<0, 0, 1, 1>();}
inline const Vector4 Vector3::xxyz() const {return Swizzle<0, 0, 1, 2>();}
inline const Vector4 Vector3::xxzx() const {return Swizzle<0, 0, 2, 0>();}
inline const Vector4 Vector3::xxzy() const {return Swizzle<0, 0, 2, 1>();}
inline const Vector4 Vector3::xxzz() const {return Swizzle<0, 0, 2, 2>();}
inline const Vector4 Vector3::xyxx() const {return Swizzle<0, 1, 0, 0>();}
inline const Vector4 Vector3::xyxy() const {return Swizzle<0, 1, 0, 1>();}
inline const Vector4 Vector3::xyxz() const {return Swizzle<0, 1, 0, 2>();}
inline const Vector4 Vector3::xyyx() const {return Swizzle<0, 1, 1, 0>();}
inline const Vector4 Vector3::xyyy() const {return Swizzle<0, 1, 1, 1>();}
inline const Vector4 Vector3::xyyz() const {return Swizzle<0, 1, 1, 2>();}
inline const Vector4 Vector3::xyzx() const {return Swizzle<0, 1, 2, 0>();}
inline const Vector4 Vector3::xyzy() const {return Swizzle<0, 1, 2, 1>();}
inline const Vector4 Vector3::xyzz() const {return Swizzle<0, 1, 2, 2>();}
inline const Vector4 Vector3::xzxx() const {return Swizzle<0, 2, 0, 0>();}
inline const Vector4 Vector3::xzxy() const {return Swizzle<0, 2, 0, 1>();}
inline const Vector4 Vector3::xzxz() const {return Swizzle<0, 2, 0, 2>();}
inline const Vector4 Vector3::xzyx() const {return Swizzle<0, 2, 1, 0>();}
inline const Vector4 Vector3::xzyy() const {return Swizzle<0, 2, 1, 1>();}
inline const Vector4 Vector3::xzyz() const {return Swizzle<0, 2, 1, 2>();}
inline const Vector4 Vector3::xzzx() const {return Swizzle<0, 2, 2, 0>();}
inline const Vector4 Vector3::xzzy() const {return Swizzle<0, 2, 2, 1>();}
inline const Vector4 Vector3::xzzz() const {return Swizzle<0, 2, 2, 2>();}
inline const Vector4 Vector3::yxxx() const {return Swizzle<1, 0, 0, 0>();}
inline const Vector4 Vector3::yxxy() const {return Swizzle<1, 0, 0, 1>();}
inline const Vector4 Vector3::yxxz() const {return Swizzle<1, 0, 0, 2>();}
inline const Vector4 Vector3::yxyx() const {return Swizzle<1, 0, 1, 0>();}
inline const Vector4 Vector3::yxyy() const {return Swizzle<1, 0, 1, 1>();}
inline const Vector4 Vector3::yxyz() const {return Swizzle<1, 0, 1, 2>();}
inline const Vector4 Vector3::yxzx() const {return Swizzle<1, 0, 2, 0>();}
inline const Vector4 Vector3::yxzy() const {return Swizzle<1, 0, 2, 1>();}
inline const Vector4 Vector3::yxzz() const {return Swizzle<1, 0, 2, 2>();}
inline const Vector4 Vector3::yyxx() const {return Swizzle<1, 1, 0, 0>();}
inline const Vector4 Vector3::yyxy() const {return Swizzle<1, 1, 0, 1>();}
inline const Vector4 Vector3::yyxz() const {return Swizzle<1, 1, 0, 2>();}
inline const Vector4 Vector3::yyyx() const {return Swizzle<1, 1, 1, 0>();}
inline const Vector4 Vector3::yyyy() const {return Swizzle<1, 1, 1, 1>();}
inline const Vector4 Vector3::yyyz() const {return Swizzle<1, 1, 1, 2>();}
inline const Vector4 Vector3::yyzx() const {return Swizzle<1, 1, 2, 0>();}
inline const Vector4 Vector3::yyzy() const {return Swizzle<1, 1, 2, 1>();}
inline const Vector4 Vector3::yyzz() const {return Swizzle<1, 1, 2, 2>();}
inline const Vector4 Vector3::yzxx() const {return Swizzle<1, 2, 0, 0>();}
inline const Vector4 Vector3::yzxy() const {return Swizzle<1, 2, 0, 1>();}
inline const Vector4 Vector3::yzxz() const {return Swizzle<1, 2, 0, 2>();}
inline const Vector4 Vector3::yzyx() const {return Swizzle<1, 2, 1, 0>();}
inline const Vector4 Vector3::yzyy() const {return Swizzle<1, 2, 1, 1>();}
inline const Vector4 Vector3::yzyz() const {return Swizzle<1, 2, 1, 2>();}
inline const Vector4 Vector3::yzzx() const {return Swizzle<1, 2, 2, 0>();}
inline const Vector4 Vector3::yzzy() const {return Swizzle<1, 2, 2, 1>();}
inline const Vector4 Vector3::yzzz() const {return Swizzle<1, 2, 2, 2>();}
inline const Vector4 Vector3::zxxx() const {return Swizzle<2, 0, 0, 0>();}
inline const Vector4 Vector3::zxxy() const {return Swizzle<2, 0, 0, 1>();}
inline const Vector4 Vector3::zxxz() const {return Swizzle<2, 0, 0, 2>();}
inline const Vector4 Vector3::zxyx() const {return Swizzle<2, 0, 1, 0>();}
inline const Vector4 Vector3::zxyy() const {return Swizzle<2, 0, 1, 1>();}
inline const Vector4 Vector3::zxyz() const {return Swizzle<2, 0, 1, 2>();}
inline const Vector4 Vector3::zxzx() const {return Swizzle<2, 0, 2, 0>();}
inline const Vector4 Vector3::zxzy() const {return Swizzle<2, 0, 2, 1>();}
inline const Vector4 Vector3::zxzz() const {return Swizzle<2, 0, 2, 2>();}
inline const Vector4 Vector3::zyxx() const {return Swizzle<2, 1, 0, 0>();}
inline const Vector4 Vector3::zyxy() const {return Swizzle<2, 1, 0, 1>();}
inline const Vector4 Vector3::zyxz() const {return Swizzle<2, 1, 0, 2>();}
inline const Vector4 Vector3::zyyx() const {return Swizzle<2, 1, 1, 0>();}
inline const Vector4 Vector3::zyyy() const {return Swizzle<2, 1, 1, 1>();}
inline const Vector4 Vector3::zyyz() const {return Swizzle<2, 1, 1, 2>();}
inline const Vector4 Vector3::zyzx() const {return Swizzle<2, 1, 2, 0>();}
inline const Vector4 Vector3::zyzy() const {return Swizzle<2, 1, 2, 1>();}
inline const Vector4 Vector3::zyzz() const {return Swizzle<2, 1, 2, 2>();}
inline const Vector4 Vector3::zzxx() const {return Swizzle<2, 2, 0, 0>();}
inline const Vector4 Vector3::zzxy() const {return Swizzle<2, 2, 0, 1>();}
inline const Vector4 Vector3::zzxz() const {return Swizzle<2, 2, 0, 2>();}
inline const Vector4 Vector3::zzyx() const {return Swizzle<2, 2, 1, 0>();}
inline const Vector4 Vector3::zzyy() const {return Swizzle<2, 2, 1, 1>();}
inline const Vector4 Vector3::zzyz() const {return Swizzle<2, 2, 1, 2>();}
inline const Vector4 Vector3::zzzx() const {return Swizzle<2, 2, 2, 0>();}
inline const Vector4 Vector3::zzzy() const {return Swizzle<2, 2, 2, 1>();}
inline const Vector4 Vector3::zzzz() const {return Swizzle<2, 2, 2, 2>();}

inline Vector4::Vector(float x)
{
#ifdef __SSE__
	data = _mm_set1_ps(x);
#else
	data[0] = x;
	data[1] = x;
	data[2] = x;
	data[3] = x;
#endif
}
inline Vector4::Vector(float x, float y, float z, float w)
{
#ifdef __SSE__
	data = _mm_setr_ps(x, y, z, w);
#else
	data[0] = x;
	data[1] = y;
	data[2] = z;
	data[3] = w;
#endif
}
inline Vector4::Vector(Vector2 vec, float z, float w)
{
#ifdef __SSE__
	__m128 tmp = _mm_unpacklo_ps(_mm_set_ss(z), _mm_set_ss(w));
	data = _mm_movelh_ps(vec, tmp);
#else
	data[0] = vec[0];
	data[1] = vec[1];
	data[2] = z;
	data[3] = w;
#endif
}
inline Vector4::Vector(float x, Vector2 vec, float w)
{
#ifdef __SSE__
	__m128 tmp = _mm_unpacklo_ps(_mm_set_ss(x), _mm_set_ss(w));
	data = _mm_movelh_ps(vec, tmp);
	data = zxyw();
#else
	data[0] = x;
	data[1] = vec[0];
	data[2] = vec[1];
	data[3] = w;
#endif
}
inline Vector4::Vector(float x, float y, Vector2 vec)
{
#ifdef __SSE__
	__m128 tmp = _mm_unpacklo_ps(_mm_set_ss(x), _mm_set_ss(y));
	data = _mm_movelh_ps(tmp, vec);
#else
	data[0] = x;
	data[1] = y;
	data[2] = vec[0];
	data[3] = vec[1];
#endif
}
inline Vector4::Vector(Vector2 vec1, Vector2 vec2)
{
#ifdef __SSE__
	data = _mm_movelh_ps(vec1, vec2);
#else
	data[0] = vec1[0];
	data[1] = vec1[1];
	data[2] = vec2[0];
	data[3] = vec2[1];
#endif
}
inline Vector4::Vector(Vector3 vec, float w)
{
#ifdef __SSE__
	data = _mm_or_ps(_mm_and_ps(vec, SSE_MASK(1, 1, 1, 0)), Vector4(0, 0, 0, w));
#else
	data[0] = vec[0];
	data[1] = vec[1];
	data[2] = vec[2];
	data[3] = w;
#endif
}
inline Vector4::Vector(float x, Vector3 vec)
{
#ifdef __SSE__
	data = _mm_move_ss(vec.xxyz(), _mm_set_ss(x));
#else
	data[0] = x;
	data[1] = vec[0];
	data[2] = vec[1];
	data[3] = vec[2];
#endif
}

#ifdef __SSE__
inline Vector4::Vector(__m128 x)
{
	data = x;
}
inline Vector4& Vector4::operator=(__m128 x)
{
	data = x;
	return *this;
}
#endif

inline float Vector4::x() const {return Swizzle<0>();}
inline float Vector4::y() const {return Swizzle<1>();}
inline float Vector4::z() const {return Swizzle<2>();}
inline float Vector4::w() const {return Swizzle<3>();}
inline const Vector2 Vector4::xx() const {return Swizzle<0, 0>();}
inline const Vector2 Vector4::xy() const {return Swizzle<0, 1>();}
inline const Vector2 Vector4::xz() const {return Swizzle<0, 2>();}
inline const Vector2 Vector4::xw() const {return Swizzle<0, 3>();}
inline const Vector2 Vector4::yx() const {return Swizzle<1, 0>();}
inline const Vector2 Vector4::yy() const {return Swizzle<1, 1>();}
inline const Vector2 Vector4::yz() const {return Swizzle<1, 2>();}
inline const Vector2 Vector4::yw() const {return Swizzle<1, 3>();}
inline const Vector2 Vector4::zx() const {return Swizzle<2, 0>();}
inline const Vector2 Vector4::zy() const {return Swizzle<2, 1>();}
inline const Vector2 Vector4::zz() const {return Swizzle<2, 2>();}
inline const Vector2 Vector4::zw() const {return Swizzle<2, 3>();}
inline const Vector2 Vector4::wx() const {return Swizzle<3, 0>();}
inline const Vector2 Vector4::wy() const {return Swizzle<3, 1>();}
inline const Vector2 Vector4::wz() const {return Swizzle<3, 2>();}
inline const Vector2 Vector4::ww() const {return Swizzle<3, 3>();}
inline const Vector3 Vector4::xxx() const {return Swizzle<0, 0, 0>();}
inline const Vector3 Vector4::xxy() const {return Swizzle<0, 0, 1>();}
inline const Vector3 Vector4::xxz() const {return Swizzle<0, 0, 2>();}
inline const Vector3 Vector4::xxw() const {return Swizzle<0, 0, 3>();}
inline const Vector3 Vector4::xyx() const {return Swizzle<0, 1, 0>();}
inline const Vector3 Vector4::xyy() const {return Swizzle<0, 1, 1>();}
inline const Vector3 Vector4::xyz() const {return Swizzle<0, 1, 2>();}
inline const Vector3 Vector4::xyw() const {return Swizzle<0, 1, 3>();}
inline const Vector3 Vector4::xzx() const {return Swizzle<0, 2, 0>();}
inline const Vector3 Vector4::xzy() const {return Swizzle<0, 2, 1>();}
inline const Vector3 Vector4::xzz() const {return Swizzle<0, 2, 2>();}
inline const Vector3 Vector4::xzw() const {return Swizzle<0, 2, 3>();}
inline const Vector3 Vector4::xwx() const {return Swizzle<0, 3, 0>();}
inline const Vector3 Vector4::xwy() const {return Swizzle<0, 3, 1>();}
inline const Vector3 Vector4::xwz() const {return Swizzle<0, 3, 2>();}
inline const Vector3 Vector4::xww() const {return Swizzle<0, 3, 3>();}
inline const Vector3 Vector4::yxx() const {return Swizzle<1, 0, 0>();}
inline const Vector3 Vector4::yxy() const {return Swizzle<1, 0, 1>();}
inline const Vector3 Vector4::yxz() const {return Swizzle<1, 0, 2>();}
inline const Vector3 Vector4::yxw() const {return Swizzle<1, 0, 3>();}
inline const Vector3 Vector4::yyx() const {return Swizzle<1, 1, 0>();}
inline const Vector3 Vector4::yyy() const {return Swizzle<1, 1, 1>();}
inline const Vector3 Vector4::yyz() const {return Swizzle<1, 1, 2>();}
inline const Vector3 Vector4::yyw() const {return Swizzle<1, 1, 3>();}
inline const Vector3 Vector4::yzx() const {return Swizzle<1, 2, 0>();}
inline const Vector3 Vector4::yzy() const {return Swizzle<1, 2, 1>();}
inline const Vector3 Vector4::yzz() const {return Swizzle<1, 2, 2>();}
inline const Vector3 Vector4::yzw() const {return Swizzle<1, 2, 3>();}
inline const Vector3 Vector4::ywx() const {return Swizzle<1, 3, 0>();}
inline const Vector3 Vector4::ywy() const {return Swizzle<1, 3, 1>();}
inline const Vector3 Vector4::ywz() const {return Swizzle<1, 3, 2>();}
inline const Vector3 Vector4::yww() const {return Swizzle<1, 3, 3>();}
inline const Vector3 Vector4::zxx() const {return Swizzle<2, 0, 0>();}
inline const Vector3 Vector4::zxy() const {return Swizzle<2, 0, 1>();}
inline const Vector3 Vector4::zxz() const {return Swizzle<2, 0, 2>();}
inline const Vector3 Vector4::zxw() const {return Swizzle<2, 0, 3>();}
inline const Vector3 Vector4::zyx() const {return Swizzle<2, 1, 0>();}
inline const Vector3 Vector4::zyy() const {return Swizzle<2, 1, 1>();}
inline const Vector3 Vector4::zyz() const {return Swizzle<2, 1, 2>();}
inline const Vector3 Vector4::zyw() const {return Swizzle<2, 1, 3>();}
inline const Vector3 Vector4::zzx() const {return Swizzle<2, 2, 0>();}
inline const Vector3 Vector4::zzy() const {return Swizzle<2, 2, 1>();}
inline const Vector3 Vector4::zzz() const {return Swizzle<2, 2, 2>();}
inline const Vector3 Vector4::zzw() const {return Swizzle<2, 2, 3>();}
inline const Vector3 Vector4::zwx() const {return Swizzle<2, 3, 0>();}
inline const Vector3 Vector4::zwy() const {return Swizzle<2, 3, 1>();}
inline const Vector3 Vector4::zwz() const {return Swizzle<2, 3, 2>();}
inline const Vector3 Vector4::zww() const {return Swizzle<2, 3, 3>();}
inline const Vector3 Vector4::wxx() const {return Swizzle<3, 0, 0>();}
inline const Vector3 Vector4::wxy() const {return Swizzle<3, 0, 1>();}
inline const Vector3 Vector4::wxz() const {return Swizzle<3, 0, 2>();}
inline const Vector3 Vector4::wxw() const {return Swizzle<3, 0, 3>();}
inline const Vector3 Vector4::wyx() const {return Swizzle<3, 1, 0>();}
inline const Vector3 Vector4::wyy() const {return Swizzle<3, 1, 1>();}
inline const Vector3 Vector4::wyz() const {return Swizzle<3, 1, 2>();}
inline const Vector3 Vector4::wyw() const {return Swizzle<3, 1, 3>();}
inline const Vector3 Vector4::wzx() const {return Swizzle<3, 2, 0>();}
inline const Vector3 Vector4::wzy() const {return Swizzle<3, 2, 1>();}
inline const Vector3 Vector4::wzz() const {return Swizzle<3, 2, 2>();}
inline const Vector3 Vector4::wzw() const {return Swizzle<3, 2, 3>();}
inline const Vector3 Vector4::wwx() const {return Swizzle<3, 3, 0>();}
inline const Vector3 Vector4::wwy() const {return Swizzle<3, 3, 1>();}
inline const Vector3 Vector4::wwz() const {return Swizzle<3, 3, 2>();}
inline const Vector3 Vector4::www() const {return Swizzle<3, 3, 3>();}
inline const Vector4 Vector4::xxxx() const {return Swizzle<0, 0, 0, 0>();}
inline const Vector4 Vector4::xxxy() const {return Swizzle<0, 0, 0, 1>();}
inline const Vector4 Vector4::xxxz() const {return Swizzle<0, 0, 0, 2>();}
inline const Vector4 Vector4::xxxw() const {return Swizzle<0, 0, 0, 3>();}
inline const Vector4 Vector4::xxyx() const {return Swizzle<0, 0, 1, 0>();}
inline const Vector4 Vector4::xxyy() const {return Swizzle<0, 0, 1, 1>();}
inline const Vector4 Vector4::xxyz() const {return Swizzle<0, 0, 1, 2>();}
inline const Vector4 Vector4::xxyw() const {return Swizzle<0, 0, 1, 3>();}
inline const Vector4 Vector4::xxzx() const {return Swizzle<0, 0, 2, 0>();}
inline const Vector4 Vector4::xxzy() const {return Swizzle<0, 0, 2, 1>();}
inline const Vector4 Vector4::xxzz() const {return Swizzle<0, 0, 2, 2>();}
inline const Vector4 Vector4::xxzw() const {return Swizzle<0, 0, 2, 3>();}
inline const Vector4 Vector4::xxwx() const {return Swizzle<0, 0, 3, 0>();}
inline const Vector4 Vector4::xxwy() const {return Swizzle<0, 0, 3, 1>();}
inline const Vector4 Vector4::xxwz() const {return Swizzle<0, 0, 3, 2>();}
inline const Vector4 Vector4::xxww() const {return Swizzle<0, 0, 3, 3>();}
inline const Vector4 Vector4::xyxx() const {return Swizzle<0, 1, 0, 0>();}
inline const Vector4 Vector4::xyxy() const {return Swizzle<0, 1, 0, 1>();}
inline const Vector4 Vector4::xyxz() const {return Swizzle<0, 1, 0, 2>();}
inline const Vector4 Vector4::xyxw() const {return Swizzle<0, 1, 0, 3>();}
inline const Vector4 Vector4::xyyx() const {return Swizzle<0, 1, 1, 0>();}
inline const Vector4 Vector4::xyyy() const {return Swizzle<0, 1, 1, 1>();}
inline const Vector4 Vector4::xyyz() const {return Swizzle<0, 1, 1, 2>();}
inline const Vector4 Vector4::xyyw() const {return Swizzle<0, 1, 1, 3>();}
inline const Vector4 Vector4::xyzx() const {return Swizzle<0, 1, 2, 0>();}
inline const Vector4 Vector4::xyzy() const {return Swizzle<0, 1, 2, 1>();}
inline const Vector4 Vector4::xyzz() const {return Swizzle<0, 1, 2, 2>();}
inline const Vector4 Vector4::xyzw() const {return Swizzle<0, 1, 2, 3>();}
inline const Vector4 Vector4::xywx() const {return Swizzle<0, 1, 3, 0>();}
inline const Vector4 Vector4::xywy() const {return Swizzle<0, 1, 3, 1>();}
inline const Vector4 Vector4::xywz() const {return Swizzle<0, 1, 3, 2>();}
inline const Vector4 Vector4::xyww() const {return Swizzle<0, 1, 3, 3>();}
inline const Vector4 Vector4::xzxx() const {return Swizzle<0, 2, 0, 0>();}
inline const Vector4 Vector4::xzxy() const {return Swizzle<0, 2, 0, 1>();}
inline const Vector4 Vector4::xzxz() const {return Swizzle<0, 2, 0, 2>();}
inline const Vector4 Vector4::xzxw() const {return Swizzle<0, 2, 0, 3>();}
inline const Vector4 Vector4::xzyx() const {return Swizzle<0, 2, 1, 0>();}
inline const Vector4 Vector4::xzyy() const {return Swizzle<0, 2, 1, 1>();}
inline const Vector4 Vector4::xzyz() const {return Swizzle<0, 2, 1, 2>();}
inline const Vector4 Vector4::xzyw() const {return Swizzle<0, 2, 1, 3>();}
inline const Vector4 Vector4::xzzx() const {return Swizzle<0, 2, 2, 0>();}
inline const Vector4 Vector4::xzzy() const {return Swizzle<0, 2, 2, 1>();}
inline const Vector4 Vector4::xzzz() const {return Swizzle<0, 2, 2, 2>();}
inline const Vector4 Vector4::xzzw() const {return Swizzle<0, 2, 2, 3>();}
inline const Vector4 Vector4::xzwx() const {return Swizzle<0, 2, 3, 0>();}
inline const Vector4 Vector4::xzwy() const {return Swizzle<0, 2, 3, 1>();}
inline const Vector4 Vector4::xzwz() const {return Swizzle<0, 2, 3, 2>();}
inline const Vector4 Vector4::xzww() const {return Swizzle<0, 2, 3, 3>();}
inline const Vector4 Vector4::xwxx() const {return Swizzle<0, 3, 0, 0>();}
inline const Vector4 Vector4::xwxy() const {return Swizzle<0, 3, 0, 1>();}
inline const Vector4 Vector4::xwxz() const {return Swizzle<0, 3, 0, 2>();}
inline const Vector4 Vector4::xwxw() const {return Swizzle<0, 3, 0, 3>();}
inline const Vector4 Vector4::xwyx() const {return Swizzle<0, 3, 1, 0>();}
inline const Vector4 Vector4::xwyy() const {return Swizzle<0, 3, 1, 1>();}
inline const Vector4 Vector4::xwyz() const {return Swizzle<0, 3, 1, 2>();}
inline const Vector4 Vector4::xwyw() const {return Swizzle<0, 3, 1, 3>();}
inline const Vector4 Vector4::xwzx() const {return Swizzle<0, 3, 2, 0>();}
inline const Vector4 Vector4::xwzy() const {return Swizzle<0, 3, 2, 1>();}
inline const Vector4 Vector4::xwzz() const {return Swizzle<0, 3, 2, 2>();}
inline const Vector4 Vector4::xwzw() const {return Swizzle<0, 3, 2, 3>();}
inline const Vector4 Vector4::xwwx() const {return Swizzle<0, 3, 3, 0>();}
inline const Vector4 Vector4::xwwy() const {return Swizzle<0, 3, 3, 1>();}
inline const Vector4 Vector4::xwwz() const {return Swizzle<0, 3, 3, 2>();}
inline const Vector4 Vector4::xwww() const {return Swizzle<0, 3, 3, 3>();}
inline const Vector4 Vector4::yxxx() const {return Swizzle<1, 0, 0, 0>();}
inline const Vector4 Vector4::yxxy() const {return Swizzle<1, 0, 0, 1>();}
inline const Vector4 Vector4::yxxz() const {return Swizzle<1, 0, 0, 2>();}
inline const Vector4 Vector4::yxxw() const {return Swizzle<1, 0, 0, 3>();}
inline const Vector4 Vector4::yxyx() const {return Swizzle<1, 0, 1, 0>();}
inline const Vector4 Vector4::yxyy() const {return Swizzle<1, 0, 1, 1>();}
inline const Vector4 Vector4::yxyz() const {return Swizzle<1, 0, 1, 2>();}
inline const Vector4 Vector4::yxyw() const {return Swizzle<1, 0, 1, 3>();}
inline const Vector4 Vector4::yxzx() const {return Swizzle<1, 0, 2, 0>();}
inline const Vector4 Vector4::yxzy() const {return Swizzle<1, 0, 2, 1>();}
inline const Vector4 Vector4::yxzz() const {return Swizzle<1, 0, 2, 2>();}
inline const Vector4 Vector4::yxzw() const {return Swizzle<1, 0, 2, 3>();}
inline const Vector4 Vector4::yxwx() const {return Swizzle<1, 0, 3, 0>();}
inline const Vector4 Vector4::yxwy() const {return Swizzle<1, 0, 3, 1>();}
inline const Vector4 Vector4::yxwz() const {return Swizzle<1, 0, 3, 2>();}
inline const Vector4 Vector4::yxww() const {return Swizzle<1, 0, 3, 3>();}
inline const Vector4 Vector4::yyxx() const {return Swizzle<1, 1, 0, 0>();}
inline const Vector4 Vector4::yyxy() const {return Swizzle<1, 1, 0, 1>();}
inline const Vector4 Vector4::yyxz() const {return Swizzle<1, 1, 0, 2>();}
inline const Vector4 Vector4::yyxw() const {return Swizzle<1, 1, 0, 3>();}
inline const Vector4 Vector4::yyyx() const {return Swizzle<1, 1, 1, 0>();}
inline const Vector4 Vector4::yyyy() const {return Swizzle<1, 1, 1, 1>();}
inline const Vector4 Vector4::yyyz() const {return Swizzle<1, 1, 1, 2>();}
inline const Vector4 Vector4::yyyw() const {return Swizzle<1, 1, 1, 3>();}
inline const Vector4 Vector4::yyzx() const {return Swizzle<1, 1, 2, 0>();}
inline const Vector4 Vector4::yyzy() const {return Swizzle<1, 1, 2, 1>();}
inline const Vector4 Vector4::yyzz() const {return Swizzle<1, 1, 2, 2>();}
inline const Vector4 Vector4::yyzw() const {return Swizzle<1, 1, 2, 3>();}
inline const Vector4 Vector4::yywx() const {return Swizzle<1, 1, 3, 0>();}
inline const Vector4 Vector4::yywy() const {return Swizzle<1, 1, 3, 1>();}
inline const Vector4 Vector4::yywz() const {return Swizzle<1, 1, 3, 2>();}
inline const Vector4 Vector4::yyww() const {return Swizzle<1, 1, 3, 3>();}
inline const Vector4 Vector4::yzxx() const {return Swizzle<1, 2, 0, 0>();}
inline const Vector4 Vector4::yzxy() const {return Swizzle<1, 2, 0, 1>();}
inline const Vector4 Vector4::yzxz() const {return Swizzle<1, 2, 0, 2>();}
inline const Vector4 Vector4::yzxw() const {return Swizzle<1, 2, 0, 3>();}
inline const Vector4 Vector4::yzyx() const {return Swizzle<1, 2, 1, 0>();}
inline const Vector4 Vector4::yzyy() const {return Swizzle<1, 2, 1, 1>();}
inline const Vector4 Vector4::yzyz() const {return Swizzle<1, 2, 1, 2>();}
inline const Vector4 Vector4::yzyw() const {return Swizzle<1, 2, 1, 3>();}
inline const Vector4 Vector4::yzzx() const {return Swizzle<1, 2, 2, 0>();}
inline const Vector4 Vector4::yzzy() const {return Swizzle<1, 2, 2, 1>();}
inline const Vector4 Vector4::yzzz() const {return Swizzle<1, 2, 2, 2>();}
inline const Vector4 Vector4::yzzw() const {return Swizzle<1, 2, 2, 3>();}
inline const Vector4 Vector4::yzwx() const {return Swizzle<1, 2, 3, 0>();}
inline const Vector4 Vector4::yzwy() const {return Swizzle<1, 2, 3, 1>();}
inline const Vector4 Vector4::yzwz() const {return Swizzle<1, 2, 3, 2>();}
inline const Vector4 Vector4::yzww() const {return Swizzle<1, 2, 3, 3>();}
inline const Vector4 Vector4::ywxx() const {return Swizzle<1, 3, 0, 0>();}
inline const Vector4 Vector4::ywxy() const {return Swizzle<1, 3, 0, 1>();}
inline const Vector4 Vector4::ywxz() const {return Swizzle<1, 3, 0, 2>();}
inline const Vector4 Vector4::ywxw() const {return Swizzle<1, 3, 0, 3>();}
inline const Vector4 Vector4::ywyx() const {return Swizzle<1, 3, 1, 0>();}
inline const Vector4 Vector4::ywyy() const {return Swizzle<1, 3, 1, 1>();}
inline const Vector4 Vector4::ywyz() const {return Swizzle<1, 3, 1, 2>();}
inline const Vector4 Vector4::ywyw() const {return Swizzle<1, 3, 1, 3>();}
inline const Vector4 Vector4::ywzx() const {return Swizzle<1, 3, 2, 0>();}
inline const Vector4 Vector4::ywzy() const {return Swizzle<1, 3, 2, 1>();}
inline const Vector4 Vector4::ywzz() const {return Swizzle<1, 3, 2, 2>();}
inline const Vector4 Vector4::ywzw() const {return Swizzle<1, 3, 2, 3>();}
inline const Vector4 Vector4::ywwx() const {return Swizzle<1, 3, 3, 0>();}
inline const Vector4 Vector4::ywwy() const {return Swizzle<1, 3, 3, 1>();}
inline const Vector4 Vector4::ywwz() const {return Swizzle<1, 3, 3, 2>();}
inline const Vector4 Vector4::ywww() const {return Swizzle<1, 3, 3, 3>();}
inline const Vector4 Vector4::zxxx() const {return Swizzle<2, 0, 0, 0>();}
inline const Vector4 Vector4::zxxy() const {return Swizzle<2, 0, 0, 1>();}
inline const Vector4 Vector4::zxxz() const {return Swizzle<2, 0, 0, 2>();}
inline const Vector4 Vector4::zxxw() const {return Swizzle<2, 0, 0, 3>();}
inline const Vector4 Vector4::zxyx() const {return Swizzle<2, 0, 1, 0>();}
inline const Vector4 Vector4::zxyy() const {return Swizzle<2, 0, 1, 1>();}
inline const Vector4 Vector4::zxyz() const {return Swizzle<2, 0, 1, 2>();}
inline const Vector4 Vector4::zxyw() const {return Swizzle<2, 0, 1, 3>();}
inline const Vector4 Vector4::zxzx() const {return Swizzle<2, 0, 2, 0>();}
inline const Vector4 Vector4::zxzy() const {return Swizzle<2, 0, 2, 1>();}
inline const Vector4 Vector4::zxzz() const {return Swizzle<2, 0, 2, 2>();}
inline const Vector4 Vector4::zxzw() const {return Swizzle<2, 0, 2, 3>();}
inline const Vector4 Vector4::zxwx() const {return Swizzle<2, 0, 3, 0>();}
inline const Vector4 Vector4::zxwy() const {return Swizzle<2, 0, 3, 1>();}
inline const Vector4 Vector4::zxwz() const {return Swizzle<2, 0, 3, 2>();}
inline const Vector4 Vector4::zxww() const {return Swizzle<2, 0, 3, 3>();}
inline const Vector4 Vector4::zyxx() const {return Swizzle<2, 1, 0, 0>();}
inline const Vector4 Vector4::zyxy() const {return Swizzle<2, 1, 0, 1>();}
inline const Vector4 Vector4::zyxz() const {return Swizzle<2, 1, 0, 2>();}
inline const Vector4 Vector4::zyxw() const {return Swizzle<2, 1, 0, 3>();}
inline const Vector4 Vector4::zyyx() const {return Swizzle<2, 1, 1, 0>();}
inline const Vector4 Vector4::zyyy() const {return Swizzle<2, 1, 1, 1>();}
inline const Vector4 Vector4::zyyz() const {return Swizzle<2, 1, 1, 2>();}
inline const Vector4 Vector4::zyyw() const {return Swizzle<2, 1, 1, 3>();}
inline const Vector4 Vector4::zyzx() const {return Swizzle<2, 1, 2, 0>();}
inline const Vector4 Vector4::zyzy() const {return Swizzle<2, 1, 2, 1>();}
inline const Vector4 Vector4::zyzz() const {return Swizzle<2, 1, 2, 2>();}
inline const Vector4 Vector4::zyzw() const {return Swizzle<2, 1, 2, 3>();}
inline const Vector4 Vector4::zywx() const {return Swizzle<2, 1, 3, 0>();}
inline const Vector4 Vector4::zywy() const {return Swizzle<2, 1, 3, 1>();}
inline const Vector4 Vector4::zywz() const {return Swizzle<2, 1, 3, 2>();}
inline const Vector4 Vector4::zyww() const {return Swizzle<2, 1, 3, 3>();}
inline const Vector4 Vector4::zzxx() const {return Swizzle<2, 2, 0, 0>();}
inline const Vector4 Vector4::zzxy() const {return Swizzle<2, 2, 0, 1>();}
inline const Vector4 Vector4::zzxz() const {return Swizzle<2, 2, 0, 2>();}
inline const Vector4 Vector4::zzxw() const {return Swizzle<2, 2, 0, 3>();}
inline const Vector4 Vector4::zzyx() const {return Swizzle<2, 2, 1, 0>();}
inline const Vector4 Vector4::zzyy() const {return Swizzle<2, 2, 1, 1>();}
inline const Vector4 Vector4::zzyz() const {return Swizzle<2, 2, 1, 2>();}
inline const Vector4 Vector4::zzyw() const {return Swizzle<2, 2, 1, 3>();}
inline const Vector4 Vector4::zzzx() const {return Swizzle<2, 2, 2, 0>();}
inline const Vector4 Vector4::zzzy() const {return Swizzle<2, 2, 2, 1>();}
inline const Vector4 Vector4::zzzz() const {return Swizzle<2, 2, 2, 2>();}
inline const Vector4 Vector4::zzzw() const {return Swizzle<2, 2, 2, 3>();}
inline const Vector4 Vector4::zzwx() const {return Swizzle<2, 2, 3, 0>();}
inline const Vector4 Vector4::zzwy() const {return Swizzle<2, 2, 3, 1>();}
inline const Vector4 Vector4::zzwz() const {return Swizzle<2, 2, 3, 2>();}
inline const Vector4 Vector4::zzww() const {return Swizzle<2, 2, 3, 3>();}
inline const Vector4 Vector4::zwxx() const {return Swizzle<2, 3, 0, 0>();}
inline const Vector4 Vector4::zwxy() const {return Swizzle<2, 3, 0, 1>();}
inline const Vector4 Vector4::zwxz() const {return Swizzle<2, 3, 0, 2>();}
inline const Vector4 Vector4::zwxw() const {return Swizzle<2, 3, 0, 3>();}
inline const Vector4 Vector4::zwyx() const {return Swizzle<2, 3, 1, 0>();}
inline const Vector4 Vector4::zwyy() const {return Swizzle<2, 3, 1, 1>();}
inline const Vector4 Vector4::zwyz() const {return Swizzle<2, 3, 1, 2>();}
inline const Vector4 Vector4::zwyw() const {return Swizzle<2, 3, 1, 3>();}
inline const Vector4 Vector4::zwzx() const {return Swizzle<2, 3, 2, 0>();}
inline const Vector4 Vector4::zwzy() const {return Swizzle<2, 3, 2, 1>();}
inline const Vector4 Vector4::zwzz() const {return Swizzle<2, 3, 2, 2>();}
inline const Vector4 Vector4::zwzw() const {return Swizzle<2, 3, 2, 3>();}
inline const Vector4 Vector4::zwwx() const {return Swizzle<2, 3, 3, 0>();}
inline const Vector4 Vector4::zwwy() const {return Swizzle<2, 3, 3, 1>();}
inline const Vector4 Vector4::zwwz() const {return Swizzle<2, 3, 3, 2>();}
inline const Vector4 Vector4::zwww() const {return Swizzle<2, 3, 3, 3>();}
inline const Vector4 Vector4::wxxx() const {return Swizzle<3, 0, 0, 0>();}
inline const Vector4 Vector4::wxxy() const {return Swizzle<3, 0, 0, 1>();}
inline const Vector4 Vector4::wxxz() const {return Swizzle<3, 0, 0, 2>();}
inline const Vector4 Vector4::wxxw() const {return Swizzle<3, 0, 0, 3>();}
inline const Vector4 Vector4::wxyx() const {return Swizzle<3, 0, 1, 0>();}
inline const Vector4 Vector4::wxyy() const {return Swizzle<3, 0, 1, 1>();}
inline const Vector4 Vector4::wxyz() const {return Swizzle<3, 0, 1, 2>();}
inline const Vector4 Vector4::wxyw() const {return Swizzle<3, 0, 1, 3>();}
inline const Vector4 Vector4::wxzx() const {return Swizzle<3, 0, 2, 0>();}
inline const Vector4 Vector4::wxzy() const {return Swizzle<3, 0, 2, 1>();}
inline const Vector4 Vector4::wxzz() const {return Swizzle<3, 0, 2, 2>();}
inline const Vector4 Vector4::wxzw() const {return Swizzle<3, 0, 2, 3>();}
inline const Vector4 Vector4::wxwx() const {return Swizzle<3, 0, 3, 0>();}
inline const Vector4 Vector4::wxwy() const {return Swizzle<3, 0, 3, 1>();}
inline const Vector4 Vector4::wxwz() const {return Swizzle<3, 0, 3, 2>();}
inline const Vector4 Vector4::wxww() const {return Swizzle<3, 0, 3, 3>();}
inline const Vector4 Vector4::wyxx() const {return Swizzle<3, 1, 0, 0>();}
inline const Vector4 Vector4::wyxy() const {return Swizzle<3, 1, 0, 1>();}
inline const Vector4 Vector4::wyxz() const {return Swizzle<3, 1, 0, 2>();}
inline const Vector4 Vector4::wyxw() const {return Swizzle<3, 1, 0, 3>();}
inline const Vector4 Vector4::wyyx() const {return Swizzle<3, 1, 1, 0>();}
inline const Vector4 Vector4::wyyy() const {return Swizzle<3, 1, 1, 1>();}
inline const Vector4 Vector4::wyyz() const {return Swizzle<3, 1, 1, 2>();}
inline const Vector4 Vector4::wyyw() const {return Swizzle<3, 1, 1, 3>();}
inline const Vector4 Vector4::wyzx() const {return Swizzle<3, 1, 2, 0>();}
inline const Vector4 Vector4::wyzy() const {return Swizzle<3, 1, 2, 1>();}
inline const Vector4 Vector4::wyzz() const {return Swizzle<3, 1, 2, 2>();}
inline const Vector4 Vector4::wyzw() const {return Swizzle<3, 1, 2, 3>();}
inline const Vector4 Vector4::wywx() const {return Swizzle<3, 1, 3, 0>();}
inline const Vector4 Vector4::wywy() const {return Swizzle<3, 1, 3, 1>();}
inline const Vector4 Vector4::wywz() const {return Swizzle<3, 1, 3, 2>();}
inline const Vector4 Vector4::wyww() const {return Swizzle<3, 1, 3, 3>();}
inline const Vector4 Vector4::wzxx() const {return Swizzle<3, 2, 0, 0>();}
inline const Vector4 Vector4::wzxy() const {return Swizzle<3, 2, 0, 1>();}
inline const Vector4 Vector4::wzxz() const {return Swizzle<3, 2, 0, 2>();}
inline const Vector4 Vector4::wzxw() const {return Swizzle<3, 2, 0, 3>();}
inline const Vector4 Vector4::wzyx() const {return Swizzle<3, 2, 1, 0>();}
inline const Vector4 Vector4::wzyy() const {return Swizzle<3, 2, 1, 1>();}
inline const Vector4 Vector4::wzyz() const {return Swizzle<3, 2, 1, 2>();}
inline const Vector4 Vector4::wzyw() const {return Swizzle<3, 2, 1, 3>();}
inline const Vector4 Vector4::wzzx() const {return Swizzle<3, 2, 2, 0>();}
inline const Vector4 Vector4::wzzy() const {return Swizzle<3, 2, 2, 1>();}
inline const Vector4 Vector4::wzzz() const {return Swizzle<3, 2, 2, 2>();}
inline const Vector4 Vector4::wzzw() const {return Swizzle<3, 2, 2, 3>();}
inline const Vector4 Vector4::wzwx() const {return Swizzle<3, 2, 3, 0>();}
inline const Vector4 Vector4::wzwy() const {return Swizzle<3, 2, 3, 1>();}
inline const Vector4 Vector4::wzwz() const {return Swizzle<3, 2, 3, 2>();}
inline const Vector4 Vector4::wzww() const {return Swizzle<3, 2, 3, 3>();}
inline const Vector4 Vector4::wwxx() const {return Swizzle<3, 3, 0, 0>();}
inline const Vector4 Vector4::wwxy() const {return Swizzle<3, 3, 0, 1>();}
inline const Vector4 Vector4::wwxz() const {return Swizzle<3, 3, 0, 2>();}
inline const Vector4 Vector4::wwxw() const {return Swizzle<3, 3, 0, 3>();}
inline const Vector4 Vector4::wwyx() const {return Swizzle<3, 3, 1, 0>();}
inline const Vector4 Vector4::wwyy() const {return Swizzle<3, 3, 1, 1>();}
inline const Vector4 Vector4::wwyz() const {return Swizzle<3, 3, 1, 2>();}
inline const Vector4 Vector4::wwyw() const {return Swizzle<3, 3, 1, 3>();}
inline const Vector4 Vector4::wwzx() const {return Swizzle<3, 3, 2, 0>();}
inline const Vector4 Vector4::wwzy() const {return Swizzle<3, 3, 2, 1>();}
inline const Vector4 Vector4::wwzz() const {return Swizzle<3, 3, 2, 2>();}
inline const Vector4 Vector4::wwzw() const {return Swizzle<3, 3, 2, 3>();}
inline const Vector4 Vector4::wwwx() const {return Swizzle<3, 3, 3, 0>();}
inline const Vector4 Vector4::wwwy() const {return Swizzle<3, 3, 3, 1>();}
inline const Vector4 Vector4::wwwz() const {return Swizzle<3, 3, 3, 2>();}
inline const Vector4 Vector4::wwww() const {return Swizzle<3, 3, 3, 3>();}

inline BVector2::BVector(bool x)
{
#ifdef __SSE__
	if (x)
		data = SSE_MASK(1, 1, 1, 1);
	else
		data = _mm_setzero_ps();
#else
	data[0] = x;
	data[1] = x;
#endif
}
inline BVector2::BVector(bool x, bool y)
{
#ifdef __SSE__
	data = SSE_MASK(x, y, 0, 0);
#else
	data[0] = x;
	data[1] = y;
#endif
}

#ifdef __SSE__
inline BVector2::BVector(__m128 x)
{
	data = x;
}
inline BVector2& BVector2::operator=(__m128 x)
{
	data = x;
	return *this;
}
#endif

inline bool BVector2::x() const {return Swizzle<0>();}
inline bool BVector2::y() const {return Swizzle<1>();}
inline const BVector2 BVector2::xx() const {return Swizzle<0, 0>();}
inline const BVector2 BVector2::xy() const {return Swizzle<0, 1>();}
inline const BVector2 BVector2::yx() const {return Swizzle<1, 0>();}
inline const BVector2 BVector2::yy() const {return Swizzle<1, 1>();}
inline const BVector3 BVector2::xxx() const {return Swizzle<0, 0, 0>();}
inline const BVector3 BVector2::xxy() const {return Swizzle<0, 0, 1>();}
inline const BVector3 BVector2::xyx() const {return Swizzle<0, 1, 0>();}
inline const BVector3 BVector2::xyy() const {return Swizzle<0, 1, 1>();}
inline const BVector3 BVector2::yxx() const {return Swizzle<1, 0, 0>();}
inline const BVector3 BVector2::yxy() const {return Swizzle<1, 0, 1>();}
inline const BVector3 BVector2::yyx() const {return Swizzle<1, 1, 0>();}
inline const BVector3 BVector2::yyy() const {return Swizzle<1, 1, 1>();}
inline const BVector4 BVector2::xxxx() const {return Swizzle<0, 0, 0, 0>();}
inline const BVector4 BVector2::xxxy() const {return Swizzle<0, 0, 0, 1>();}
inline const BVector4 BVector2::xxyx() const {return Swizzle<0, 0, 1, 0>();}
inline const BVector4 BVector2::xxyy() const {return Swizzle<0, 0, 1, 1>();}
inline const BVector4 BVector2::xyxx() const {return Swizzle<0, 1, 0, 0>();}
inline const BVector4 BVector2::xyxy() const {return Swizzle<0, 1, 0, 1>();}
inline const BVector4 BVector2::xyyx() const {return Swizzle<0, 1, 1, 0>();}
inline const BVector4 BVector2::xyyy() const {return Swizzle<0, 1, 1, 1>();}
inline const BVector4 BVector2::yxxx() const {return Swizzle<1, 0, 0, 0>();}
inline const BVector4 BVector2::yxxy() const {return Swizzle<1, 0, 0, 1>();}
inline const BVector4 BVector2::yxyx() const {return Swizzle<1, 0, 1, 0>();}
inline const BVector4 BVector2::yxyy() const {return Swizzle<1, 0, 1, 1>();}
inline const BVector4 BVector2::yyxx() const {return Swizzle<1, 1, 0, 0>();}
inline const BVector4 BVector2::yyxy() const {return Swizzle<1, 1, 0, 1>();}
inline const BVector4 BVector2::yyyx() const {return Swizzle<1, 1, 1, 0>();}
inline const BVector4 BVector2::yyyy() const {return Swizzle<1, 1, 1, 1>();}

inline BVector3::BVector(bool x)
{
#ifdef __SSE__
	if (x)
		data = SSE_MASK(1, 1, 1, 1);
	else
		data = _mm_setzero_ps();
#else
	data[0] = x;
	data[1] = x;
	data[2] = x;
#endif
}
inline BVector3::BVector(bool x, bool y, bool z)
{
#ifdef __SSE__
	data = SSE_MASK(x, y, z, 0);
#else
	data[0] = x;
	data[1] = y;
	data[2] = z;
#endif
}
inline BVector3::BVector(BVector2 vec, bool z)
{
#ifdef __SSE__
	if (z)
		data = _mm_or_ps(vec, SSE_MASK(0, 0, 1, 0));
	else
		data = _mm_andnot_ps(vec, SSE_MASK(0, 0, 1, 0));
#else
	data[0] = vec[0];
	data[1] = vec[1];
	data[2] = z;
#endif
}
inline BVector3::BVector(bool x, BVector2 vec)
{
#ifdef __SSE__
	if (x)
		data = _mm_move_ss(_mm_unpacklo_ps(vec, vec), SSE_MASK(1, 1, 1, 1));
	else
		data = _mm_move_ss(_mm_unpacklo_ps(vec, vec), _mm_setzero_ps());
#else
	data[0] = x;
	data[1] = vec[0];
	data[2] = vec[1];
#endif
}

#ifdef __SSE__
inline BVector3::BVector(__m128 x)
{
	data = x;
}
inline BVector3& BVector3::operator=(__m128 x)
{
	data = x;
	return *this;
}
#endif

inline bool BVector3::x() const {return Swizzle<0>();}
inline bool BVector3::y() const {return Swizzle<1>();}
inline bool BVector3::z() const {return Swizzle<2>();}
inline const BVector2 BVector3::xx() const {return Swizzle<0, 0>();}
inline const BVector2 BVector3::xy() const {return Swizzle<0, 1>();}
inline const BVector2 BVector3::xz() const {return Swizzle<0, 2>();}
inline const BVector2 BVector3::yx() const {return Swizzle<1, 0>();}
inline const BVector2 BVector3::yy() const {return Swizzle<1, 1>();}
inline const BVector2 BVector3::yz() const {return Swizzle<1, 2>();}
inline const BVector2 BVector3::zx() const {return Swizzle<2, 0>();}
inline const BVector2 BVector3::zy() const {return Swizzle<2, 1>();}
inline const BVector2 BVector3::zz() const {return Swizzle<2, 2>();}
inline const BVector3 BVector3::xxx() const {return Swizzle<0, 0, 0>();}
inline const BVector3 BVector3::xxy() const {return Swizzle<0, 0, 1>();}
inline const BVector3 BVector3::xxz() const {return Swizzle<0, 0, 2>();}
inline const BVector3 BVector3::xyx() const {return Swizzle<0, 1, 0>();}
inline const BVector3 BVector3::xyy() const {return Swizzle<0, 1, 1>();}
inline const BVector3 BVector3::xyz() const {return Swizzle<0, 1, 2>();}
inline const BVector3 BVector3::xzx() const {return Swizzle<0, 2, 0>();}
inline const BVector3 BVector3::xzy() const {return Swizzle<0, 2, 1>();}
inline const BVector3 BVector3::xzz() const {return Swizzle<0, 2, 2>();}
inline const BVector3 BVector3::yxx() const {return Swizzle<1, 0, 0>();}
inline const BVector3 BVector3::yxy() const {return Swizzle<1, 0, 1>();}
inline const BVector3 BVector3::yxz() const {return Swizzle<1, 0, 2>();}
inline const BVector3 BVector3::yyx() const {return Swizzle<1, 1, 0>();}
inline const BVector3 BVector3::yyy() const {return Swizzle<1, 1, 1>();}
inline const BVector3 BVector3::yyz() const {return Swizzle<1, 1, 2>();}
inline const BVector3 BVector3::yzx() const {return Swizzle<1, 2, 0>();}
inline const BVector3 BVector3::yzy() const {return Swizzle<1, 2, 1>();}
inline const BVector3 BVector3::yzz() const {return Swizzle<1, 2, 2>();}
inline const BVector3 BVector3::zxx() const {return Swizzle<2, 0, 0>();}
inline const BVector3 BVector3::zxy() const {return Swizzle<2, 0, 1>();}
inline const BVector3 BVector3::zxz() const {return Swizzle<2, 0, 2>();}
inline const BVector3 BVector3::zyx() const {return Swizzle<2, 1, 0>();}
inline const BVector3 BVector3::zyy() const {return Swizzle<2, 1, 1>();}
inline const BVector3 BVector3::zyz() const {return Swizzle<2, 1, 2>();}
inline const BVector3 BVector3::zzx() const {return Swizzle<2, 2, 0>();}
inline const BVector3 BVector3::zzy() const {return Swizzle<2, 2, 1>();}
inline const BVector3 BVector3::zzz() const {return Swizzle<2, 2, 2>();}
inline const BVector4 BVector3::xxxx() const {return Swizzle<0, 0, 0, 0>();}
inline const BVector4 BVector3::xxxy() const {return Swizzle<0, 0, 0, 1>();}
inline const BVector4 BVector3::xxxz() const {return Swizzle<0, 0, 0, 2>();}
inline const BVector4 BVector3::xxyx() const {return Swizzle<0, 0, 1, 0>();}
inline const BVector4 BVector3::xxyy() const {return Swizzle<0, 0, 1, 1>();}
inline const BVector4 BVector3::xxyz() const {return Swizzle<0, 0, 1, 2>();}
inline const BVector4 BVector3::xxzx() const {return Swizzle<0, 0, 2, 0>();}
inline const BVector4 BVector3::xxzy() const {return Swizzle<0, 0, 2, 1>();}
inline const BVector4 BVector3::xxzz() const {return Swizzle<0, 0, 2, 2>();}
inline const BVector4 BVector3::xyxx() const {return Swizzle<0, 1, 0, 0>();}
inline const BVector4 BVector3::xyxy() const {return Swizzle<0, 1, 0, 1>();}
inline const BVector4 BVector3::xyxz() const {return Swizzle<0, 1, 0, 2>();}
inline const BVector4 BVector3::xyyx() const {return Swizzle<0, 1, 1, 0>();}
inline const BVector4 BVector3::xyyy() const {return Swizzle<0, 1, 1, 1>();}
inline const BVector4 BVector3::xyyz() const {return Swizzle<0, 1, 1, 2>();}
inline const BVector4 BVector3::xyzx() const {return Swizzle<0, 1, 2, 0>();}
inline const BVector4 BVector3::xyzy() const {return Swizzle<0, 1, 2, 1>();}
inline const BVector4 BVector3::xyzz() const {return Swizzle<0, 1, 2, 2>();}
inline const BVector4 BVector3::xzxx() const {return Swizzle<0, 2, 0, 0>();}
inline const BVector4 BVector3::xzxy() const {return Swizzle<0, 2, 0, 1>();}
inline const BVector4 BVector3::xzxz() const {return Swizzle<0, 2, 0, 2>();}
inline const BVector4 BVector3::xzyx() const {return Swizzle<0, 2, 1, 0>();}
inline const BVector4 BVector3::xzyy() const {return Swizzle<0, 2, 1, 1>();}
inline const BVector4 BVector3::xzyz() const {return Swizzle<0, 2, 1, 2>();}
inline const BVector4 BVector3::xzzx() const {return Swizzle<0, 2, 2, 0>();}
inline const BVector4 BVector3::xzzy() const {return Swizzle<0, 2, 2, 1>();}
inline const BVector4 BVector3::xzzz() const {return Swizzle<0, 2, 2, 2>();}
inline const BVector4 BVector3::yxxx() const {return Swizzle<1, 0, 0, 0>();}
inline const BVector4 BVector3::yxxy() const {return Swizzle<1, 0, 0, 1>();}
inline const BVector4 BVector3::yxxz() const {return Swizzle<1, 0, 0, 2>();}
inline const BVector4 BVector3::yxyx() const {return Swizzle<1, 0, 1, 0>();}
inline const BVector4 BVector3::yxyy() const {return Swizzle<1, 0, 1, 1>();}
inline const BVector4 BVector3::yxyz() const {return Swizzle<1, 0, 1, 2>();}
inline const BVector4 BVector3::yxzx() const {return Swizzle<1, 0, 2, 0>();}
inline const BVector4 BVector3::yxzy() const {return Swizzle<1, 0, 2, 1>();}
inline const BVector4 BVector3::yxzz() const {return Swizzle<1, 0, 2, 2>();}
inline const BVector4 BVector3::yyxx() const {return Swizzle<1, 1, 0, 0>();}
inline const BVector4 BVector3::yyxy() const {return Swizzle<1, 1, 0, 1>();}
inline const BVector4 BVector3::yyxz() const {return Swizzle<1, 1, 0, 2>();}
inline const BVector4 BVector3::yyyx() const {return Swizzle<1, 1, 1, 0>();}
inline const BVector4 BVector3::yyyy() const {return Swizzle<1, 1, 1, 1>();}
inline const BVector4 BVector3::yyyz() const {return Swizzle<1, 1, 1, 2>();}
inline const BVector4 BVector3::yyzx() const {return Swizzle<1, 1, 2, 0>();}
inline const BVector4 BVector3::yyzy() const {return Swizzle<1, 1, 2, 1>();}
inline const BVector4 BVector3::yyzz() const {return Swizzle<1, 1, 2, 2>();}
inline const BVector4 BVector3::yzxx() const {return Swizzle<1, 2, 0, 0>();}
inline const BVector4 BVector3::yzxy() const {return Swizzle<1, 2, 0, 1>();}
inline const BVector4 BVector3::yzxz() const {return Swizzle<1, 2, 0, 2>();}
inline const BVector4 BVector3::yzyx() const {return Swizzle<1, 2, 1, 0>();}
inline const BVector4 BVector3::yzyy() const {return Swizzle<1, 2, 1, 1>();}
inline const BVector4 BVector3::yzyz() const {return Swizzle<1, 2, 1, 2>();}
inline const BVector4 BVector3::yzzx() const {return Swizzle<1, 2, 2, 0>();}
inline const BVector4 BVector3::yzzy() const {return Swizzle<1, 2, 2, 1>();}
inline const BVector4 BVector3::yzzz() const {return Swizzle<1, 2, 2, 2>();}
inline const BVector4 BVector3::zxxx() const {return Swizzle<2, 0, 0, 0>();}
inline const BVector4 BVector3::zxxy() const {return Swizzle<2, 0, 0, 1>();}
inline const BVector4 BVector3::zxxz() const {return Swizzle<2, 0, 0, 2>();}
inline const BVector4 BVector3::zxyx() const {return Swizzle<2, 0, 1, 0>();}
inline const BVector4 BVector3::zxyy() const {return Swizzle<2, 0, 1, 1>();}
inline const BVector4 BVector3::zxyz() const {return Swizzle<2, 0, 1, 2>();}
inline const BVector4 BVector3::zxzx() const {return Swizzle<2, 0, 2, 0>();}
inline const BVector4 BVector3::zxzy() const {return Swizzle<2, 0, 2, 1>();}
inline const BVector4 BVector3::zxzz() const {return Swizzle<2, 0, 2, 2>();}
inline const BVector4 BVector3::zyxx() const {return Swizzle<2, 1, 0, 0>();}
inline const BVector4 BVector3::zyxy() const {return Swizzle<2, 1, 0, 1>();}
inline const BVector4 BVector3::zyxz() const {return Swizzle<2, 1, 0, 2>();}
inline const BVector4 BVector3::zyyx() const {return Swizzle<2, 1, 1, 0>();}
inline const BVector4 BVector3::zyyy() const {return Swizzle<2, 1, 1, 1>();}
inline const BVector4 BVector3::zyyz() const {return Swizzle<2, 1, 1, 2>();}
inline const BVector4 BVector3::zyzx() const {return Swizzle<2, 1, 2, 0>();}
inline const BVector4 BVector3::zyzy() const {return Swizzle<2, 1, 2, 1>();}
inline const BVector4 BVector3::zyzz() const {return Swizzle<2, 1, 2, 2>();}
inline const BVector4 BVector3::zzxx() const {return Swizzle<2, 2, 0, 0>();}
inline const BVector4 BVector3::zzxy() const {return Swizzle<2, 2, 0, 1>();}
inline const BVector4 BVector3::zzxz() const {return Swizzle<2, 2, 0, 2>();}
inline const BVector4 BVector3::zzyx() const {return Swizzle<2, 2, 1, 0>();}
inline const BVector4 BVector3::zzyy() const {return Swizzle<2, 2, 1, 1>();}
inline const BVector4 BVector3::zzyz() const {return Swizzle<2, 2, 1, 2>();}
inline const BVector4 BVector3::zzzx() const {return Swizzle<2, 2, 2, 0>();}
inline const BVector4 BVector3::zzzy() const {return Swizzle<2, 2, 2, 1>();}
inline const BVector4 BVector3::zzzz() const {return Swizzle<2, 2, 2, 2>();}

inline BVector4::BVector(bool x)
{
#ifdef __SSE__
	if (x)
		data = SSE_MASK(1, 1, 1, 1);
	else
		data = _mm_setzero_ps();
#else
	data[0] = x;
	data[1] = x;
	data[2] = x;
	data[3] = x;
#endif
}
inline BVector4::BVector(bool x, bool y, bool z, bool w)
{
#ifdef __SSE__
	data = SSE_MASK(x, y, z, w);
#else
	data[0] = x;
	data[1] = y;
	data[2] = z;
	data[3] = w;
#endif
}
inline BVector4::BVector(BVector2 vec, bool z, bool w)
{
#ifdef __SSE__
	__m128 tmp = _mm_and_ps(_mm_movelh_ps(vec, tmp), SSE_MASK(1, 1, 0, 0));
	data = _mm_or_ps(tmp, SSE_MASK(0, 0, z, w));
#else
	data[0] = vec[0];
	data[1] = vec[1];
	data[2] = z;
	data[3] = w;
#endif
}
inline BVector4::BVector(bool x, BVector2 vec, bool w)
{
#ifdef __SSE__
	__m128 tmp = _mm_and_ps(_mm_unpacklo_ps(vec, vec), SSE_MASK(0, 1, 1, 0));
	data = _mm_or_ps(tmp, SSE_MASK(x, 0, 0, w));
#else
	data[0] = x;
	data[1] = vec[0];
	data[2] = vec[1];
	data[3] = w;
#endif
}
inline BVector4::BVector(bool x, bool y, BVector2 vec)
{
#ifdef __SSE__
	data = _mm_movelh_ps(SSE_MASK(x, y, 0, 0), vec);
#else
	data[0] = x;
	data[1] = y;
	data[2] = vec[0];
	data[3] = vec[1];
#endif
}
inline BVector4::BVector(BVector2 vec1, BVector2 vec2)
{
#ifdef __SSE__
	data = _mm_movelh_ps(vec1, vec2);
#else
	data[0] = vec1[0];
	data[1] = vec1[1];
	data[2] = vec2[0];
	data[3] = vec2[1];
#endif
}
inline BVector4::BVector(BVector3 vec, bool w)
{
#ifdef __SSE__
	if (w)
		data = _mm_or_ps(vec, SSE_MASK(0, 0, 0, 1));
	else
		data = _mm_andnot_ps(vec, SSE_MASK(0, 0, 0, 1));
#else
	data[0] = vec[0];
	data[1] = vec[1];
	data[2] = vec[2];
	data[3] = w;
#endif
}
inline BVector4::BVector(bool x, BVector3 vec)
{
#ifdef __SSE__
	if (x)
		data = _mm_move_ss(vec.xxyz(), SSE_MASK(1, 1, 1, 1));
	else
		data = _mm_move_ss(vec.xxyz(), _mm_setzero_ps());
#else
	data[0] = x;
	data[1] = vec[0];
	data[2] = vec[1];
	data[3] = vec[2];
#endif
}

#ifdef __SSE__
inline BVector4::BVector(__m128 x)
{
	data = x;
}
inline BVector4& BVector4::operator=(__m128 x)
{
	data = x;
	return *this;
}
#endif

inline bool BVector4::x() const {return Swizzle<0>();}
inline bool BVector4::y() const {return Swizzle<1>();}
inline bool BVector4::z() const {return Swizzle<2>();}
inline bool BVector4::w() const {return Swizzle<3>();}
inline const BVector2 BVector4::xx() const {return Swizzle<0, 0>();}
inline const BVector2 BVector4::xy() const {return Swizzle<0, 1>();}
inline const BVector2 BVector4::xz() const {return Swizzle<0, 2>();}
inline const BVector2 BVector4::xw() const {return Swizzle<0, 3>();}
inline const BVector2 BVector4::yx() const {return Swizzle<1, 0>();}
inline const BVector2 BVector4::yy() const {return Swizzle<1, 1>();}
inline const BVector2 BVector4::yz() const {return Swizzle<1, 2>();}
inline const BVector2 BVector4::yw() const {return Swizzle<1, 3>();}
inline const BVector2 BVector4::zx() const {return Swizzle<2, 0>();}
inline const BVector2 BVector4::zy() const {return Swizzle<2, 1>();}
inline const BVector2 BVector4::zz() const {return Swizzle<2, 2>();}
inline const BVector2 BVector4::zw() const {return Swizzle<2, 3>();}
inline const BVector2 BVector4::wx() const {return Swizzle<3, 0>();}
inline const BVector2 BVector4::wy() const {return Swizzle<3, 1>();}
inline const BVector2 BVector4::wz() const {return Swizzle<3, 2>();}
inline const BVector2 BVector4::ww() const {return Swizzle<3, 3>();}
inline const BVector3 BVector4::xxx() const {return Swizzle<0, 0, 0>();}
inline const BVector3 BVector4::xxy() const {return Swizzle<0, 0, 1>();}
inline const BVector3 BVector4::xxz() const {return Swizzle<0, 0, 2>();}
inline const BVector3 BVector4::xxw() const {return Swizzle<0, 0, 3>();}
inline const BVector3 BVector4::xyx() const {return Swizzle<0, 1, 0>();}
inline const BVector3 BVector4::xyy() const {return Swizzle<0, 1, 1>();}
inline const BVector3 BVector4::xyz() const {return Swizzle<0, 1, 2>();}
inline const BVector3 BVector4::xyw() const {return Swizzle<0, 1, 3>();}
inline const BVector3 BVector4::xzx() const {return Swizzle<0, 2, 0>();}
inline const BVector3 BVector4::xzy() const {return Swizzle<0, 2, 1>();}
inline const BVector3 BVector4::xzz() const {return Swizzle<0, 2, 2>();}
inline const BVector3 BVector4::xzw() const {return Swizzle<0, 2, 3>();}
inline const BVector3 BVector4::xwx() const {return Swizzle<0, 3, 0>();}
inline const BVector3 BVector4::xwy() const {return Swizzle<0, 3, 1>();}
inline const BVector3 BVector4::xwz() const {return Swizzle<0, 3, 2>();}
inline const BVector3 BVector4::xww() const {return Swizzle<0, 3, 3>();}
inline const BVector3 BVector4::yxx() const {return Swizzle<1, 0, 0>();}
inline const BVector3 BVector4::yxy() const {return Swizzle<1, 0, 1>();}
inline const BVector3 BVector4::yxz() const {return Swizzle<1, 0, 2>();}
inline const BVector3 BVector4::yxw() const {return Swizzle<1, 0, 3>();}
inline const BVector3 BVector4::yyx() const {return Swizzle<1, 1, 0>();}
inline const BVector3 BVector4::yyy() const {return Swizzle<1, 1, 1>();}
inline const BVector3 BVector4::yyz() const {return Swizzle<1, 1, 2>();}
inline const BVector3 BVector4::yyw() const {return Swizzle<1, 1, 3>();}
inline const BVector3 BVector4::yzx() const {return Swizzle<1, 2, 0>();}
inline const BVector3 BVector4::yzy() const {return Swizzle<1, 2, 1>();}
inline const BVector3 BVector4::yzz() const {return Swizzle<1, 2, 2>();}
inline const BVector3 BVector4::yzw() const {return Swizzle<1, 2, 3>();}
inline const BVector3 BVector4::ywx() const {return Swizzle<1, 3, 0>();}
inline const BVector3 BVector4::ywy() const {return Swizzle<1, 3, 1>();}
inline const BVector3 BVector4::ywz() const {return Swizzle<1, 3, 2>();}
inline const BVector3 BVector4::yww() const {return Swizzle<1, 3, 3>();}
inline const BVector3 BVector4::zxx() const {return Swizzle<2, 0, 0>();}
inline const BVector3 BVector4::zxy() const {return Swizzle<2, 0, 1>();}
inline const BVector3 BVector4::zxz() const {return Swizzle<2, 0, 2>();}
inline const BVector3 BVector4::zxw() const {return Swizzle<2, 0, 3>();}
inline const BVector3 BVector4::zyx() const {return Swizzle<2, 1, 0>();}
inline const BVector3 BVector4::zyy() const {return Swizzle<2, 1, 1>();}
inline const BVector3 BVector4::zyz() const {return Swizzle<2, 1, 2>();}
inline const BVector3 BVector4::zyw() const {return Swizzle<2, 1, 3>();}
inline const BVector3 BVector4::zzx() const {return Swizzle<2, 2, 0>();}
inline const BVector3 BVector4::zzy() const {return Swizzle<2, 2, 1>();}
inline const BVector3 BVector4::zzz() const {return Swizzle<2, 2, 2>();}
inline const BVector3 BVector4::zzw() const {return Swizzle<2, 2, 3>();}
inline const BVector3 BVector4::zwx() const {return Swizzle<2, 3, 0>();}
inline const BVector3 BVector4::zwy() const {return Swizzle<2, 3, 1>();}
inline const BVector3 BVector4::zwz() const {return Swizzle<2, 3, 2>();}
inline const BVector3 BVector4::zww() const {return Swizzle<2, 3, 3>();}
inline const BVector3 BVector4::wxx() const {return Swizzle<3, 0, 0>();}
inline const BVector3 BVector4::wxy() const {return Swizzle<3, 0, 1>();}
inline const BVector3 BVector4::wxz() const {return Swizzle<3, 0, 2>();}
inline const BVector3 BVector4::wxw() const {return Swizzle<3, 0, 3>();}
inline const BVector3 BVector4::wyx() const {return Swizzle<3, 1, 0>();}
inline const BVector3 BVector4::wyy() const {return Swizzle<3, 1, 1>();}
inline const BVector3 BVector4::wyz() const {return Swizzle<3, 1, 2>();}
inline const BVector3 BVector4::wyw() const {return Swizzle<3, 1, 3>();}
inline const BVector3 BVector4::wzx() const {return Swizzle<3, 2, 0>();}
inline const BVector3 BVector4::wzy() const {return Swizzle<3, 2, 1>();}
inline const BVector3 BVector4::wzz() const {return Swizzle<3, 2, 2>();}
inline const BVector3 BVector4::wzw() const {return Swizzle<3, 2, 3>();}
inline const BVector3 BVector4::wwx() const {return Swizzle<3, 3, 0>();}
inline const BVector3 BVector4::wwy() const {return Swizzle<3, 3, 1>();}
inline const BVector3 BVector4::wwz() const {return Swizzle<3, 3, 2>();}
inline const BVector3 BVector4::www() const {return Swizzle<3, 3, 3>();}
inline const BVector4 BVector4::xxxx() const {return Swizzle<0, 0, 0, 0>();}
inline const BVector4 BVector4::xxxy() const {return Swizzle<0, 0, 0, 1>();}
inline const BVector4 BVector4::xxxz() const {return Swizzle<0, 0, 0, 2>();}
inline const BVector4 BVector4::xxxw() const {return Swizzle<0, 0, 0, 3>();}
inline const BVector4 BVector4::xxyx() const {return Swizzle<0, 0, 1, 0>();}
inline const BVector4 BVector4::xxyy() const {return Swizzle<0, 0, 1, 1>();}
inline const BVector4 BVector4::xxyz() const {return Swizzle<0, 0, 1, 2>();}
inline const BVector4 BVector4::xxyw() const {return Swizzle<0, 0, 1, 3>();}
inline const BVector4 BVector4::xxzx() const {return Swizzle<0, 0, 2, 0>();}
inline const BVector4 BVector4::xxzy() const {return Swizzle<0, 0, 2, 1>();}
inline const BVector4 BVector4::xxzz() const {return Swizzle<0, 0, 2, 2>();}
inline const BVector4 BVector4::xxzw() const {return Swizzle<0, 0, 2, 3>();}
inline const BVector4 BVector4::xxwx() const {return Swizzle<0, 0, 3, 0>();}
inline const BVector4 BVector4::xxwy() const {return Swizzle<0, 0, 3, 1>();}
inline const BVector4 BVector4::xxwz() const {return Swizzle<0, 0, 3, 2>();}
inline const BVector4 BVector4::xxww() const {return Swizzle<0, 0, 3, 3>();}
inline const BVector4 BVector4::xyxx() const {return Swizzle<0, 1, 0, 0>();}
inline const BVector4 BVector4::xyxy() const {return Swizzle<0, 1, 0, 1>();}
inline const BVector4 BVector4::xyxz() const {return Swizzle<0, 1, 0, 2>();}
inline const BVector4 BVector4::xyxw() const {return Swizzle<0, 1, 0, 3>();}
inline const BVector4 BVector4::xyyx() const {return Swizzle<0, 1, 1, 0>();}
inline const BVector4 BVector4::xyyy() const {return Swizzle<0, 1, 1, 1>();}
inline const BVector4 BVector4::xyyz() const {return Swizzle<0, 1, 1, 2>();}
inline const BVector4 BVector4::xyyw() const {return Swizzle<0, 1, 1, 3>();}
inline const BVector4 BVector4::xyzx() const {return Swizzle<0, 1, 2, 0>();}
inline const BVector4 BVector4::xyzy() const {return Swizzle<0, 1, 2, 1>();}
inline const BVector4 BVector4::xyzz() const {return Swizzle<0, 1, 2, 2>();}
inline const BVector4 BVector4::xyzw() const {return Swizzle<0, 1, 2, 3>();}
inline const BVector4 BVector4::xywx() const {return Swizzle<0, 1, 3, 0>();}
inline const BVector4 BVector4::xywy() const {return Swizzle<0, 1, 3, 1>();}
inline const BVector4 BVector4::xywz() const {return Swizzle<0, 1, 3, 2>();}
inline const BVector4 BVector4::xyww() const {return Swizzle<0, 1, 3, 3>();}
inline const BVector4 BVector4::xzxx() const {return Swizzle<0, 2, 0, 0>();}
inline const BVector4 BVector4::xzxy() const {return Swizzle<0, 2, 0, 1>();}
inline const BVector4 BVector4::xzxz() const {return Swizzle<0, 2, 0, 2>();}
inline const BVector4 BVector4::xzxw() const {return Swizzle<0, 2, 0, 3>();}
inline const BVector4 BVector4::xzyx() const {return Swizzle<0, 2, 1, 0>();}
inline const BVector4 BVector4::xzyy() const {return Swizzle<0, 2, 1, 1>();}
inline const BVector4 BVector4::xzyz() const {return Swizzle<0, 2, 1, 2>();}
inline const BVector4 BVector4::xzyw() const {return Swizzle<0, 2, 1, 3>();}
inline const BVector4 BVector4::xzzx() const {return Swizzle<0, 2, 2, 0>();}
inline const BVector4 BVector4::xzzy() const {return Swizzle<0, 2, 2, 1>();}
inline const BVector4 BVector4::xzzz() const {return Swizzle<0, 2, 2, 2>();}
inline const BVector4 BVector4::xzzw() const {return Swizzle<0, 2, 2, 3>();}
inline const BVector4 BVector4::xzwx() const {return Swizzle<0, 2, 3, 0>();}
inline const BVector4 BVector4::xzwy() const {return Swizzle<0, 2, 3, 1>();}
inline const BVector4 BVector4::xzwz() const {return Swizzle<0, 2, 3, 2>();}
inline const BVector4 BVector4::xzww() const {return Swizzle<0, 2, 3, 3>();}
inline const BVector4 BVector4::xwxx() const {return Swizzle<0, 3, 0, 0>();}
inline const BVector4 BVector4::xwxy() const {return Swizzle<0, 3, 0, 1>();}
inline const BVector4 BVector4::xwxz() const {return Swizzle<0, 3, 0, 2>();}
inline const BVector4 BVector4::xwxw() const {return Swizzle<0, 3, 0, 3>();}
inline const BVector4 BVector4::xwyx() const {return Swizzle<0, 3, 1, 0>();}
inline const BVector4 BVector4::xwyy() const {return Swizzle<0, 3, 1, 1>();}
inline const BVector4 BVector4::xwyz() const {return Swizzle<0, 3, 1, 2>();}
inline const BVector4 BVector4::xwyw() const {return Swizzle<0, 3, 1, 3>();}
inline const BVector4 BVector4::xwzx() const {return Swizzle<0, 3, 2, 0>();}
inline const BVector4 BVector4::xwzy() const {return Swizzle<0, 3, 2, 1>();}
inline const BVector4 BVector4::xwzz() const {return Swizzle<0, 3, 2, 2>();}
inline const BVector4 BVector4::xwzw() const {return Swizzle<0, 3, 2, 3>();}
inline const BVector4 BVector4::xwwx() const {return Swizzle<0, 3, 3, 0>();}
inline const BVector4 BVector4::xwwy() const {return Swizzle<0, 3, 3, 1>();}
inline const BVector4 BVector4::xwwz() const {return Swizzle<0, 3, 3, 2>();}
inline const BVector4 BVector4::xwww() const {return Swizzle<0, 3, 3, 3>();}
inline const BVector4 BVector4::yxxx() const {return Swizzle<1, 0, 0, 0>();}
inline const BVector4 BVector4::yxxy() const {return Swizzle<1, 0, 0, 1>();}
inline const BVector4 BVector4::yxxz() const {return Swizzle<1, 0, 0, 2>();}
inline const BVector4 BVector4::yxxw() const {return Swizzle<1, 0, 0, 3>();}
inline const BVector4 BVector4::yxyx() const {return Swizzle<1, 0, 1, 0>();}
inline const BVector4 BVector4::yxyy() const {return Swizzle<1, 0, 1, 1>();}
inline const BVector4 BVector4::yxyz() const {return Swizzle<1, 0, 1, 2>();}
inline const BVector4 BVector4::yxyw() const {return Swizzle<1, 0, 1, 3>();}
inline const BVector4 BVector4::yxzx() const {return Swizzle<1, 0, 2, 0>();}
inline const BVector4 BVector4::yxzy() const {return Swizzle<1, 0, 2, 1>();}
inline const BVector4 BVector4::yxzz() const {return Swizzle<1, 0, 2, 2>();}
inline const BVector4 BVector4::yxzw() const {return Swizzle<1, 0, 2, 3>();}
inline const BVector4 BVector4::yxwx() const {return Swizzle<1, 0, 3, 0>();}
inline const BVector4 BVector4::yxwy() const {return Swizzle<1, 0, 3, 1>();}
inline const BVector4 BVector4::yxwz() const {return Swizzle<1, 0, 3, 2>();}
inline const BVector4 BVector4::yxww() const {return Swizzle<1, 0, 3, 3>();}
inline const BVector4 BVector4::yyxx() const {return Swizzle<1, 1, 0, 0>();}
inline const BVector4 BVector4::yyxy() const {return Swizzle<1, 1, 0, 1>();}
inline const BVector4 BVector4::yyxz() const {return Swizzle<1, 1, 0, 2>();}
inline const BVector4 BVector4::yyxw() const {return Swizzle<1, 1, 0, 3>();}
inline const BVector4 BVector4::yyyx() const {return Swizzle<1, 1, 1, 0>();}
inline const BVector4 BVector4::yyyy() const {return Swizzle<1, 1, 1, 1>();}
inline const BVector4 BVector4::yyyz() const {return Swizzle<1, 1, 1, 2>();}
inline const BVector4 BVector4::yyyw() const {return Swizzle<1, 1, 1, 3>();}
inline const BVector4 BVector4::yyzx() const {return Swizzle<1, 1, 2, 0>();}
inline const BVector4 BVector4::yyzy() const {return Swizzle<1, 1, 2, 1>();}
inline const BVector4 BVector4::yyzz() const {return Swizzle<1, 1, 2, 2>();}
inline const BVector4 BVector4::yyzw() const {return Swizzle<1, 1, 2, 3>();}
inline const BVector4 BVector4::yywx() const {return Swizzle<1, 1, 3, 0>();}
inline const BVector4 BVector4::yywy() const {return Swizzle<1, 1, 3, 1>();}
inline const BVector4 BVector4::yywz() const {return Swizzle<1, 1, 3, 2>();}
inline const BVector4 BVector4::yyww() const {return Swizzle<1, 1, 3, 3>();}
inline const BVector4 BVector4::yzxx() const {return Swizzle<1, 2, 0, 0>();}
inline const BVector4 BVector4::yzxy() const {return Swizzle<1, 2, 0, 1>();}
inline const BVector4 BVector4::yzxz() const {return Swizzle<1, 2, 0, 2>();}
inline const BVector4 BVector4::yzxw() const {return Swizzle<1, 2, 0, 3>();}
inline const BVector4 BVector4::yzyx() const {return Swizzle<1, 2, 1, 0>();}
inline const BVector4 BVector4::yzyy() const {return Swizzle<1, 2, 1, 1>();}
inline const BVector4 BVector4::yzyz() const {return Swizzle<1, 2, 1, 2>();}
inline const BVector4 BVector4::yzyw() const {return Swizzle<1, 2, 1, 3>();}
inline const BVector4 BVector4::yzzx() const {return Swizzle<1, 2, 2, 0>();}
inline const BVector4 BVector4::yzzy() const {return Swizzle<1, 2, 2, 1>();}
inline const BVector4 BVector4::yzzz() const {return Swizzle<1, 2, 2, 2>();}
inline const BVector4 BVector4::yzzw() const {return Swizzle<1, 2, 2, 3>();}
inline const BVector4 BVector4::yzwx() const {return Swizzle<1, 2, 3, 0>();}
inline const BVector4 BVector4::yzwy() const {return Swizzle<1, 2, 3, 1>();}
inline const BVector4 BVector4::yzwz() const {return Swizzle<1, 2, 3, 2>();}
inline const BVector4 BVector4::yzww() const {return Swizzle<1, 2, 3, 3>();}
inline const BVector4 BVector4::ywxx() const {return Swizzle<1, 3, 0, 0>();}
inline const BVector4 BVector4::ywxy() const {return Swizzle<1, 3, 0, 1>();}
inline const BVector4 BVector4::ywxz() const {return Swizzle<1, 3, 0, 2>();}
inline const BVector4 BVector4::ywxw() const {return Swizzle<1, 3, 0, 3>();}
inline const BVector4 BVector4::ywyx() const {return Swizzle<1, 3, 1, 0>();}
inline const BVector4 BVector4::ywyy() const {return Swizzle<1, 3, 1, 1>();}
inline const BVector4 BVector4::ywyz() const {return Swizzle<1, 3, 1, 2>();}
inline const BVector4 BVector4::ywyw() const {return Swizzle<1, 3, 1, 3>();}
inline const BVector4 BVector4::ywzx() const {return Swizzle<1, 3, 2, 0>();}
inline const BVector4 BVector4::ywzy() const {return Swizzle<1, 3, 2, 1>();}
inline const BVector4 BVector4::ywzz() const {return Swizzle<1, 3, 2, 2>();}
inline const BVector4 BVector4::ywzw() const {return Swizzle<1, 3, 2, 3>();}
inline const BVector4 BVector4::ywwx() const {return Swizzle<1, 3, 3, 0>();}
inline const BVector4 BVector4::ywwy() const {return Swizzle<1, 3, 3, 1>();}
inline const BVector4 BVector4::ywwz() const {return Swizzle<1, 3, 3, 2>();}
inline const BVector4 BVector4::ywww() const {return Swizzle<1, 3, 3, 3>();}
inline const BVector4 BVector4::zxxx() const {return Swizzle<2, 0, 0, 0>();}
inline const BVector4 BVector4::zxxy() const {return Swizzle<2, 0, 0, 1>();}
inline const BVector4 BVector4::zxxz() const {return Swizzle<2, 0, 0, 2>();}
inline const BVector4 BVector4::zxxw() const {return Swizzle<2, 0, 0, 3>();}
inline const BVector4 BVector4::zxyx() const {return Swizzle<2, 0, 1, 0>();}
inline const BVector4 BVector4::zxyy() const {return Swizzle<2, 0, 1, 1>();}
inline const BVector4 BVector4::zxyz() const {return Swizzle<2, 0, 1, 2>();}
inline const BVector4 BVector4::zxyw() const {return Swizzle<2, 0, 1, 3>();}
inline const BVector4 BVector4::zxzx() const {return Swizzle<2, 0, 2, 0>();}
inline const BVector4 BVector4::zxzy() const {return Swizzle<2, 0, 2, 1>();}
inline const BVector4 BVector4::zxzz() const {return Swizzle<2, 0, 2, 2>();}
inline const BVector4 BVector4::zxzw() const {return Swizzle<2, 0, 2, 3>();}
inline const BVector4 BVector4::zxwx() const {return Swizzle<2, 0, 3, 0>();}
inline const BVector4 BVector4::zxwy() const {return Swizzle<2, 0, 3, 1>();}
inline const BVector4 BVector4::zxwz() const {return Swizzle<2, 0, 3, 2>();}
inline const BVector4 BVector4::zxww() const {return Swizzle<2, 0, 3, 3>();}
inline const BVector4 BVector4::zyxx() const {return Swizzle<2, 1, 0, 0>();}
inline const BVector4 BVector4::zyxy() const {return Swizzle<2, 1, 0, 1>();}
inline const BVector4 BVector4::zyxz() const {return Swizzle<2, 1, 0, 2>();}
inline const BVector4 BVector4::zyxw() const {return Swizzle<2, 1, 0, 3>();}
inline const BVector4 BVector4::zyyx() const {return Swizzle<2, 1, 1, 0>();}
inline const BVector4 BVector4::zyyy() const {return Swizzle<2, 1, 1, 1>();}
inline const BVector4 BVector4::zyyz() const {return Swizzle<2, 1, 1, 2>();}
inline const BVector4 BVector4::zyyw() const {return Swizzle<2, 1, 1, 3>();}
inline const BVector4 BVector4::zyzx() const {return Swizzle<2, 1, 2, 0>();}
inline const BVector4 BVector4::zyzy() const {return Swizzle<2, 1, 2, 1>();}
inline const BVector4 BVector4::zyzz() const {return Swizzle<2, 1, 2, 2>();}
inline const BVector4 BVector4::zyzw() const {return Swizzle<2, 1, 2, 3>();}
inline const BVector4 BVector4::zywx() const {return Swizzle<2, 1, 3, 0>();}
inline const BVector4 BVector4::zywy() const {return Swizzle<2, 1, 3, 1>();}
inline const BVector4 BVector4::zywz() const {return Swizzle<2, 1, 3, 2>();}
inline const BVector4 BVector4::zyww() const {return Swizzle<2, 1, 3, 3>();}
inline const BVector4 BVector4::zzxx() const {return Swizzle<2, 2, 0, 0>();}
inline const BVector4 BVector4::zzxy() const {return Swizzle<2, 2, 0, 1>();}
inline const BVector4 BVector4::zzxz() const {return Swizzle<2, 2, 0, 2>();}
inline const BVector4 BVector4::zzxw() const {return Swizzle<2, 2, 0, 3>();}
inline const BVector4 BVector4::zzyx() const {return Swizzle<2, 2, 1, 0>();}
inline const BVector4 BVector4::zzyy() const {return Swizzle<2, 2, 1, 1>();}
inline const BVector4 BVector4::zzyz() const {return Swizzle<2, 2, 1, 2>();}
inline const BVector4 BVector4::zzyw() const {return Swizzle<2, 2, 1, 3>();}
inline const BVector4 BVector4::zzzx() const {return Swizzle<2, 2, 2, 0>();}
inline const BVector4 BVector4::zzzy() const {return Swizzle<2, 2, 2, 1>();}
inline const BVector4 BVector4::zzzz() const {return Swizzle<2, 2, 2, 2>();}
inline const BVector4 BVector4::zzzw() const {return Swizzle<2, 2, 2, 3>();}
inline const BVector4 BVector4::zzwx() const {return Swizzle<2, 2, 3, 0>();}
inline const BVector4 BVector4::zzwy() const {return Swizzle<2, 2, 3, 1>();}
inline const BVector4 BVector4::zzwz() const {return Swizzle<2, 2, 3, 2>();}
inline const BVector4 BVector4::zzww() const {return Swizzle<2, 2, 3, 3>();}
inline const BVector4 BVector4::zwxx() const {return Swizzle<2, 3, 0, 0>();}
inline const BVector4 BVector4::zwxy() const {return Swizzle<2, 3, 0, 1>();}
inline const BVector4 BVector4::zwxz() const {return Swizzle<2, 3, 0, 2>();}
inline const BVector4 BVector4::zwxw() const {return Swizzle<2, 3, 0, 3>();}
inline const BVector4 BVector4::zwyx() const {return Swizzle<2, 3, 1, 0>();}
inline const BVector4 BVector4::zwyy() const {return Swizzle<2, 3, 1, 1>();}
inline const BVector4 BVector4::zwyz() const {return Swizzle<2, 3, 1, 2>();}
inline const BVector4 BVector4::zwyw() const {return Swizzle<2, 3, 1, 3>();}
inline const BVector4 BVector4::zwzx() const {return Swizzle<2, 3, 2, 0>();}
inline const BVector4 BVector4::zwzy() const {return Swizzle<2, 3, 2, 1>();}
inline const BVector4 BVector4::zwzz() const {return Swizzle<2, 3, 2, 2>();}
inline const BVector4 BVector4::zwzw() const {return Swizzle<2, 3, 2, 3>();}
inline const BVector4 BVector4::zwwx() const {return Swizzle<2, 3, 3, 0>();}
inline const BVector4 BVector4::zwwy() const {return Swizzle<2, 3, 3, 1>();}
inline const BVector4 BVector4::zwwz() const {return Swizzle<2, 3, 3, 2>();}
inline const BVector4 BVector4::zwww() const {return Swizzle<2, 3, 3, 3>();}
inline const BVector4 BVector4::wxxx() const {return Swizzle<3, 0, 0, 0>();}
inline const BVector4 BVector4::wxxy() const {return Swizzle<3, 0, 0, 1>();}
inline const BVector4 BVector4::wxxz() const {return Swizzle<3, 0, 0, 2>();}
inline const BVector4 BVector4::wxxw() const {return Swizzle<3, 0, 0, 3>();}
inline const BVector4 BVector4::wxyx() const {return Swizzle<3, 0, 1, 0>();}
inline const BVector4 BVector4::wxyy() const {return Swizzle<3, 0, 1, 1>();}
inline const BVector4 BVector4::wxyz() const {return Swizzle<3, 0, 1, 2>();}
inline const BVector4 BVector4::wxyw() const {return Swizzle<3, 0, 1, 3>();}
inline const BVector4 BVector4::wxzx() const {return Swizzle<3, 0, 2, 0>();}
inline const BVector4 BVector4::wxzy() const {return Swizzle<3, 0, 2, 1>();}
inline const BVector4 BVector4::wxzz() const {return Swizzle<3, 0, 2, 2>();}
inline const BVector4 BVector4::wxzw() const {return Swizzle<3, 0, 2, 3>();}
inline const BVector4 BVector4::wxwx() const {return Swizzle<3, 0, 3, 0>();}
inline const BVector4 BVector4::wxwy() const {return Swizzle<3, 0, 3, 1>();}
inline const BVector4 BVector4::wxwz() const {return Swizzle<3, 0, 3, 2>();}
inline const BVector4 BVector4::wxww() const {return Swizzle<3, 0, 3, 3>();}
inline const BVector4 BVector4::wyxx() const {return Swizzle<3, 1, 0, 0>();}
inline const BVector4 BVector4::wyxy() const {return Swizzle<3, 1, 0, 1>();}
inline const BVector4 BVector4::wyxz() const {return Swizzle<3, 1, 0, 2>();}
inline const BVector4 BVector4::wyxw() const {return Swizzle<3, 1, 0, 3>();}
inline const BVector4 BVector4::wyyx() const {return Swizzle<3, 1, 1, 0>();}
inline const BVector4 BVector4::wyyy() const {return Swizzle<3, 1, 1, 1>();}
inline const BVector4 BVector4::wyyz() const {return Swizzle<3, 1, 1, 2>();}
inline const BVector4 BVector4::wyyw() const {return Swizzle<3, 1, 1, 3>();}
inline const BVector4 BVector4::wyzx() const {return Swizzle<3, 1, 2, 0>();}
inline const BVector4 BVector4::wyzy() const {return Swizzle<3, 1, 2, 1>();}
inline const BVector4 BVector4::wyzz() const {return Swizzle<3, 1, 2, 2>();}
inline const BVector4 BVector4::wyzw() const {return Swizzle<3, 1, 2, 3>();}
inline const BVector4 BVector4::wywx() const {return Swizzle<3, 1, 3, 0>();}
inline const BVector4 BVector4::wywy() const {return Swizzle<3, 1, 3, 1>();}
inline const BVector4 BVector4::wywz() const {return Swizzle<3, 1, 3, 2>();}
inline const BVector4 BVector4::wyww() const {return Swizzle<3, 1, 3, 3>();}
inline const BVector4 BVector4::wzxx() const {return Swizzle<3, 2, 0, 0>();}
inline const BVector4 BVector4::wzxy() const {return Swizzle<3, 2, 0, 1>();}
inline const BVector4 BVector4::wzxz() const {return Swizzle<3, 2, 0, 2>();}
inline const BVector4 BVector4::wzxw() const {return Swizzle<3, 2, 0, 3>();}
inline const BVector4 BVector4::wzyx() const {return Swizzle<3, 2, 1, 0>();}
inline const BVector4 BVector4::wzyy() const {return Swizzle<3, 2, 1, 1>();}
inline const BVector4 BVector4::wzyz() const {return Swizzle<3, 2, 1, 2>();}
inline const BVector4 BVector4::wzyw() const {return Swizzle<3, 2, 1, 3>();}
inline const BVector4 BVector4::wzzx() const {return Swizzle<3, 2, 2, 0>();}
inline const BVector4 BVector4::wzzy() const {return Swizzle<3, 2, 2, 1>();}
inline const BVector4 BVector4::wzzz() const {return Swizzle<3, 2, 2, 2>();}
inline const BVector4 BVector4::wzzw() const {return Swizzle<3, 2, 2, 3>();}
inline const BVector4 BVector4::wzwx() const {return Swizzle<3, 2, 3, 0>();}
inline const BVector4 BVector4::wzwy() const {return Swizzle<3, 2, 3, 1>();}
inline const BVector4 BVector4::wzwz() const {return Swizzle<3, 2, 3, 2>();}
inline const BVector4 BVector4::wzww() const {return Swizzle<3, 2, 3, 3>();}
inline const BVector4 BVector4::wwxx() const {return Swizzle<3, 3, 0, 0>();}
inline const BVector4 BVector4::wwxy() const {return Swizzle<3, 3, 0, 1>();}
inline const BVector4 BVector4::wwxz() const {return Swizzle<3, 3, 0, 2>();}
inline const BVector4 BVector4::wwxw() const {return Swizzle<3, 3, 0, 3>();}
inline const BVector4 BVector4::wwyx() const {return Swizzle<3, 3, 1, 0>();}
inline const BVector4 BVector4::wwyy() const {return Swizzle<3, 3, 1, 1>();}
inline const BVector4 BVector4::wwyz() const {return Swizzle<3, 3, 1, 2>();}
inline const BVector4 BVector4::wwyw() const {return Swizzle<3, 3, 1, 3>();}
inline const BVector4 BVector4::wwzx() const {return Swizzle<3, 3, 2, 0>();}
inline const BVector4 BVector4::wwzy() const {return Swizzle<3, 3, 2, 1>();}
inline const BVector4 BVector4::wwzz() const {return Swizzle<3, 3, 2, 2>();}
inline const BVector4 BVector4::wwzw() const {return Swizzle<3, 3, 2, 3>();}
inline const BVector4 BVector4::wwwx() const {return Swizzle<3, 3, 3, 0>();}
inline const BVector4 BVector4::wwwy() const {return Swizzle<3, 3, 3, 1>();}
inline const BVector4 BVector4::wwwz() const {return Swizzle<3, 3, 3, 2>();}
inline const BVector4 BVector4::wwww() const {return Swizzle<3, 3, 3, 3>();}

template<int length> inline BVectorEq<length>::BVectorEq(BVector<length> x)
	: BVector<length>(x) {}

template<int length> inline BVectorEq<length>::operator bool() const
{
	return this->All();
}

template<int length> inline BVectorNe<length>::BVectorNe(BVector<length> x)
	: BVector<length>(x) {}

template<int length> inline BVectorNe<length>::operator bool() const
{
	return this->Any();
}

template<int length> inline const Vector<length> VectorMin(Vector<length> a, Vector<length> b)
{
#ifdef __SSE__
	return _mm_min_ps(a, b);
#else
	return (a < b).Mix(a, b);
#endif
}
template<int length> inline const Vector<length> VectorMax(Vector<length> a, Vector<length> b)
{
#ifdef __SSE__
	return _mm_max_ps(a, b);
#else
	return (a > b).Mix(a, b);
#endif
}

template<int length> inline float DistanceSq(Vector<length> a, Vector<length> b)
{
	return (b - a).LengthSq();
}

template<int length> inline float Distance(Vector<length> a, Vector<length> b)
{
	return sqrt(DistanceSq(a, b));
}

template<int length> inline const Vector<length> VectorLerp(Vector<length> from, Vector<length> to, float frac)
{
	return from + (to - from) * frac;
}

inline float DotProduct(Vector2 a, Vector2 b)
{
#ifdef __SSE__
	return _mm_cvtss_f32(dot2_ps(a, b));
#else
	return a[0]*b[0] + a[1]*b[1];
#endif
}
inline float DotProduct(Vector3 a, Vector3 b)
{
#ifdef __SSE__
	return _mm_cvtss_f32(dot3_ps(a, b));
#else
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
#endif
}
inline float DotProduct(Vector4 a, Vector4 b)
{
#ifdef __SSE__
	return _mm_cvtss_f32(dot4_ps(a, b));
#else
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
#endif
}

inline const Vector3 CrossProduct(Vector3 a, Vector3 b)
{
	return a.yzx() * b.zxy() - a.zxy() * b.yzx();
}
