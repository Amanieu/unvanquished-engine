//@@COPYRIGHT@@

// Axis-aligned box

class Box {
public:
	// A box is defined by its minimum and maximum coordinates
	Vector3 mins, maxs;

	// Default constructor
	Box() = default;

	// Construct from an existing box
	Box(Vector3 mins_, Vector3 maxs_)
	{
		mins = mins_;
		maxs = maxs_;
	}

	// Check if another box intersects this one
	bool IntersectsBox(Box box) const
	{
		return ((box.mins <= maxs) & (box.maxs >= mins)).All();
	}

	// Check if a box is completely inside this one
	bool ContainsBox(Box box) const
	{
		return ((box.mins >= mins) & (box.maxs <= maxs)).All();
	}

	// Check if a point is inside the box
	bool ContainsPoint(Vector3 point) const
	{
		return ((point <= maxs) & (point >= mins)).All();
	}

	// Comparaison operators
	bool operator==(Box other) const
	{
		return mins == other.mins && maxs == other.maxs;
	}
	bool operator!=(Box other) const
	{
		return mins != other.mins || maxs != other.maxs;
	}
};

// Cull a box using a model-view-projection matrix. If using D3D, the z range
// is 0 to w, instead of -w to w with OpenGL.
template<bool D3DFrustum> inline int CullBox(Matrix mvp, Box box)
{
	// Extract corner points
#ifdef __SSE__
	Vector4 xXyY = _mm_unpacklo_ps(box.mins, box.maxs);
	Vector4 xXxX = _mm_movelh_ps(xXyY, xXyY);
	Vector4 yyYY = _mm_shuffle_ps(box.mins, box.maxs, _MM_SHUFFLE(1, 1, 1, 1));
#else
	Vector4 xXxX = Vector4(box.mins.x(), box.maxs.x(), box.mins.x(), box.maxs.x());
	Vector4 yyYY = Vector4(box.mins.yy(), box.maxs.yy());
#endif
	Vector4 zzzz = box.mins.zzzz();
	Vector4 ZZZZ = box.maxs.zzzz();

	// Transform corners with matrix
	Vector4 corners0[4];
	Vector4 corners1[4];
	corners0[0] = corners1[0] = xXxX * mvp[0].xxxx() + yyYY * mvp[1].xxxx() + mvp[3].xxxx();
	corners0[1] = corners1[1] = xXxX * mvp[0].yyyy() + yyYY * mvp[1].yyyy() + mvp[3].yyyy();
	corners0[2] = corners1[2] = xXxX * mvp[0].zzzz() + yyYY * mvp[1].zzzz() + mvp[3].zzzz();
	corners0[3] = corners1[3] = xXxX * mvp[0].wwww() + yyYY * mvp[1].wwww() + mvp[3].wwww();
	corners0[0] += zzzz * mvp[2].xxxx();
	corners0[1] += zzzz * mvp[2].yyyy();
	corners0[2] += zzzz * mvp[2].zzzz();
	corners0[3] += zzzz * mvp[2].wwww();
	corners1[0] += ZZZZ * mvp[2].xxxx();
	corners1[1] += ZZZZ * mvp[2].yyyy();
	corners1[2] += ZZZZ * mvp[2].zzzz();
	corners1[3] += ZZZZ * mvp[2].wwww();

	// Cull against planes
	BVector4 outside0, outside1;
#ifdef __SSE__
	// We use the fact that VectorMask only looks at the top bit of each element
	// to save 6 negations and 2 comparaisons for < 0.
	outside0 = BVector4(corners0[3] - corners0[0]);
	outside0 |= BVector4(corners0[0] + corners0[3]);
	outside0 |= BVector4(corners0[3] - corners0[1]);
	outside0 |= BVector4(corners0[1] + corners0[3]);
	outside0 |= BVector4(corners0[3] - corners0[2]);
	if (D3DFrustum)
		outside0 |= BVector4(corners0[2]);
	else
		outside0 |= BVector4(corners0[2] + corners0[3]);
	outside1 = BVector4(corners1[3] - corners1[0]);
	outside1 |= BVector4(corners1[0] + corners1[3]);
	outside1 |= BVector4(corners1[3] - corners1[1]);
	outside1 |= BVector4(corners1[1] + corners1[3]);
	outside1 |= BVector4(corners1[3] - corners1[2]);
	if (D3DFrustum)
		outside1 |= BVector4(corners1[2]);
	else
		outside1 |= BVector4(corners1[2] + corners1[3]);
#else
	outside0 = corners0[0] > corners0[3];
	outside0 |= corners0[0] < -corners0[3];
	outside0 |= corners0[1] > corners0[3];
	outside0 |= corners0[1] < -corners0[3];
	outside0 |= corners0[2] > corners0[3];
	if (D3DFrustum)
		outside0 |= corners0[2] < Vector4(0);
	else
		outside0 |= corners0[2] < -corners0[3];
	outside1 = corners1[0] > corners1[3];
	outside1 |= corners1[0] < -corners1[3];
	outside1 |= corners1[1] > corners1[3];
	outside1 |= corners1[1] < -corners1[3];
	outside1 |= corners1[2] > corners1[3];
	if (D3DFrustum)
		outside1 |= corners1[2] < Vector4(0);
	else
		outside1 |= corners1[2] < -corners1[3];
#endif

	// Merge results
	bool inside = (outside0 & outside1).NotAll();
	bool outside = (outside0 | outside1).Any();
	return outside + outside + inside;
}
