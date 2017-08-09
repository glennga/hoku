/// @file test-star.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestStar class, as well as the main function to run the tests.

#include "test-star.h"

/// Check that the components are not altered without set_unit being set.
///
/// @return 0 when finished.
int TestStar::test_constructor_no_unit()
{
    Star a(1, 1, 1);

    assert_equal(a.i, 1.0, "ConstructorNoUnitI");
    assert_equal(a.j, 1.0, "ConstructorNoUnitJ");
    return 0 * assert_equal(a.k, 1.0, "ConstructorNoUnitK");
}

/// Check that the norm of the generated vector with set_unit = true is equal to 1.0.
///
/// @return 0 when finished.
int TestStar::test_constructor_unit()
{
    Star a(1, 1, 1, 0, true);

    return 0 * assert_equal(a.norm(), 1.0, "ConstructorUnit");
}

/// Check if two stars are correctly added together.
///
/// @return 0 when finished.
int TestStar::test_plus_operator()
{
    Star a(1, 1, 1), b(0.5, 0.5, 0.5);

    return 0 * assert_equal(b + b, a, "PlusOperator", (b + b).str() + "," + a.str());
}

/// Check if two stars are correctly subtracted together.
///
/// @return 0 when finished.
int TestStar::test_minus_operator()
{
    Star a(1, 1, 1), b(0.5, 0.5, 0.5);

    return 0 * assert_equal(a - b, b, "MinusOperator", (a - b).str() + "," + b.str());
}

/// Check if a star is scaled correctly.
///
/// @return 0 when finished.
int TestStar::test_scale_operator()
{
    Star a(0, 0, 1);

    return 0 * assert_equal((a * 2).norm(), 2.0, "ScaleOperator");
}

/// Check if the norm is correctly computed for a star. Answers checked through WolframAlpha.
///
/// @return 0 when finished.
int TestStar::test_norm_computation()
{
    Star a(1.2, 6.5, 1.8);
    double b = 6.85055;

    assert_equal(a.norm(), b, "NormComputation", 0.00001);

    return 0;
}

/// Check if the norm of a generated unit vector is equal to one.
///
/// @return 0 when finished.
int TestStar::test_unit_norm()
{
    Star a = Star::chance() * 85.0, b = a.as_unit();

    return 0 * assert_equal(b.norm(), 1.0, "UnitNorm");
}

/// Check if an attempt to find the length of a <0, 0, 0> star is made.
///
/// @return 0 when finished.
int TestStar::test_unit_zero_star()
{
    Star a = Star::zero(), b = a.as_unit();

    return 0 * assert_equal(a, b, "UnitZeroVector", a.str() + "," + b.str());
}

/// Check if two identical stars (component-wise) are determined to be equal.
///
/// @return 0 when finished.
int TestStar::test_equality_same()
{
    Star a(1, 1, 1), b(1, 1, 1);

    return 0 * assert_equal(a, b, "EqualitySame", a.str() + "," + b.str());
}

/// Check if two similar stars are equal in the given precision.
///
/// @return 0 when finished.
int TestStar::test_equality_precision()
{
    Star a(0, 0, 1), b(0, 0, 1.001);

    return 0 * assert_true(Star::is_equal(a, b, 0.0011), "EqualityPrecision", a.str() + "," + b.str() + "0.0011,");
}

/// Check if the chance method returns a unit star.
///
/// @return 0 when finished.
int TestStar::test_chance_unit()
{
    return 0 * assert_equal(Star::chance().norm(), 1.0, "ChanceUnit");
}

/// Check if the HR number assigned is correct from overloaded chance method.
///
/// @return 0 when finished.
int TestStar::test_chance_hr()
{
    return 0 * assert_equal(Star::chance(-100).hr, -100, "ChanceHRNumberEquality");
}

/// Check if the chance method returns a different star upon the next use.
///
/// @return 0 when finished.
int TestStar::test_chance_duplicate()
{
    Star a = Star::chance(), b = Star::chance();

    return 0 * assert_not_equal(a, b, "ChanceDuplicate", a.str() + "," + b.str());
}

/// Check if the dot product of two stars is computed correctly (test one). Answers checked through WolframAlpha.
///
/// @return 0 when finished.
int TestStar::test_dot_computation_1()
{
    Star a(1, 1, 1);

    return 0 * assert_equal(Star::dot(a, a), 3, "DotComputationOne", 0.1);
}

/// Check if the dot product of two stars is computed correctly (test two). Answers checked through WolframAlpha.
///
/// @return 0 when finished.
int TestStar::test_dot_computation_2()
{
    Star a(1, 1, 1), b(4, 0.8, 123);

    return 0 * assert_equal(Star::dot(a, b), 127.8, "DotComputationTwo", 0.1);
}

/// Check if the cross product of two stars is computed correctly (test one). Answers checked through WolframAlpha.
///
/// @return 0 when finished.
int TestStar::test_cross_computation_1()
{
    Star a(1, 1, 1), b(0, 0, 0), c = Star::cross(a, a);

    return 0 * assert_equal(c, b, "CrossComputationOne", c.str() + "," + b.str());
}

/// Check if the cross product of two stars is computed correctly (test two). Answers checked through WolframAlpha.
///
/// @return 0 when finished.
int TestStar::test_cross_computation_2()
{
    Star a(1, 1, 1), b(4, 0.8, 123), c(-122.2, 119, 3.2), d = Star::cross(b, a);

    return 0 * assert_equal(d, c, "CrossComputationTwo", d.str() + "," + c.str());
}

/// Check if the angle between two stars is correctly computed (test one). Answers checked through WolframAlpha.
///
/// @return 0 when finished.
int TestStar::test_angle_computation_1()
{
    Star a(1, 1, 1, 0, true), b(-1, 1, -1, 0, true);

    return 0 * assert_equal(Star::angle_between(a, b), 109.5, "AngleComputationOne", 0.1);
}

/// Check if the angle between two stars is correctly computed (test two). Answers checked through WolframAlpha.
///
/// @return 0 when finished.
int TestStar::test_angle_computation_2()
{
    Star a(1, 1, 1.1, 0, true), b(-1, -1, -1, 0, true);

    return 0 * assert_equal(Star::angle_between(a, b), 177.4, "AngleComputationOne", 0.1);
}

/// Check if the angle between two stars is actually less than a given angle theta.
///
/// @return 0 when finished.
int TestStar::test_angle_within_check()
{
    Star a(1, 1, 1), b(1.1, 1, 1);

    return 0 * assert_true(Star::within_angle(a, b, 15), "AngleWithinCheck", a.str() + "," + b.str() + ",15");
}

/// Check if the angle between two stars is actually less than a given angle theta.
///
/// @return 0 when finished.
int TestStar::test_angle_out_check()
{
    Star a(1, 1, 1), b(-1, 1, 1);

    return 0 * assert_false(Star::within_angle(a, b, 15), "AngleOutCheck", a.str() + "," + b.str() + ",15");
}

/// Check that the HR number of a star is set to 0.
///
/// @return 0 when finished.
int TestStar::test_hr_clear()
{
    return 0 * assert_equal(Star::reset_hr(Star(0, 0, 0, 5)).hr, 0, "WithoutBSC0Check");
}

/// Check that the when calculating the angle between the same stars, NaN is not returned.
///
/// @return 0 when finished.
int TestStar::test_angle_same()
{
    Star a(1, 1, 1), b(1, 1, 1);

    return 0 * assert_false(std::isnan(Star::angle_between(a, b)), "NaNTestSameAngle", a.str() + "," + b.str());
}

/// Check that the components returned by the get methods are as expected.
///
/// @return 0 when finished.
int TestStar::test_get_operators()
{
    Star a(1, 2, 3, 4);

    assert_equal(a[0], a.i, "StarGetI");
    assert_equal(a[1], a.j, "StarGetJ");
    assert_equal(a[2], a.k, "StarGetK");
    return 0 * assert_equal(a.get_hr(), a.hr, "StarGetBSC");
}

/// Enumerate all tests in TestStar.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestStar::enumerate_tests(int test_case)
{
    switch (test_case)
    {
    case 0: return test_constructor_no_unit();
    case 1: return test_constructor_unit();
    case 2: return test_plus_operator();
    case 3: return test_minus_operator();
    case 4: return test_scale_operator();
    case 5: return test_norm_computation();
    case 6: return test_unit_norm();
    case 7: return test_unit_zero_star();
    case 8: return test_equality_same();
    case 9: return test_equality_precision();
    case 10: return test_chance_unit();
    case 11: return test_chance_hr();
    case 12: return test_chance_duplicate();
    case 13: return test_dot_computation_1();
    case 14: return test_dot_computation_2();
    case 15: return test_cross_computation_1();
    case 16: return test_cross_computation_2();
    case 17: return test_angle_computation_1();
    case 18: return test_angle_computation_2();
    case 19: return test_angle_within_check();
    case 20: return test_angle_out_check();
    case 21: return test_angle_same();
    case 22: return test_hr_clear();
    case 23: return test_get_operators();
    default: return -1;
    }
}

/// Run the tests in TestStar. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main()
{
    return TestStar().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
