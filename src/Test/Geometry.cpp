//@@COPYRIGHT@@

// Tests for Box and Plane classes

TestSuite(GeometryTest)

TestCase(BoxTest)
{
	Box b1(Vector3(0, 0, 0), Vector3(3, 3, 3));
	Box b2(Vector3(1, 1, 1), Vector3(2, 2, 2));
	Box b3(Vector3(-1, -1, -1), Vector3(0.5, 0.5, 0.5));

	TestCheck(b1.IntersectsBox(b2));
	TestCheck(b1.IntersectsBox(b3));
	TestCheck(!b2.IntersectsBox(b3));

	TestCheck(b1.ContainsBox(b2));
	TestCheck(!b1.ContainsBox(b3));
	TestCheck(!b2.ContainsBox(b1));
	TestCheck(!b2.ContainsBox(b3));
	TestCheck(!b3.ContainsBox(b1));
	TestCheck(!b3.ContainsBox(b2));

	TestCheck(b3.ContainsPoint(Vector3(0, 0, 0)));
	TestCheck(!b3.ContainsPoint(Vector3(1, 1, 1)));
}

TestCase(PlaneTest)
{
	Plane p1(1, 0, 0, 0);
	Plane p2(1, 1, 1, -3);

	p2.Normalize();
	TestCheckClose(p2.Normal().Length(), 1, FLOAT_TOLERANCE);

	TestCheckEqual(p1.PointDist(Vector3(1, 1, 1)), 1);
	TestCheckEqual(p1.PointDist(Vector3(6, 4, 5)), 6);
	TestCheckEqual(p2.PointDist(Vector3(1, 1, 1)), 0);
	TestCheckEqual(p2.PointDist(Vector3(3, 1, 2)), sqrt(3));

	Plane p3 = PlaneFromNormalPoint(Vector3(1, 1, 1), Vector3(1, 1, 1));
	p3.Normalize();
	TestCheckEqual(p3, p2);

	Plane p4 = PlaneFromPoints(Vector3(0, 0, 1), Vector3(1, 0, 1), Vector3(0, 1, 1));
	TestCheckEqual(p4, Plane(0, 0, 1, -1));

	Vector3 intersect = PlaneIntersection(Plane(1, 0, 0, -1), Plane(0, 1, 0, -1), Plane(0, 0, 1, 0));
	TestCheckEqual(intersect, Vector3(1, 1, 0));

	Box box(Vector3(-1, -1, -1), Vector3(1, 1, 1));
	TestCheckEqual(PlaneFromNormalPoint(Vector3(1, 1, 1), Vector3(2, 2, 2)).CullBox(box), CULL_OUTSIDE);
	TestCheckEqual(PlaneFromNormalPoint(Vector3(1, 1, 1), Vector3(0, 0, 0)).CullBox(box), CULL_INTERSECT);
	TestCheckEqual(PlaneFromNormalPoint(Vector3(1, 1, 1), Vector3(-2, -2, -2)).CullBox(box), CULL_INSIDE);
}

EndTestSuite()
