//@@COPYRIGHT@@

// Boost test framework initialization

boost::unit_test::test_suite *init_unit_test_suite(int, char **)
{
	Math::Init();
	return NULL;
}
