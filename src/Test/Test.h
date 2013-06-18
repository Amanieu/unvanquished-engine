//@@COPYRIGHT@@

// Test framework main header

#include <boost/test/unit_test.hpp>
#include <iostream>

// Tolerance percentage for floating point comparaisons
// 6 significant figures ~= 20 bits of precision
#define FLOAT_TOLERANCE 0.0001

// Macros to make stuff more readable
#define TestSuite BOOST_AUTO_TEST_SUITE
#define EndTestSuite BOOST_AUTO_TEST_SUITE_END
#define TestCase BOOST_AUTO_TEST_CASE
#define TestCheck BOOST_CHECK
#define TestCheckEqual BOOST_CHECK_EQUAL
#define TestCheckClose BOOST_CHECK_CLOSE
#define TestCheckClose2(a, b) BOOST_CHECK_PREDICATE(TestClosePredicate(), (a)(b))
#define TestMsg BOOST_MESSAGE

// Custom predicate for comparing vector types
class TestClosePredicate {
public:
	template<int length> bool operator()(const Vector<length> &a, const Vector<length> &b)
	{
		boost::test_tools::close_at_tolerance<float> tester(boost::test_tools::percent_tolerance_t<float>(FLOAT_TOLERANCE));
		for (int i = 0; i < length; i++) {
			if (!tester(a[i], b[i]) && (fabs(a[i]) > FLOAT_TOLERANCE / 100 || fabs(b[i]) > FLOAT_TOLERANCE / 100))
				return false;
		}

		return true;
	}
	bool operator()(const Matrix &a, const Matrix &b)
	{
		for (int i = 0; i < 4; i++) {
			if (!operator()(a[i], b[i]))
				return false;
		}

		return true;
	}
};

// Helpers for printing vector types
template<int length> inline std::ostream &operator<<(std::ostream &os, const Vector<length> &vec)
{
	os << "{" << vec[0] << ", " << vec[1];
	if (length >= 3)
		os << ", " << vec[2];
	if (length == 4)
		os << ", " << vec[3];
	os << "}";
	return os;
}
template<int length> inline std::ostream &operator<<(std::ostream &os, const BVector<length> &vec)
{
	os << "{" << vec[0] << ", " << vec[1];
	if (length >= 3)
		os << ", " << vec[2];
	if (length == 4)
		os << ", " << vec[3];
	os << "}";
	return os;
}
inline std::ostream &operator<<(std::ostream &os, const Matrix &m)
{
	os << "\nMatrix(\n\t" << m[0] << ",\n\t" << m[1] << ",\n\t" << m[2] << ",\n\t" << m[3] << "\n)";
	return os;
}
