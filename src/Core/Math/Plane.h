//@@COPYRIGHT@@

// 3D plane, in the form ax + by + cz + d = 0

// Results of a cull operation
const int CULL_INSIDE = 1;
const int CULL_OUTSIDE = 2;
const int CULL_INTERSECT = 3;

class Plane: public Vector4 {
public:
	// Constructors
	Plane() = default;
	template<typename... T> Plane(T... x): Vector4(x...) {}

	// Get the normal and distance of the plane
	const Vector3 Normal() const
	{
		return xyz();
	}
	float Dist() const
	{
		return w();
	}

	// Normalize the plane's normal. All operations assume a normalized plane.
	const Plane Normalized() const
	{
		return *this * rsqrt(Normal().LengthSq());
	}
	void Normalize()
	{
		*this = Normalized();
	}

	// Find the distance of a point from the plane
	float PointDist(Vector3 point) const
	{
		return DotProduct(point, Normal()) + Dist();
	}

	// Cull a box
	int CullBox(Box box) const
	{
		BVector3 mask = Normal() < Vector3(0);
		Vector3 corner0 = mask.Mix(box.mins, box.maxs);
		Vector3 corner1 = mask.Mix(box.maxs, box.mins);

#ifdef __SSE__
		__m128 corner0x = _mm_mul_ps(corner0, *this);
		__m128 corner1x = _mm_mul_ps(corner1, *this);
		__m128 x = _mm_unpacklo_ps(corner0x, corner1x);
		__m128 y = _mm_movehl_ps(x, x);
		__m128 z = _mm_unpackhi_ps(corner0x, corner1x);
		__m128 dist = wwww();
		__m128 result = _mm_add_ps(_mm_add_ps(x, y), _mm_add_ps(z, dist));
		return (_mm_movemask_ps(result) ^ 1) & 3;
#else
		bool inside = DotProduct(corner0, Normal()) > -Dist();
		bool outside = DotProduct(corner1, Normal()) < -Dist();
		return outside + outside + inside;
#endif
	}
};

// Build a plane from a point and a normal
inline const Plane PlaneFromNormalPoint(Vector3 normal, Vector3 point)
{
	return Plane(normal, -DotProduct(point, normal));
}

// Build a plane from 3 points
inline const Plane PlaneFromPoints(Vector3 a, Vector3 b, Vector3 c)
{
	Vector3 d1 = b - a;
	Vector3 d2 = c - a;
	Vector3 normal = CrossProduct(d1, d2);
	normal.Normalize();
	return PlaneFromNormalPoint(normal, a);
}

// Find the intersection point of 3 planes. Planes must be normalized.
inline const Vector3 PlaneIntersection(Plane p1, Plane p2, Plane p3)
{
	Vector3 n1 = p1.Normal();
	Vector3 n2 = p2.Normal();
	Vector3 n3 = p3.Normal();

	Vector3 n1n2 = CrossProduct(n1, n2);
	Vector3 n2n3 = CrossProduct(n2, n3);
	Vector3 n3n1 = CrossProduct(n3, n1);

	Vector3 out = n2n3 * -p1.Dist();
	out -= n3n1 * p2.Dist();
	out -= n1n2 * p3.Dist();

	float denom = DotProduct(n1, n2n3);
	out /= denom;

	return out;
}

// Transform a plane with a matrix
inline const Plane Matrix::TransformPlane(Plane plane) const
{
	return Plane(Transform4(plane));
}

// Reflection matrix
inline const Matrix MatrixReflect(Plane plane)
{
	Matrix out;
	Vector3 n = plane.Normal();
	out[0] = Vector4(-2 * plane.xxx() * n, 0) + Vector4(1, 0, 0, 0);
	out[1] = Vector4(-2 * plane.yyy() * n, 0) + Vector4(0, 1, 0, 0);
	out[2] = Vector4(-2 * plane.zzz() * n, 0) + Vector4(0, 0, 1, 0);
	out[3] = Vector4(-2 * plane.www() * n, 1);
	return out;
}
