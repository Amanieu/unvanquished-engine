//@@COPYRIGHT@@

// Vector classes

// Forward declarations
template<int length> class BVector;
template<int length> class BVectorEq;
template<int length> class BVectorNe;
template<int length> class Vector;

// Typedefs for defined vector types
typedef Vector<2> Vector2;
typedef Vector<3> Vector3;
typedef Vector<4> Vector4;
typedef BVector<2> BVector2;
typedef BVector<3> BVector3;
typedef BVector<4> BVector4;

// Generic base class for float vectors
template<int length> class VectorBase {
	static_assert(length >= 2 && length <= 4, "Vector length must be one of (2, 3, 4).");

protected:
	// Instance data, usable by specialized Vector classes
#ifdef __SSE__
	__m128 data;
#else
	float data[length];
#endif

public:
#ifdef __SSE__
	// Implicit conversion to SSE vector type for use with intrinsics
	operator __m128() const;
#endif

	// Swizzle operations
	template<int x> float Swizzle() const;
	template<int x, int y> const Vector2 Swizzle() const;
	template<int x, int y, int z> const Vector3 Swizzle() const;
	template<int x, int y, int z, int w> const Vector4 Swizzle() const;

	// Subscript operator, only returns rvalue
	float operator[](int index) const;

	// Method to set one element of the vector
	void Set(int index, float value);

	// Comparaison operators, == and != can be implicitly converted to bool
	const BVectorEq<length> operator==(Vector<length> other) const;
	const BVectorNe<length> operator!=(Vector<length> other) const;
	const BVector<length> operator>=(Vector<length> other) const;
	const BVector<length> operator<=(Vector<length> other) const;
	const BVector<length> operator>(Vector<length> other) const;
	const BVector<length> operator<(Vector<length> other) const;

	// Arithmetic operators
	const Vector<length> operator-() const;
	const Vector<length> operator+(Vector<length> other) const;
	const Vector<length> operator-(Vector<length> other) const;
	const Vector<length> operator*(float x) const;
	const Vector<length> operator*(Vector<length> other) const;
	const Vector<length> operator/(float x) const;
	const Vector<length> operator/(Vector<length> other) const;

	// Compound arithmetic operators
	Vector<length>& operator+=(Vector<length> other);
	Vector<length>& operator-=(Vector<length> other);
	Vector<length>& operator*=(float x);
	Vector<length>& operator*=(Vector<length> other);
	Vector<length>& operator/=(float x);
	Vector<length>& operator/=(Vector<length> other);

	// Vector length
	float LengthSq() const;
	float Length() const;

	// Vector normalization
	const Vector<length> Normalized() const;
	void Normalize();

	// Flip the signs for the elements specified by a BVector
	void FlipSigns(BVector<length> signs);
	template<typename... T> void FlipSigns(T&&... signs);
};

// Scalar multiplication and division with the scalar first
template<int length> const Vector<length> operator*(float x, Vector<length> other);
template<int length> const Vector<length> operator/(float x, Vector<length> other);

// Generic base class for bool vectors
template<int length> class BVectorBase {
	static_assert(length >= 2 && length <= 4, "Vector length must be one of (2, 3, 4).");

protected:
	// Instance data, usable by specialized Vector classes
#ifdef __SSE__
	__m128 data;
#else
	bool data[length];
#endif

public:
#ifdef __SSE__
	// Implicit conversion to SSE vector type for use with intrinsics
	operator __m128() const;
#endif

	// Swizzle operations
	template<int x> bool Swizzle() const;
	template<int x, int y> const BVector2 Swizzle() const;
	template<int x, int y, int z> const BVector3 Swizzle() const;
	template<int x, int y, int z, int w> const BVector4 Swizzle() const;

	// Subscript operator, only returns rvalue
	bool operator[](int index) const;

	// Method to set one element of the vector
	void Set(int index, bool value);

	// Comparaison operators, == and != can be implicitly converted to bool
	const BVectorEq<length> operator==(BVector<length> other) const;
	const BVectorNe<length> operator!=(BVector<length> other) const;

	// Binary operators
	const BVector<length> operator~() const;
	const BVector<length> operator|(BVector<length> other) const;
	const BVector<length> operator&(BVector<length> other) const;
	const BVector<length> operator^(BVector<length> other) const;

	// Compound binary operators
	BVector<length>& operator|=(BVector<length> other);
	BVector<length>& operator&=(BVector<length> other);
	BVector<length>& operator^=(BVector<length> other);

	// Group testing operations
	bool Any() const;
	bool All() const;
	bool None() const;
	bool NotAll() const;

	// Mix 2 vectors together
	const Vector<length> Mix(Vector<length> valueTrue, Vector<length> valueFalse) const;

	// Mask a vector by setting all false values to 0
	const Vector<length> Mask(Vector<length> value) const;

	// Get an integer bit mask
	int Bits() const;
};

// Base vector types, these are never used, only specializations are used
template<int length> class Vector {
	static_assert(length >= 2 && length <= 4, "Vector length must be one of (2, 3, 4).");
};
template<int length> class BVector {
	static_assert(length >= 2 && length <= 4, "Vector length must be one of (2, 3, 4).");
};

// 2D float vector
template<> class Vector<2>: public VectorBase<2> {
public:
	// Constructors
	Vector() = default;
	explicit Vector(float x);
	Vector(float x, float y);
#ifdef __SSE__
	Vector(__m128 x);
#endif
	Vector(Vector3) = delete;
	Vector(Vector4) = delete;

	// Assignment operators
	Vector& operator=(const Vector2& other) = default;
#ifdef __SSE__
	Vector& operator=(__m128);
#endif
	Vector& operator=(Vector3) = delete;
	Vector& operator=(Vector4) = delete;

	// Swizzle shortcuts
	float x() const;
	float y() const;
	const Vector2 xx() const;
	const Vector2 xy() const;
	const Vector2 yx() const;
	const Vector2 yy() const;
	const Vector3 xxx() const;
	const Vector3 xxy() const;
	const Vector3 xyx() const;
	const Vector3 xyy() const;
	const Vector3 yxx() const;
	const Vector3 yxy() const;
	const Vector3 yyx() const;
	const Vector3 yyy() const;
	const Vector4 xxxx() const;
	const Vector4 xxxy() const;
	const Vector4 xxyx() const;
	const Vector4 xxyy() const;
	const Vector4 xyxx() const;
	const Vector4 xyxy() const;
	const Vector4 xyyx() const;
	const Vector4 xyyy() const;
	const Vector4 yxxx() const;
	const Vector4 yxxy() const;
	const Vector4 yxyx() const;
	const Vector4 yxyy() const;
	const Vector4 yyxx() const;
	const Vector4 yyxy() const;
	const Vector4 yyyx() const;
	const Vector4 yyyy() const;
};

// 3D float vector
template<> class Vector<3>: public VectorBase<3> {
public:
	// Constructors
	Vector() = default;
	explicit Vector(float x);
	Vector(float x, float y, float z);
	Vector(Vector2 vec, float z);
	Vector(float x, Vector2 vec);
#ifdef __SSE__
	Vector(__m128 x);
#endif
	Vector(Vector2) = delete;
	Vector(Vector4) = delete;

	// Assignment operators
	Vector& operator=(const Vector3& other) = default;
#ifdef __SSE__
	Vector& operator=(__m128);
#endif
	Vector& operator=(Vector2) = delete;
	Vector& operator=(Vector4) = delete;

	// Swizzle shortcuts
	float x() const;
	float y() const;
	float z() const;
	const Vector2 xx() const;
	const Vector2 xy() const;
	const Vector2 xz() const;
	const Vector2 yx() const;
	const Vector2 yy() const;
	const Vector2 yz() const;
	const Vector2 zx() const;
	const Vector2 zy() const;
	const Vector2 zz() const;
	const Vector3 xxx() const;
	const Vector3 xxy() const;
	const Vector3 xxz() const;
	const Vector3 xyx() const;
	const Vector3 xyy() const;
	const Vector3 xyz() const;
	const Vector3 xzx() const;
	const Vector3 xzy() const;
	const Vector3 xzz() const;
	const Vector3 yxx() const;
	const Vector3 yxy() const;
	const Vector3 yxz() const;
	const Vector3 yyx() const;
	const Vector3 yyy() const;
	const Vector3 yyz() const;
	const Vector3 yzx() const;
	const Vector3 yzy() const;
	const Vector3 yzz() const;
	const Vector3 zxx() const;
	const Vector3 zxy() const;
	const Vector3 zxz() const;
	const Vector3 zyx() const;
	const Vector3 zyy() const;
	const Vector3 zyz() const;
	const Vector3 zzx() const;
	const Vector3 zzy() const;
	const Vector3 zzz() const;
	const Vector4 xxxx() const;
	const Vector4 xxxy() const;
	const Vector4 xxxz() const;
	const Vector4 xxyx() const;
	const Vector4 xxyy() const;
	const Vector4 xxyz() const;
	const Vector4 xxzx() const;
	const Vector4 xxzy() const;
	const Vector4 xxzz() const;
	const Vector4 xyxx() const;
	const Vector4 xyxy() const;
	const Vector4 xyxz() const;
	const Vector4 xyyx() const;
	const Vector4 xyyy() const;
	const Vector4 xyyz() const;
	const Vector4 xyzx() const;
	const Vector4 xyzy() const;
	const Vector4 xyzz() const;
	const Vector4 xzxx() const;
	const Vector4 xzxy() const;
	const Vector4 xzxz() const;
	const Vector4 xzyx() const;
	const Vector4 xzyy() const;
	const Vector4 xzyz() const;
	const Vector4 xzzx() const;
	const Vector4 xzzy() const;
	const Vector4 xzzz() const;
	const Vector4 yxxx() const;
	const Vector4 yxxy() const;
	const Vector4 yxxz() const;
	const Vector4 yxyx() const;
	const Vector4 yxyy() const;
	const Vector4 yxyz() const;
	const Vector4 yxzx() const;
	const Vector4 yxzy() const;
	const Vector4 yxzz() const;
	const Vector4 yyxx() const;
	const Vector4 yyxy() const;
	const Vector4 yyxz() const;
	const Vector4 yyyx() const;
	const Vector4 yyyy() const;
	const Vector4 yyyz() const;
	const Vector4 yyzx() const;
	const Vector4 yyzy() const;
	const Vector4 yyzz() const;
	const Vector4 yzxx() const;
	const Vector4 yzxy() const;
	const Vector4 yzxz() const;
	const Vector4 yzyx() const;
	const Vector4 yzyy() const;
	const Vector4 yzyz() const;
	const Vector4 yzzx() const;
	const Vector4 yzzy() const;
	const Vector4 yzzz() const;
	const Vector4 zxxx() const;
	const Vector4 zxxy() const;
	const Vector4 zxxz() const;
	const Vector4 zxyx() const;
	const Vector4 zxyy() const;
	const Vector4 zxyz() const;
	const Vector4 zxzx() const;
	const Vector4 zxzy() const;
	const Vector4 zxzz() const;
	const Vector4 zyxx() const;
	const Vector4 zyxy() const;
	const Vector4 zyxz() const;
	const Vector4 zyyx() const;
	const Vector4 zyyy() const;
	const Vector4 zyyz() const;
	const Vector4 zyzx() const;
	const Vector4 zyzy() const;
	const Vector4 zyzz() const;
	const Vector4 zzxx() const;
	const Vector4 zzxy() const;
	const Vector4 zzxz() const;
	const Vector4 zzyx() const;
	const Vector4 zzyy() const;
	const Vector4 zzyz() const;
	const Vector4 zzzx() const;
	const Vector4 zzzy() const;
	const Vector4 zzzz() const;
};

// 4D float vector
template<> class Vector<4>: public VectorBase<4> {
public:
	// Constructors
	Vector() = default;
	explicit Vector(float x);
	Vector(float x, float y, float z, float w);
	Vector(Vector2 vec, float z, float w);
	Vector(float x, Vector2 vec, float w);
	Vector(float x, float y, Vector2 vec);
	Vector(Vector2 vec1, Vector2 vec2);
	Vector(Vector3 vec, float w);
	Vector(float x, Vector3 vec);
#ifdef __SSE__
	Vector(__m128 x);
#endif
	Vector(Vector2) = delete;
	Vector(Vector3) = delete;

	// Assignment operators
	Vector& operator=(const Vector4& other) = default;
#ifdef __SSE__
	Vector& operator=(__m128);
#endif
	Vector& operator=(Vector2) = delete;
	Vector& operator=(Vector3) = delete;

	// Swizzle shortcuts
	float x() const;
	float y() const;
	float z() const;
	float w() const;
	const Vector2 xx() const;
	const Vector2 xy() const;
	const Vector2 xz() const;
	const Vector2 xw() const;
	const Vector2 yx() const;
	const Vector2 yy() const;
	const Vector2 yz() const;
	const Vector2 yw() const;
	const Vector2 zx() const;
	const Vector2 zy() const;
	const Vector2 zz() const;
	const Vector2 zw() const;
	const Vector2 wx() const;
	const Vector2 wy() const;
	const Vector2 wz() const;
	const Vector2 ww() const;
	const Vector3 xxx() const;
	const Vector3 xxy() const;
	const Vector3 xxz() const;
	const Vector3 xxw() const;
	const Vector3 xyx() const;
	const Vector3 xyy() const;
	const Vector3 xyz() const;
	const Vector3 xyw() const;
	const Vector3 xzx() const;
	const Vector3 xzy() const;
	const Vector3 xzz() const;
	const Vector3 xzw() const;
	const Vector3 xwx() const;
	const Vector3 xwy() const;
	const Vector3 xwz() const;
	const Vector3 xww() const;
	const Vector3 yxx() const;
	const Vector3 yxy() const;
	const Vector3 yxz() const;
	const Vector3 yxw() const;
	const Vector3 yyx() const;
	const Vector3 yyy() const;
	const Vector3 yyz() const;
	const Vector3 yyw() const;
	const Vector3 yzx() const;
	const Vector3 yzy() const;
	const Vector3 yzz() const;
	const Vector3 yzw() const;
	const Vector3 ywx() const;
	const Vector3 ywy() const;
	const Vector3 ywz() const;
	const Vector3 yww() const;
	const Vector3 zxx() const;
	const Vector3 zxy() const;
	const Vector3 zxz() const;
	const Vector3 zxw() const;
	const Vector3 zyx() const;
	const Vector3 zyy() const;
	const Vector3 zyz() const;
	const Vector3 zyw() const;
	const Vector3 zzx() const;
	const Vector3 zzy() const;
	const Vector3 zzz() const;
	const Vector3 zzw() const;
	const Vector3 zwx() const;
	const Vector3 zwy() const;
	const Vector3 zwz() const;
	const Vector3 zww() const;
	const Vector3 wxx() const;
	const Vector3 wxy() const;
	const Vector3 wxz() const;
	const Vector3 wxw() const;
	const Vector3 wyx() const;
	const Vector3 wyy() const;
	const Vector3 wyz() const;
	const Vector3 wyw() const;
	const Vector3 wzx() const;
	const Vector3 wzy() const;
	const Vector3 wzz() const;
	const Vector3 wzw() const;
	const Vector3 wwx() const;
	const Vector3 wwy() const;
	const Vector3 wwz() const;
	const Vector3 www() const;
	const Vector4 xxxx() const;
	const Vector4 xxxy() const;
	const Vector4 xxxz() const;
	const Vector4 xxxw() const;
	const Vector4 xxyx() const;
	const Vector4 xxyy() const;
	const Vector4 xxyz() const;
	const Vector4 xxyw() const;
	const Vector4 xxzx() const;
	const Vector4 xxzy() const;
	const Vector4 xxzz() const;
	const Vector4 xxzw() const;
	const Vector4 xxwx() const;
	const Vector4 xxwy() const;
	const Vector4 xxwz() const;
	const Vector4 xxww() const;
	const Vector4 xyxx() const;
	const Vector4 xyxy() const;
	const Vector4 xyxz() const;
	const Vector4 xyxw() const;
	const Vector4 xyyx() const;
	const Vector4 xyyy() const;
	const Vector4 xyyz() const;
	const Vector4 xyyw() const;
	const Vector4 xyzx() const;
	const Vector4 xyzy() const;
	const Vector4 xyzz() const;
	const Vector4 xyzw() const;
	const Vector4 xywx() const;
	const Vector4 xywy() const;
	const Vector4 xywz() const;
	const Vector4 xyww() const;
	const Vector4 xzxx() const;
	const Vector4 xzxy() const;
	const Vector4 xzxz() const;
	const Vector4 xzxw() const;
	const Vector4 xzyx() const;
	const Vector4 xzyy() const;
	const Vector4 xzyz() const;
	const Vector4 xzyw() const;
	const Vector4 xzzx() const;
	const Vector4 xzzy() const;
	const Vector4 xzzz() const;
	const Vector4 xzzw() const;
	const Vector4 xzwx() const;
	const Vector4 xzwy() const;
	const Vector4 xzwz() const;
	const Vector4 xzww() const;
	const Vector4 xwxx() const;
	const Vector4 xwxy() const;
	const Vector4 xwxz() const;
	const Vector4 xwxw() const;
	const Vector4 xwyx() const;
	const Vector4 xwyy() const;
	const Vector4 xwyz() const;
	const Vector4 xwyw() const;
	const Vector4 xwzx() const;
	const Vector4 xwzy() const;
	const Vector4 xwzz() const;
	const Vector4 xwzw() const;
	const Vector4 xwwx() const;
	const Vector4 xwwy() const;
	const Vector4 xwwz() const;
	const Vector4 xwww() const;
	const Vector4 yxxx() const;
	const Vector4 yxxy() const;
	const Vector4 yxxz() const;
	const Vector4 yxxw() const;
	const Vector4 yxyx() const;
	const Vector4 yxyy() const;
	const Vector4 yxyz() const;
	const Vector4 yxyw() const;
	const Vector4 yxzx() const;
	const Vector4 yxzy() const;
	const Vector4 yxzz() const;
	const Vector4 yxzw() const;
	const Vector4 yxwx() const;
	const Vector4 yxwy() const;
	const Vector4 yxwz() const;
	const Vector4 yxww() const;
	const Vector4 yyxx() const;
	const Vector4 yyxy() const;
	const Vector4 yyxz() const;
	const Vector4 yyxw() const;
	const Vector4 yyyx() const;
	const Vector4 yyyy() const;
	const Vector4 yyyz() const;
	const Vector4 yyyw() const;
	const Vector4 yyzx() const;
	const Vector4 yyzy() const;
	const Vector4 yyzz() const;
	const Vector4 yyzw() const;
	const Vector4 yywx() const;
	const Vector4 yywy() const;
	const Vector4 yywz() const;
	const Vector4 yyww() const;
	const Vector4 yzxx() const;
	const Vector4 yzxy() const;
	const Vector4 yzxz() const;
	const Vector4 yzxw() const;
	const Vector4 yzyx() const;
	const Vector4 yzyy() const;
	const Vector4 yzyz() const;
	const Vector4 yzyw() const;
	const Vector4 yzzx() const;
	const Vector4 yzzy() const;
	const Vector4 yzzz() const;
	const Vector4 yzzw() const;
	const Vector4 yzwx() const;
	const Vector4 yzwy() const;
	const Vector4 yzwz() const;
	const Vector4 yzww() const;
	const Vector4 ywxx() const;
	const Vector4 ywxy() const;
	const Vector4 ywxz() const;
	const Vector4 ywxw() const;
	const Vector4 ywyx() const;
	const Vector4 ywyy() const;
	const Vector4 ywyz() const;
	const Vector4 ywyw() const;
	const Vector4 ywzx() const;
	const Vector4 ywzy() const;
	const Vector4 ywzz() const;
	const Vector4 ywzw() const;
	const Vector4 ywwx() const;
	const Vector4 ywwy() const;
	const Vector4 ywwz() const;
	const Vector4 ywww() const;
	const Vector4 zxxx() const;
	const Vector4 zxxy() const;
	const Vector4 zxxz() const;
	const Vector4 zxxw() const;
	const Vector4 zxyx() const;
	const Vector4 zxyy() const;
	const Vector4 zxyz() const;
	const Vector4 zxyw() const;
	const Vector4 zxzx() const;
	const Vector4 zxzy() const;
	const Vector4 zxzz() const;
	const Vector4 zxzw() const;
	const Vector4 zxwx() const;
	const Vector4 zxwy() const;
	const Vector4 zxwz() const;
	const Vector4 zxww() const;
	const Vector4 zyxx() const;
	const Vector4 zyxy() const;
	const Vector4 zyxz() const;
	const Vector4 zyxw() const;
	const Vector4 zyyx() const;
	const Vector4 zyyy() const;
	const Vector4 zyyz() const;
	const Vector4 zyyw() const;
	const Vector4 zyzx() const;
	const Vector4 zyzy() const;
	const Vector4 zyzz() const;
	const Vector4 zyzw() const;
	const Vector4 zywx() const;
	const Vector4 zywy() const;
	const Vector4 zywz() const;
	const Vector4 zyww() const;
	const Vector4 zzxx() const;
	const Vector4 zzxy() const;
	const Vector4 zzxz() const;
	const Vector4 zzxw() const;
	const Vector4 zzyx() const;
	const Vector4 zzyy() const;
	const Vector4 zzyz() const;
	const Vector4 zzyw() const;
	const Vector4 zzzx() const;
	const Vector4 zzzy() const;
	const Vector4 zzzz() const;
	const Vector4 zzzw() const;
	const Vector4 zzwx() const;
	const Vector4 zzwy() const;
	const Vector4 zzwz() const;
	const Vector4 zzww() const;
	const Vector4 zwxx() const;
	const Vector4 zwxy() const;
	const Vector4 zwxz() const;
	const Vector4 zwxw() const;
	const Vector4 zwyx() const;
	const Vector4 zwyy() const;
	const Vector4 zwyz() const;
	const Vector4 zwyw() const;
	const Vector4 zwzx() const;
	const Vector4 zwzy() const;
	const Vector4 zwzz() const;
	const Vector4 zwzw() const;
	const Vector4 zwwx() const;
	const Vector4 zwwy() const;
	const Vector4 zwwz() const;
	const Vector4 zwww() const;
	const Vector4 wxxx() const;
	const Vector4 wxxy() const;
	const Vector4 wxxz() const;
	const Vector4 wxxw() const;
	const Vector4 wxyx() const;
	const Vector4 wxyy() const;
	const Vector4 wxyz() const;
	const Vector4 wxyw() const;
	const Vector4 wxzx() const;
	const Vector4 wxzy() const;
	const Vector4 wxzz() const;
	const Vector4 wxzw() const;
	const Vector4 wxwx() const;
	const Vector4 wxwy() const;
	const Vector4 wxwz() const;
	const Vector4 wxww() const;
	const Vector4 wyxx() const;
	const Vector4 wyxy() const;
	const Vector4 wyxz() const;
	const Vector4 wyxw() const;
	const Vector4 wyyx() const;
	const Vector4 wyyy() const;
	const Vector4 wyyz() const;
	const Vector4 wyyw() const;
	const Vector4 wyzx() const;
	const Vector4 wyzy() const;
	const Vector4 wyzz() const;
	const Vector4 wyzw() const;
	const Vector4 wywx() const;
	const Vector4 wywy() const;
	const Vector4 wywz() const;
	const Vector4 wyww() const;
	const Vector4 wzxx() const;
	const Vector4 wzxy() const;
	const Vector4 wzxz() const;
	const Vector4 wzxw() const;
	const Vector4 wzyx() const;
	const Vector4 wzyy() const;
	const Vector4 wzyz() const;
	const Vector4 wzyw() const;
	const Vector4 wzzx() const;
	const Vector4 wzzy() const;
	const Vector4 wzzz() const;
	const Vector4 wzzw() const;
	const Vector4 wzwx() const;
	const Vector4 wzwy() const;
	const Vector4 wzwz() const;
	const Vector4 wzww() const;
	const Vector4 wwxx() const;
	const Vector4 wwxy() const;
	const Vector4 wwxz() const;
	const Vector4 wwxw() const;
	const Vector4 wwyx() const;
	const Vector4 wwyy() const;
	const Vector4 wwyz() const;
	const Vector4 wwyw() const;
	const Vector4 wwzx() const;
	const Vector4 wwzy() const;
	const Vector4 wwzz() const;
	const Vector4 wwzw() const;
	const Vector4 wwwx() const;
	const Vector4 wwwy() const;
	const Vector4 wwwz() const;
	const Vector4 wwww() const;
};

// 2D bool vector
template<> class BVector<2>: public BVectorBase<2> {
public:
	// Constructors
	BVector() = default;
	explicit BVector(bool x);
	BVector(bool x, bool y);
#ifdef __SSE__
	BVector(__m128 x);
#endif
	BVector(BVector3) = delete;
	BVector(BVector4) = delete;

	// Assignment operators
	BVector& operator=(const BVector2& other) = default;
#ifdef __SSE__
	BVector& operator=(__m128);
#endif
	BVector& operator=(BVector3) = delete;
	BVector& operator=(BVector4) = delete;

	// Swizzle shortcuts
	bool x() const;
	bool y() const;
	const BVector2 xx() const;
	const BVector2 xy() const;
	const BVector2 yx() const;
	const BVector2 yy() const;
	const BVector3 xxx() const;
	const BVector3 xxy() const;
	const BVector3 xyx() const;
	const BVector3 xyy() const;
	const BVector3 yxx() const;
	const BVector3 yxy() const;
	const BVector3 yyx() const;
	const BVector3 yyy() const;
	const BVector4 xxxx() const;
	const BVector4 xxxy() const;
	const BVector4 xxyx() const;
	const BVector4 xxyy() const;
	const BVector4 xyxx() const;
	const BVector4 xyxy() const;
	const BVector4 xyyx() const;
	const BVector4 xyyy() const;
	const BVector4 yxxx() const;
	const BVector4 yxxy() const;
	const BVector4 yxyx() const;
	const BVector4 yxyy() const;
	const BVector4 yyxx() const;
	const BVector4 yyxy() const;
	const BVector4 yyyx() const;
	const BVector4 yyyy() const;
};

// 3D bool vector
template<> class BVector<3>: public BVectorBase<3> {
public:
	// Constructors
	BVector() = default;
	explicit BVector(bool x);
	BVector(bool x, bool y, bool z);
	BVector(BVector2 vec, bool z);
	BVector(bool x, BVector2 vec);
#ifdef __SSE__
	BVector(__m128 x);
#endif
	BVector(BVector2) = delete;
	BVector(BVector4) = delete;

	// Assignment operators
	BVector& operator=(const BVector3& other) = default;
#ifdef __SSE__
	BVector& operator=(__m128);
#endif
	BVector& operator=(BVector2) = delete;
	BVector& operator=(BVector4) = delete;

	// Swizzle shortcuts
	bool x() const;
	bool y() const;
	bool z() const;
	const BVector2 xx() const;
	const BVector2 xy() const;
	const BVector2 xz() const;
	const BVector2 yx() const;
	const BVector2 yy() const;
	const BVector2 yz() const;
	const BVector2 zx() const;
	const BVector2 zy() const;
	const BVector2 zz() const;
	const BVector3 xxx() const;
	const BVector3 xxy() const;
	const BVector3 xxz() const;
	const BVector3 xyx() const;
	const BVector3 xyy() const;
	const BVector3 xyz() const;
	const BVector3 xzx() const;
	const BVector3 xzy() const;
	const BVector3 xzz() const;
	const BVector3 yxx() const;
	const BVector3 yxy() const;
	const BVector3 yxz() const;
	const BVector3 yyx() const;
	const BVector3 yyy() const;
	const BVector3 yyz() const;
	const BVector3 yzx() const;
	const BVector3 yzy() const;
	const BVector3 yzz() const;
	const BVector3 zxx() const;
	const BVector3 zxy() const;
	const BVector3 zxz() const;
	const BVector3 zyx() const;
	const BVector3 zyy() const;
	const BVector3 zyz() const;
	const BVector3 zzx() const;
	const BVector3 zzy() const;
	const BVector3 zzz() const;
	const BVector4 xxxx() const;
	const BVector4 xxxy() const;
	const BVector4 xxxz() const;
	const BVector4 xxyx() const;
	const BVector4 xxyy() const;
	const BVector4 xxyz() const;
	const BVector4 xxzx() const;
	const BVector4 xxzy() const;
	const BVector4 xxzz() const;
	const BVector4 xyxx() const;
	const BVector4 xyxy() const;
	const BVector4 xyxz() const;
	const BVector4 xyyx() const;
	const BVector4 xyyy() const;
	const BVector4 xyyz() const;
	const BVector4 xyzx() const;
	const BVector4 xyzy() const;
	const BVector4 xyzz() const;
	const BVector4 xzxx() const;
	const BVector4 xzxy() const;
	const BVector4 xzxz() const;
	const BVector4 xzyx() const;
	const BVector4 xzyy() const;
	const BVector4 xzyz() const;
	const BVector4 xzzx() const;
	const BVector4 xzzy() const;
	const BVector4 xzzz() const;
	const BVector4 yxxx() const;
	const BVector4 yxxy() const;
	const BVector4 yxxz() const;
	const BVector4 yxyx() const;
	const BVector4 yxyy() const;
	const BVector4 yxyz() const;
	const BVector4 yxzx() const;
	const BVector4 yxzy() const;
	const BVector4 yxzz() const;
	const BVector4 yyxx() const;
	const BVector4 yyxy() const;
	const BVector4 yyxz() const;
	const BVector4 yyyx() const;
	const BVector4 yyyy() const;
	const BVector4 yyyz() const;
	const BVector4 yyzx() const;
	const BVector4 yyzy() const;
	const BVector4 yyzz() const;
	const BVector4 yzxx() const;
	const BVector4 yzxy() const;
	const BVector4 yzxz() const;
	const BVector4 yzyx() const;
	const BVector4 yzyy() const;
	const BVector4 yzyz() const;
	const BVector4 yzzx() const;
	const BVector4 yzzy() const;
	const BVector4 yzzz() const;
	const BVector4 zxxx() const;
	const BVector4 zxxy() const;
	const BVector4 zxxz() const;
	const BVector4 zxyx() const;
	const BVector4 zxyy() const;
	const BVector4 zxyz() const;
	const BVector4 zxzx() const;
	const BVector4 zxzy() const;
	const BVector4 zxzz() const;
	const BVector4 zyxx() const;
	const BVector4 zyxy() const;
	const BVector4 zyxz() const;
	const BVector4 zyyx() const;
	const BVector4 zyyy() const;
	const BVector4 zyyz() const;
	const BVector4 zyzx() const;
	const BVector4 zyzy() const;
	const BVector4 zyzz() const;
	const BVector4 zzxx() const;
	const BVector4 zzxy() const;
	const BVector4 zzxz() const;
	const BVector4 zzyx() const;
	const BVector4 zzyy() const;
	const BVector4 zzyz() const;
	const BVector4 zzzx() const;
	const BVector4 zzzy() const;
	const BVector4 zzzz() const;
};

// 4D bool vector
template<> class BVector<4>: public BVectorBase<4> {
public:
	// Constructors
	BVector() = default;
	explicit BVector(bool x);
	BVector(bool x, bool y, bool z, bool w);
	BVector(BVector2 vec, bool z, bool w);
	BVector(bool x, BVector2 vec, bool w);
	BVector(bool x, bool y, BVector2 vec);
	BVector(BVector2 vec1, BVector2 vec2);
	BVector(BVector3 vec, bool w);
	BVector(bool x, BVector3 vec);
#ifdef __SSE__
	BVector(__m128 x);
#endif
	BVector(BVector2) = delete;
	BVector(BVector3) = delete;

	// Assignment operators
	BVector& operator=(const BVector4& other) = default;
#ifdef __SSE__
	BVector& operator=(__m128);
#endif
	BVector& operator=(BVector2) = delete;
	BVector& operator=(BVector3) = delete;

	// Swizzle shortcuts
	bool x() const;
	bool y() const;
	bool z() const;
	bool w() const;
	const BVector2 xx() const;
	const BVector2 xy() const;
	const BVector2 xz() const;
	const BVector2 xw() const;
	const BVector2 yx() const;
	const BVector2 yy() const;
	const BVector2 yz() const;
	const BVector2 yw() const;
	const BVector2 zx() const;
	const BVector2 zy() const;
	const BVector2 zz() const;
	const BVector2 zw() const;
	const BVector2 wx() const;
	const BVector2 wy() const;
	const BVector2 wz() const;
	const BVector2 ww() const;
	const BVector3 xxx() const;
	const BVector3 xxy() const;
	const BVector3 xxz() const;
	const BVector3 xxw() const;
	const BVector3 xyx() const;
	const BVector3 xyy() const;
	const BVector3 xyz() const;
	const BVector3 xyw() const;
	const BVector3 xzx() const;
	const BVector3 xzy() const;
	const BVector3 xzz() const;
	const BVector3 xzw() const;
	const BVector3 xwx() const;
	const BVector3 xwy() const;
	const BVector3 xwz() const;
	const BVector3 xww() const;
	const BVector3 yxx() const;
	const BVector3 yxy() const;
	const BVector3 yxz() const;
	const BVector3 yxw() const;
	const BVector3 yyx() const;
	const BVector3 yyy() const;
	const BVector3 yyz() const;
	const BVector3 yyw() const;
	const BVector3 yzx() const;
	const BVector3 yzy() const;
	const BVector3 yzz() const;
	const BVector3 yzw() const;
	const BVector3 ywx() const;
	const BVector3 ywy() const;
	const BVector3 ywz() const;
	const BVector3 yww() const;
	const BVector3 zxx() const;
	const BVector3 zxy() const;
	const BVector3 zxz() const;
	const BVector3 zxw() const;
	const BVector3 zyx() const;
	const BVector3 zyy() const;
	const BVector3 zyz() const;
	const BVector3 zyw() const;
	const BVector3 zzx() const;
	const BVector3 zzy() const;
	const BVector3 zzz() const;
	const BVector3 zzw() const;
	const BVector3 zwx() const;
	const BVector3 zwy() const;
	const BVector3 zwz() const;
	const BVector3 zww() const;
	const BVector3 wxx() const;
	const BVector3 wxy() const;
	const BVector3 wxz() const;
	const BVector3 wxw() const;
	const BVector3 wyx() const;
	const BVector3 wyy() const;
	const BVector3 wyz() const;
	const BVector3 wyw() const;
	const BVector3 wzx() const;
	const BVector3 wzy() const;
	const BVector3 wzz() const;
	const BVector3 wzw() const;
	const BVector3 wwx() const;
	const BVector3 wwy() const;
	const BVector3 wwz() const;
	const BVector3 www() const;
	const BVector4 xxxx() const;
	const BVector4 xxxy() const;
	const BVector4 xxxz() const;
	const BVector4 xxxw() const;
	const BVector4 xxyx() const;
	const BVector4 xxyy() const;
	const BVector4 xxyz() const;
	const BVector4 xxyw() const;
	const BVector4 xxzx() const;
	const BVector4 xxzy() const;
	const BVector4 xxzz() const;
	const BVector4 xxzw() const;
	const BVector4 xxwx() const;
	const BVector4 xxwy() const;
	const BVector4 xxwz() const;
	const BVector4 xxww() const;
	const BVector4 xyxx() const;
	const BVector4 xyxy() const;
	const BVector4 xyxz() const;
	const BVector4 xyxw() const;
	const BVector4 xyyx() const;
	const BVector4 xyyy() const;
	const BVector4 xyyz() const;
	const BVector4 xyyw() const;
	const BVector4 xyzx() const;
	const BVector4 xyzy() const;
	const BVector4 xyzz() const;
	const BVector4 xyzw() const;
	const BVector4 xywx() const;
	const BVector4 xywy() const;
	const BVector4 xywz() const;
	const BVector4 xyww() const;
	const BVector4 xzxx() const;
	const BVector4 xzxy() const;
	const BVector4 xzxz() const;
	const BVector4 xzxw() const;
	const BVector4 xzyx() const;
	const BVector4 xzyy() const;
	const BVector4 xzyz() const;
	const BVector4 xzyw() const;
	const BVector4 xzzx() const;
	const BVector4 xzzy() const;
	const BVector4 xzzz() const;
	const BVector4 xzzw() const;
	const BVector4 xzwx() const;
	const BVector4 xzwy() const;
	const BVector4 xzwz() const;
	const BVector4 xzww() const;
	const BVector4 xwxx() const;
	const BVector4 xwxy() const;
	const BVector4 xwxz() const;
	const BVector4 xwxw() const;
	const BVector4 xwyx() const;
	const BVector4 xwyy() const;
	const BVector4 xwyz() const;
	const BVector4 xwyw() const;
	const BVector4 xwzx() const;
	const BVector4 xwzy() const;
	const BVector4 xwzz() const;
	const BVector4 xwzw() const;
	const BVector4 xwwx() const;
	const BVector4 xwwy() const;
	const BVector4 xwwz() const;
	const BVector4 xwww() const;
	const BVector4 yxxx() const;
	const BVector4 yxxy() const;
	const BVector4 yxxz() const;
	const BVector4 yxxw() const;
	const BVector4 yxyx() const;
	const BVector4 yxyy() const;
	const BVector4 yxyz() const;
	const BVector4 yxyw() const;
	const BVector4 yxzx() const;
	const BVector4 yxzy() const;
	const BVector4 yxzz() const;
	const BVector4 yxzw() const;
	const BVector4 yxwx() const;
	const BVector4 yxwy() const;
	const BVector4 yxwz() const;
	const BVector4 yxww() const;
	const BVector4 yyxx() const;
	const BVector4 yyxy() const;
	const BVector4 yyxz() const;
	const BVector4 yyxw() const;
	const BVector4 yyyx() const;
	const BVector4 yyyy() const;
	const BVector4 yyyz() const;
	const BVector4 yyyw() const;
	const BVector4 yyzx() const;
	const BVector4 yyzy() const;
	const BVector4 yyzz() const;
	const BVector4 yyzw() const;
	const BVector4 yywx() const;
	const BVector4 yywy() const;
	const BVector4 yywz() const;
	const BVector4 yyww() const;
	const BVector4 yzxx() const;
	const BVector4 yzxy() const;
	const BVector4 yzxz() const;
	const BVector4 yzxw() const;
	const BVector4 yzyx() const;
	const BVector4 yzyy() const;
	const BVector4 yzyz() const;
	const BVector4 yzyw() const;
	const BVector4 yzzx() const;
	const BVector4 yzzy() const;
	const BVector4 yzzz() const;
	const BVector4 yzzw() const;
	const BVector4 yzwx() const;
	const BVector4 yzwy() const;
	const BVector4 yzwz() const;
	const BVector4 yzww() const;
	const BVector4 ywxx() const;
	const BVector4 ywxy() const;
	const BVector4 ywxz() const;
	const BVector4 ywxw() const;
	const BVector4 ywyx() const;
	const BVector4 ywyy() const;
	const BVector4 ywyz() const;
	const BVector4 ywyw() const;
	const BVector4 ywzx() const;
	const BVector4 ywzy() const;
	const BVector4 ywzz() const;
	const BVector4 ywzw() const;
	const BVector4 ywwx() const;
	const BVector4 ywwy() const;
	const BVector4 ywwz() const;
	const BVector4 ywww() const;
	const BVector4 zxxx() const;
	const BVector4 zxxy() const;
	const BVector4 zxxz() const;
	const BVector4 zxxw() const;
	const BVector4 zxyx() const;
	const BVector4 zxyy() const;
	const BVector4 zxyz() const;
	const BVector4 zxyw() const;
	const BVector4 zxzx() const;
	const BVector4 zxzy() const;
	const BVector4 zxzz() const;
	const BVector4 zxzw() const;
	const BVector4 zxwx() const;
	const BVector4 zxwy() const;
	const BVector4 zxwz() const;
	const BVector4 zxww() const;
	const BVector4 zyxx() const;
	const BVector4 zyxy() const;
	const BVector4 zyxz() const;
	const BVector4 zyxw() const;
	const BVector4 zyyx() const;
	const BVector4 zyyy() const;
	const BVector4 zyyz() const;
	const BVector4 zyyw() const;
	const BVector4 zyzx() const;
	const BVector4 zyzy() const;
	const BVector4 zyzz() const;
	const BVector4 zyzw() const;
	const BVector4 zywx() const;
	const BVector4 zywy() const;
	const BVector4 zywz() const;
	const BVector4 zyww() const;
	const BVector4 zzxx() const;
	const BVector4 zzxy() const;
	const BVector4 zzxz() const;
	const BVector4 zzxw() const;
	const BVector4 zzyx() const;
	const BVector4 zzyy() const;
	const BVector4 zzyz() const;
	const BVector4 zzyw() const;
	const BVector4 zzzx() const;
	const BVector4 zzzy() const;
	const BVector4 zzzz() const;
	const BVector4 zzzw() const;
	const BVector4 zzwx() const;
	const BVector4 zzwy() const;
	const BVector4 zzwz() const;
	const BVector4 zzww() const;
	const BVector4 zwxx() const;
	const BVector4 zwxy() const;
	const BVector4 zwxz() const;
	const BVector4 zwxw() const;
	const BVector4 zwyx() const;
	const BVector4 zwyy() const;
	const BVector4 zwyz() const;
	const BVector4 zwyw() const;
	const BVector4 zwzx() const;
	const BVector4 zwzy() const;
	const BVector4 zwzz() const;
	const BVector4 zwzw() const;
	const BVector4 zwwx() const;
	const BVector4 zwwy() const;
	const BVector4 zwwz() const;
	const BVector4 zwww() const;
	const BVector4 wxxx() const;
	const BVector4 wxxy() const;
	const BVector4 wxxz() const;
	const BVector4 wxxw() const;
	const BVector4 wxyx() const;
	const BVector4 wxyy() const;
	const BVector4 wxyz() const;
	const BVector4 wxyw() const;
	const BVector4 wxzx() const;
	const BVector4 wxzy() const;
	const BVector4 wxzz() const;
	const BVector4 wxzw() const;
	const BVector4 wxwx() const;
	const BVector4 wxwy() const;
	const BVector4 wxwz() const;
	const BVector4 wxww() const;
	const BVector4 wyxx() const;
	const BVector4 wyxy() const;
	const BVector4 wyxz() const;
	const BVector4 wyxw() const;
	const BVector4 wyyx() const;
	const BVector4 wyyy() const;
	const BVector4 wyyz() const;
	const BVector4 wyyw() const;
	const BVector4 wyzx() const;
	const BVector4 wyzy() const;
	const BVector4 wyzz() const;
	const BVector4 wyzw() const;
	const BVector4 wywx() const;
	const BVector4 wywy() const;
	const BVector4 wywz() const;
	const BVector4 wyww() const;
	const BVector4 wzxx() const;
	const BVector4 wzxy() const;
	const BVector4 wzxz() const;
	const BVector4 wzxw() const;
	const BVector4 wzyx() const;
	const BVector4 wzyy() const;
	const BVector4 wzyz() const;
	const BVector4 wzyw() const;
	const BVector4 wzzx() const;
	const BVector4 wzzy() const;
	const BVector4 wzzz() const;
	const BVector4 wzzw() const;
	const BVector4 wzwx() const;
	const BVector4 wzwy() const;
	const BVector4 wzwz() const;
	const BVector4 wzww() const;
	const BVector4 wwxx() const;
	const BVector4 wwxy() const;
	const BVector4 wwxz() const;
	const BVector4 wwxw() const;
	const BVector4 wwyx() const;
	const BVector4 wwyy() const;
	const BVector4 wwyz() const;
	const BVector4 wwyw() const;
	const BVector4 wwzx() const;
	const BVector4 wwzy() const;
	const BVector4 wwzz() const;
	const BVector4 wwzw() const;
	const BVector4 wwwx() const;
	const BVector4 wwwy() const;
	const BVector4 wwwz() const;
	const BVector4 wwww() const;
};

// BVector returned from an == comparaison, implicitly converts to bool
template<int length> class BVectorEq: public BVector<length> {
	static_assert(length >= 2 && length <= 4, "BVector length must be one of (2, 3, 4).");

private:
	// Constructor used by operator==
	BVectorEq(BVector<length> x);
	friend class VectorBase<length>;
	friend class BVectorBase<length>;

public:
	// Default constructor
	BVectorEq() = default;

	// Conversion to bool
	explicit operator bool() const;
};

// BVector returned from an != comparaison, implicitly converts to bool
template<int length> class BVectorNe: public BVector<length> {
	static_assert(length >= 2 && length <= 4, "BVector length must be one of (2, 3, 4).");

private:
	// Constructor used by operator!=
	BVectorNe(BVector<length> x);
	friend class VectorBase<length>;
	friend class BVectorBase<length>;

public:
	// Default constructor
	BVectorNe() = default;

	// Conversion to bool
	explicit operator bool() const;
};

// Minimum and maximum of 2 vectors
template<int length> const Vector<length> VectorMin(Vector<length> a, Vector<length> b);
template<int length> const Vector<length> VectorMax(Vector<length> a, Vector<length> b);

// Distance between 2 points
template<int length> float DistanceSq(Vector<length> a, Vector<length> b);
template<int length> float Distance(Vector<length> a, Vector<length> b);

// Vector linear interpolation
template<int length> const Vector<length> VectorLerp(Vector<length> from, Vector<length> to, float frac);

// Vector dot product
float DotProduct(Vector2 a, Vector2 b);
float DotProduct(Vector3 a, Vector3 b);
float DotProduct(Vector4 a, Vector4 b);

// 3D vector cross product
const Vector3 CrossProduct(Vector3 a, Vector3 b);
