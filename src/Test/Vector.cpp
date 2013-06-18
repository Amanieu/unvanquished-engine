//@@COPYRIGHT@@

// Unit tests for vector classes

TestSuite(VectorTest)

TestCase(SimpleConstructors)
{
	Vector2 v1(60);
	Vector2 v2(0, 1);
	Vector3 v3(1, 2, 3);
	Vector4 v4(5, 7, 9, 8);
	TestCheckEqual(v1[0], 60);
	TestCheckEqual(v1[1], 60);
	TestCheckEqual(v2[0], 0);
	TestCheckEqual(v2[1], 1);
	TestCheckEqual(v3[0], 1);
	TestCheckEqual(v3[1], 2);
	TestCheckEqual(v3[2], 3);
	TestCheckEqual(v4[0], 5);
	TestCheckEqual(v4[1], 7);
	TestCheckEqual(v4[2], 9);
	TestCheckEqual(v4[3], 8);
}

TestCase(AdvancedConstructors)
{
	Vector2 v2(0, 1);
	Vector3 v3(0, 1, 2);
	TestCheckEqual(Vector3(v2, 2), Vector3(0, 1, 2));
	TestCheckEqual(Vector3(2, v2), Vector3(2, 0, 1));
	TestCheckEqual(Vector4(v2, Vector2(2, 3)), Vector4(0, 1, 2, 3));
	TestCheckEqual(Vector4(v2, 2, 3), Vector4(0, 1, 2, 3));
	TestCheckEqual(Vector4(2, 3, v2), Vector4(2, 3, 0, 1));
	TestCheckEqual(Vector4(2, v2, 3), Vector4(2, 0, 1, 3));
	TestCheckEqual(Vector4(v3, 3), Vector4(0, 1, 2, 3));
	TestCheckEqual(Vector4(3, v3), Vector4(3, 0, 1, 2));
}

TestCase(Compare)
{
	Vector4 v1(5, 7, 9, 8);
	Vector4 v2(5, 7, 8, 8);
	BVector4 m = v1 == v2;
	TestCheck(m[0]);
	TestCheck(m[1]);
	TestCheck(!m[2]);
	TestCheck(m[3]);
	TestCheck(m.Any());
	TestCheck(!m.All());
	TestCheck(!m.None());
	TestCheck(m.NotAll());
	m = v1 != v2;
	TestCheck(!m[0]);
	TestCheck(!m[1]);
	TestCheck(m[2]);
	TestCheck(!m[3]);
	TestCheck(m.Any());
	TestCheck(!m.All());
	TestCheck(!m.None());
	TestCheck(m.NotAll());
	m = v1 > v2;
	TestCheck(!m[0]);
	TestCheck(!m[1]);
	TestCheck(m[2]);
	TestCheck(!m[3]);
	TestCheck(m.Any());
	TestCheck(!m.All());
	TestCheck(!m.None());
	TestCheck(m.NotAll());
	m = v1 < v2;
	TestCheck(!m[0]);
	TestCheck(!m[1]);
	TestCheck(!m[2]);
	TestCheck(!m[3]);
	TestCheck(!m.Any());
	TestCheck(!m.All());
	TestCheck(m.None());
	TestCheck(m.NotAll());
	TestCheck((~m).All());

	TestCheckEqual((v1 <= v2), ~(v1 > v2));
	TestCheckEqual((v1 < v2), ~(v1 >= v2));
	TestCheckEqual((v1 != v2), ~(v1 == v2));
}

TestCase(Mask)
{
	BVector2 v1(true, false);
	BVector2 v2(true, true);
	BVector2 v3(false, false);

	TestCheckEqual((v1 & v2), v1);
	TestCheckEqual((v1 & v3), v3);
	TestCheckEqual((v2 & v3), v3);
	TestCheckEqual((v1 | v2), v2);
	TestCheckEqual((v1 | v3), v1);
	TestCheckEqual((v2 | v3), v2);
	TestCheckEqual((v1 ^ v2), ~v1);
	TestCheckEqual((v1 ^ v3), v1);
	TestCheckEqual((v2 ^ v3), v2);

	Vector2 a1(0, 1);
	Vector2 a2(2, 3);
	TestCheckEqual(v1.Mask(a1), Vector2(0));
	TestCheckEqual(v1.Mask(a2), Vector2(2, 0));
	TestCheckEqual(v1.Mix(a1, a2), Vector2(0, 3));
	TestCheckEqual(v1.Mix(a2, a1), Vector2(2, 1));
}

TestCase(Operators)
{
	Vector2 v1(0, 1);
	Vector2 v2(2, 3);

	TestCheckEqual(v1 + v2, Vector2(2, 4));
	TestCheckEqual(v1 - v2, Vector2(-2, -2));
	TestCheckEqual(v1 * v2, Vector2(0, 3));
	TestCheckEqual(5 * v2, Vector2(10, 15));
	TestCheckEqual(v1 / v2, Vector2(0, 1.0f / 3));
	TestCheckEqual(-v2, Vector2(-2, -3));
}

TestCase(Swizzle)
{
	Vector4 v1(0, 1, 2, 3);

	TestCheckEqual(v1.x(), 0);
	TestCheckEqual(v1.y(), 1);
	TestCheckEqual(v1.z(), 2);
	TestCheckEqual(v1.w(), 3);
	TestCheckEqual(v1.xy(), Vector2(0, 1));
	TestCheckEqual(v1.xw(), Vector2(0, 3));
	TestCheckEqual(v1.wz(), Vector2(3, 2));
	TestCheckEqual(v1.yy(), Vector2(1, 1));
	TestCheckEqual(v1.xyz(), Vector3(0, 1, 2));
	TestCheckEqual(v1.wyz(), Vector3(3, 1, 2));
	TestCheckEqual(v1.xxy(), Vector3(0, 0, 1));
	TestCheckEqual(v1.zzz(), Vector3(2, 2, 2));
	TestCheckEqual(v1.xyzw(), v1);
	TestCheckEqual(v1.zwxy(), Vector4(2, 3, 0, 1));
	TestCheckEqual(v1.zywy(), Vector4(2, 1, 3, 1));
	TestCheckEqual(v1.xxxx(), Vector4(0, 0, 0, 0));
}
/* FIXME
TestCase(Sign)
{
	Vector3 v1(-3, 4, -1);
	Vector3 v2(1, 2, 3);

	TestCheckEqual(v1.Abs(), Vector3(3, 4, 1));
	Vector3 tmp = v2;
	tmp.CopySigns(v1);
	TestCheckEqual(tmp, Vector3(-1, 2, -3));
	tmp = v1;
	tmp.FlipSigns(0, 1, 1);
	TestCheckEqual(tmp, Vector3(-3, -4, 1));
	tmp = v1;
	tmp.ClearSigns(0, 1, 1);
	TestCheckEqual(tmp, Vector3(-3, 4, 1));
	tmp = v1;
	tmp.SetSigns(0, 1, 1);
	TestCheckEqual(tmp, Vector3(-3, -4, -1));
}

static inline void TestMathOps(float value)
{
	Vector2 v1(value);
	TestCheckClose(v1.Recip()[0], 1 / value, FLOAT_TOLERANCE);
	TestCheckClose(v1.Rsqrt()[0], rsqrt(value), FLOAT_TOLERANCE);
	TestCheckClose(v1.Sqrt()[0], sqrt(value), FLOAT_TOLERANCE);
}

TestCase(MathOps)
{
	TestMathOps(1);
	TestMathOps(2);
	TestMathOps(3);
	TestMathOps(4);
	TestMathOps(5);
	TestMathOps(6);
	TestMathOps(100);
	TestMathOps(0.01);
	TestMathOps(10000000);
	TestMathOps(2);
	TestMathOps(0.000001);
}*/

TestCase(VectorOps)
{
	Vector2 v2a(1, 2);
	Vector2 v2b(3, 4);
	Vector3 v3a(1, 2, 3);
	Vector3 v3b(4, 5, 6);
	Vector4 v4a(1, 2, 3, 4);
	Vector4 v4b(5, 6, 7, 8);

	TestCheckEqual(DotProduct(v2a, v2b), 11);
	TestCheckEqual(DotProduct(v3a, v3b), 32);
	TestCheckEqual(DotProduct(v4a, v4b), 70);

	TestCheckEqual(CrossProduct(v3a, v3b), Vector3(-3, 6, -3));
	TestCheckEqual(CrossProduct(v3b, v3a), Vector3(3, -6, 3));

	TestCheckEqual(DistanceSq(v2a, v2b), 8);
	TestCheckClose(Distance(v2a, v2b), sqrt(8), FLOAT_TOLERANCE);
	TestCheckEqual(v2b.Length(), 5);

	v4b.Normalize();
	TestCheckClose(v4b.Length(), 1, FLOAT_TOLERANCE);

	TestCheckEqual(VectorMax(Vector4(1, 5, 6, 1), Vector4(3, 8, 1, 1)), Vector4(3, 8, 6, 1));
	TestCheckEqual(VectorMin(Vector4(1, 5, 6, 1), Vector4(3, 8, 1, 1)), Vector4(1, 5, 1, 1));

	TestCheckEqual(VectorLerp(v2a, v2b, 0.5), Vector2(2, 3));
}

/* FIXME
void TestHalf(float value)
{
	int16_t half[4];
	Vector4 x(value);
	x.ToHalf(half);
	TestCheckEqual(half[0], FloatToHalf(value));
	TestCheckEqual(half[0], half[1]);
	TestCheckEqual(half[0], half[2]);
	TestCheckEqual(half[0], half[3]);
}

TestCase(Half)
{
	TestHalf(-1);
	TestHalf(-2);
	TestHalf(-3);
	TestHalf(0);
	TestHalf(1);
	TestHalf(2);
	TestHalf(3);
	TestHalf(99999999);
}*/

#ifdef __SSE__
#define TestSSEFunc(func, value) TestCheckClose(Vector2(func##_ps(Vector2(value)))[0], func(value), FLOAT_TOLERANCE);
#define TestAtan2(val1, val2) TestCheckClose(Vector2(atan2_ps(Vector2(val1), Vector2(val2)))[0], atan2(val1, val2), FLOAT_TOLERANCE);
void TestSSEMath(float value)
{
	TestSSEFunc(sin, value);
	TestSSEFunc(cos, value);
	TestSSEFunc(tan, value);
	TestSSEFunc(atan, value);
	Vector2 s, c;
	std::tie(s, c) = sincos_ps(Vector2(value));
	TestCheckClose(s[0], sin(value), FLOAT_TOLERANCE);
	TestCheckClose(c[0], cos(value), FLOAT_TOLERANCE);
}

TestCase(SSEMath)
{
	TestSSEMath(0);
	TestSSEMath(1);
	TestSSEMath(2);
	TestSSEMath(3);
	TestSSEMath(4);
	TestSSEMath(5);
	TestSSEMath(6);
	TestSSEMath(-1);
	TestSSEMath(-2);
	TestSSEMath(-3);
	TestSSEMath(-4);
	TestSSEMath(-5);
	TestSSEMath(-6);
	TestSSEMath(0.1);
	TestSSEMath(0.01);
	TestSSEMath(0.001);
	TestSSEMath(10);
	TestSSEMath(100);
	TestSSEMath(1000);
	TestSSEMath(10000);

	TestSSEFunc(asin, -0.75);
	TestSSEFunc(asin, -0.5);
	TestSSEFunc(asin, -0.25);
	TestSSEFunc(asin, 0);
	TestSSEFunc(asin, 0.25);
	TestSSEFunc(asin, 0.5);
	TestSSEFunc(asin, 0.75);

	TestSSEFunc(acos, -0.75);
	TestSSEFunc(acos, -0.5);
	TestSSEFunc(acos, -0.25);
	TestSSEFunc(acos, 0);
	TestSSEFunc(acos, 0.25);
	TestSSEFunc(acos, 0.5);
	TestSSEFunc(acos, 0.75);

	TestSSEFunc(atan, -1);
	TestSSEFunc(atan, -0.5);
	TestSSEFunc(atan, 0);
	TestSSEFunc(atan, 0.5);
	TestSSEFunc(atan, 1);

	TestAtan2(1, 1);
	TestAtan2(5, 2);
	TestAtan2(90, 0.5);
	TestAtan2(2, 30);
	TestAtan2(5, 5);
	TestAtan2(7, 0);

	TestSSEFunc(fastsin, -3);
	TestSSEFunc(fastsin, -2.5);
	TestSSEFunc(fastsin, -2);
	TestSSEFunc(fastsin, -1.5);
	TestSSEFunc(fastsin, -1);
	TestSSEFunc(fastsin, -0.5);
	TestSSEFunc(fastsin, 0);
	TestSSEFunc(fastsin, 0.5);
	TestSSEFunc(fastsin, 1);
	TestSSEFunc(fastsin, 1.5);
	TestSSEFunc(fastsin, 2);
	TestSSEFunc(fastsin, 2.5);
	TestSSEFunc(fastsin, 3);

	TestSSEFunc(fastcos, -3);
	TestSSEFunc(fastcos, -2.5);
	TestSSEFunc(fastcos, -2);
	TestSSEFunc(fastcos, -1.5);
	TestSSEFunc(fastcos, -1);
	TestSSEFunc(fastcos, -0.5);
	TestSSEFunc(fastcos, 0);
	TestSSEFunc(fastcos, 0.5);
	TestSSEFunc(fastcos, 1);
	TestSSEFunc(fastcos, 1.5);
	TestSSEFunc(fastcos, 2);
	TestSSEFunc(fastcos, 2.5);
	TestSSEFunc(fastcos, 3);
}
#endif

EndTestSuite()
