//@@COPYRIGHT@@

// Unit tests for math functions

TestSuite(MathTest)

TestCase(DegRad)
{
	TestCheckClose(DegToRad(90), M_PI / 2, FLOAT_TOLERANCE);
	TestCheckClose(DegToRad(45), M_PI / 4, FLOAT_TOLERANCE);
	TestCheckClose(DegToRad(180), M_PI, FLOAT_TOLERANCE);
	TestCheckClose(DegToRad(360), M_PI * 2, FLOAT_TOLERANCE);
	TestCheckClose(RadToDeg(M_PI / 2), 90, FLOAT_TOLERANCE);
	TestCheckClose(RadToDeg(M_PI / 4), 45, FLOAT_TOLERANCE);
	TestCheckClose(RadToDeg(M_PI), 180, FLOAT_TOLERANCE);
	TestCheckClose(RadToDeg(M_PI * 2), 360, FLOAT_TOLERANCE);
}

TestCase(Pow2)
{
	TestCheck(IsPowerOf2(1));
	TestCheck(IsPowerOf2(2));
	TestCheck(!IsPowerOf2(3));
	TestCheck(IsPowerOf2(4));
	TestCheck(!IsPowerOf2(5));
	TestCheck(!IsPowerOf2(6));
	TestCheck(!IsPowerOf2(7));
	TestCheck(IsPowerOf2(8));
	TestCheck(!IsPowerOf2(9));
	TestCheck(!IsPowerOf2(10));
	TestCheck(!IsPowerOf2(255));
	TestCheck(IsPowerOf2(256));

	TestCheckEqual(NextPowerOf2(1), 1);
	TestCheckEqual(NextPowerOf2(2), 2);
	TestCheckEqual(NextPowerOf2(3), 4);
	TestCheckEqual(NextPowerOf2(4), 4);
	TestCheckEqual(NextPowerOf2(5), 8);
	TestCheckEqual(NextPowerOf2(6), 8);
	TestCheckEqual(NextPowerOf2(7), 8);
	TestCheckEqual(NextPowerOf2(8), 8);
	TestCheckEqual(NextPowerOf2(9), 16);
	TestCheckEqual(NextPowerOf2(10), 16);
	TestCheckEqual(NextPowerOf2(255), 256);
	TestCheckEqual(NextPowerOf2(256), 256);
}

TestCase(IntBitOps)
{
	TestCheckEqual(IntLog2(1), 0);
	TestCheckEqual(IntLog2(2), 1);
	TestCheckEqual(IntLog2(3), 1);
	TestCheckEqual(IntLog2(4), 2);
	TestCheckEqual(IntLog2(5), 2);
	TestCheckEqual(IntLog2(6), 2);
	TestCheckEqual(IntLog2(7), 2);
	TestCheckEqual(IntLog2(8), 3);
	TestCheckEqual(IntLog2(9), 3);
	TestCheckEqual(IntLog2(10), 3);
	TestCheckEqual(IntLog2(255), 7);
	TestCheckEqual(IntLog2(256), 8);

	TestCheckEqual(IntFFS(1), 0);
	TestCheckEqual(IntFFS(2), 1);
	TestCheckEqual(IntFFS(3), 0);
	TestCheckEqual(IntFFS(4), 2);
	TestCheckEqual(IntFFS(5), 0);
	TestCheckEqual(IntFFS(6), 1);
	TestCheckEqual(IntFFS(7), 0);
	TestCheckEqual(IntFFS(8), 3);
	TestCheckEqual(IntFFS(9), 0);
	TestCheckEqual(IntFFS(10), 1);
	TestCheckEqual(IntFFS(255), 0);
	TestCheckEqual(IntFFS(256), 8);
}

TestCase(FastSinCos)
{
	TestCheckClose(fastsin(0), sin(0), FLOAT_TOLERANCE);
	TestCheckClose(fastsin(0.25), sin(0.25), FLOAT_TOLERANCE);
	TestCheckClose(fastsin(0.5), sin(0.5), FLOAT_TOLERANCE);
	TestCheckClose(fastsin(0.75), sin(0.75), FLOAT_TOLERANCE);
	TestCheckClose(fastsin(1), sin(1), FLOAT_TOLERANCE);
	TestCheckClose(fastsin(1.25), sin(1.25), FLOAT_TOLERANCE);
	TestCheckClose(fastsin(1.5), sin(1.5), FLOAT_TOLERANCE);
	TestCheckClose(fastsin(-0.25), sin(-0.25), FLOAT_TOLERANCE);
	TestCheckClose(fastsin(-0.5), sin(-0.5), FLOAT_TOLERANCE);
	TestCheckClose(fastsin(-0.75), sin(-0.75), FLOAT_TOLERANCE);
	TestCheckClose(fastsin(-1), sin(-1), FLOAT_TOLERANCE);
	TestCheckClose(fastsin(-1.25), sin(-1.25), FLOAT_TOLERANCE);
	TestCheckClose(fastsin(-1.5), sin(-1.5), FLOAT_TOLERANCE);

	TestCheckClose(fastcos(0), cos(0), FLOAT_TOLERANCE);
	TestCheckClose(fastcos(0.25), cos(0.25), FLOAT_TOLERANCE);
	TestCheckClose(fastcos(0.5), cos(0.5), FLOAT_TOLERANCE);
	TestCheckClose(fastcos(0.75), cos(0.75), FLOAT_TOLERANCE);
	TestCheckClose(fastcos(1), cos(1), FLOAT_TOLERANCE);
	TestCheckClose(fastcos(1.25), cos(1.25), FLOAT_TOLERANCE);
	TestCheckClose(fastcos(1.5), cos(1.5), FLOAT_TOLERANCE);
	TestCheckClose(fastcos(-0.25), cos(-0.25), FLOAT_TOLERANCE);
	TestCheckClose(fastcos(-0.5), cos(-0.5), FLOAT_TOLERANCE);
	TestCheckClose(fastcos(-0.75), cos(-0.75), FLOAT_TOLERANCE);
	TestCheckClose(fastcos(-1), cos(-1), FLOAT_TOLERANCE);
	TestCheckClose(fastcos(-1.25), cos(-1.25), FLOAT_TOLERANCE);
	TestCheckClose(fastcos(-1.5), cos(-1.5), FLOAT_TOLERANCE);
}

TestCase(Half)
{
	TestCheckClose(HalfToFloat(FloatToHalf(0)), 0, 0.01);
	TestCheckClose(HalfToFloat(FloatToHalf(1)), 1, 0.01);
	TestCheckClose(HalfToFloat(FloatToHalf(2)), 2, 0.01);
	TestCheckClose(HalfToFloat(FloatToHalf(0.5)), 0.5, 0.01);
	TestCheckClose(HalfToFloat(FloatToHalf(3)), 3, 0.01);
	TestCheckClose(HalfToFloat(FloatToHalf(25)), 25, 0.01);
	TestCheckEqual(FloatToHalf(1.2), 15565);
}

EndTestSuite()

Matrix Test(Plane x)
{
	return MatrixReflect(x);
}
