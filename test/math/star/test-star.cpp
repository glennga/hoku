/*
 * @file: test_star.cpp
 *
 * @brief: Source file for the TestStar class, as well as the main function to run the tests.
 */

#include "test-star.h"

/*
 * Check that the components are not altered without set_unit being set.
 */
void TestStar::test_constructor_no_unit() {
    Star a(1, 1, 1);

    assert_equal(a.i, 1.0, "ConstructorNoUnitI");
    assert_equal(a.j, 1.0, "ConstructorNoUnitJ");
    assert_equal(a.k, 1.0, "ConstructorNoUnitK");
}

/*
 * Check that the norm of the generated vector with set_unit = true is equal to 1.0.
 */
void TestStar::test_constructor_unit() {
    Star a(1, 1, 1, 0, true);

    assert_equal(a.norm(), 1.0, "ConstructorUnit");
}

/*
 * Check if two stars are correctly added together.
 */
void TestStar::test_plus_operator() {
    Star a(1, 1, 1);
    Star b(0.5, 0.5, 0.5);

    assert_equal(b + b, a, "PlusOperator",
                 (b + b).as_string() + "," + a.as_string());
}

/*
 * Check if two stars are correctly subtracted together.
 */
void TestStar::test_minus_operator() {
    Star a(1, 1, 1);
    Star b(0.5, 0.5, 0.5);

    assert_equal(a - b, b, "MinusOperator",
                 (a - b).as_string() + "," + b.as_string());
}

/*
 * Check if a star is scaled correctly.
 */
void TestStar::test_scale_operator() {
    Star a(0, 0, 1);

    assert_equal((a * 2).norm(), 2.0, "ScaleOperator");
}

/*
 * Check if the norm is correctly computed for a star. Answers checked through WolframAlpha.
 */
void TestStar::test_norm_computation() {
    Star a(1.2, 6.5, 1.8);
    double b = 6.85055;

    assert_equal(a.norm(), b, "NormComputation", 0.00001);
}

/*
 * Check if the norm of a generated unit vector is equal to one.
 */
void TestStar::test_unit_norm() {
    Star a = Star::chance() * 85.0;
    Star b = a.as_unit();

    assert_equal(b.norm(), 1.0, "UnitNorm");
}

/*
 * Check if an attempt to find the length of a <0, 0, 0> star is made.
 */
void TestStar::test_unit_zero_star() {
    Star a(0, 0, 0);
    Star b = a.as_unit();

    assert_equal(a, b, "UnitZeroVector", a.as_string() + "," + b.as_string());
}

/*
 * Check if two identical stars (component-wise) are determined to be equal.
 */
void TestStar::test_equality_same() {
    Star a(1, 1, 1);
    Star b(1, 1, 1);

    assert_equal(a, b, "EqualitySame", a.as_string() + "," + b.as_string());
}

/*
 * Check if two similar stars are equal in the given precision.
 */
void TestStar::test_equality_precision() {
    Star a(0, 0, 1);
    Star b(0, 0, 1.001);

    assert_true(Star::is_equal(a, b, 0.0011), "EqualityPrecision");
}

/*
 * Check if the chance method returns a unit star.
 */
void TestStar::test_chance_unit() {
    Star a = Star::chance();

    assert_equal(a.norm(), 1.0, "ChanceUnit");
}

/*
 * Check if the HR number assigned is correct from overloaded chance method.
 */
void TestStar::test_chance_hr() {
    Star a = Star::chance(-100);

    assert_equal(a.hr, -100, "ChanceHRNumberEquality");
}

/*
 * Check if the chance method returns a different star upon the next use.
 */
void TestStar::test_chance_duplicate() {
    Star a = Star::chance();
    Star b = Star::chance();

    assert_not_equal(a, b, "ChanceDuplicate", a.as_string() + "," + b.as_string());
}

/*
 * Check if the dot product of two stars is computed correctly (test one). Answers checked through
 * WolframAlpha.
 */
void TestStar::test_dot_computation_1() {
    Star a(1, 1, 1);

    assert_equal(Star::dot(a, a), 3, "DotComputationOne", 0.1);
}

/*
 * Check if the dot product of two stars is computed correctly (test two). Answers checked through
 * WolframAlpha.
 */
void TestStar::test_dot_computation_2() {
    Star a(1, 1, 1);
    Star b(4, 0.8, 123);

    assert_equal(Star::dot(a, b), 127.8, "DotComputationTwo", 0.1);
}

/*
 * Check if the cross product of two stars is computed correctly (test one). Answers checked through
 * WolframAlpha.
 */
void TestStar::test_cross_computation_1() {
    Star a(1, 1, 1);
    Star b(0, 0, 0);

    assert_equal(Star::cross(a, a), b, "CrossComputationOne",
                 Star::cross(a, a).as_string() + "," + b.as_string());
//    assert_true(Star::is_equal(Star::cross(a, a), b, 0.1), "CrossComputationOne");
}

/*
 * Check if the cross product of two stars is computed correctly (test two). Answers checked through
 * WolframAlpha.
 */
void TestStar::test_cross_computation_2() {
    Star a(1, 1, 1);
    Star b(4, 0.8, 123);
    Star c(-122.2, 119, 3.2);

    assert_equal(Star::cross(b, a), c, "CrossComputationTwo",
                 Star::cross(b, a).as_string() + "," + c.as_string());
}

/*
 * Check if the angle between two stars is correctly computed (test one). Answers checked through
 * WolframAlpha.
 */
void TestStar::test_angle_computation_1() {
    Star a(1, 1, 1, 0, true);
    Star b(-1, 1, -1, 0, true);

    assert_equal(Star::angle_between(a, b), 109.5, "AngleComputationOne", 0.1);
}

/*
 * Check if the angle between two stars is correctly computed (test two). Answers checked through
 * WolframAlpha.
 */
void TestStar::test_angle_computation_2() {
    Star a(1, 1, 1.1, 0, true);
    Star b(-1, -1, -1, 0, true);

    assert_equal(Star::angle_between(a, b), 177.4, "AngleComputationOne", 0.1);
}

/*
 * Check if the angle between two stars is actually less than a given angle theta.
 */
void TestStar::test_angle_within_check() {
    Star a(1, 1, 1);
    Star b(1.1, 1, 1);

    assert_true(Star::within_angle(a, b, 15), "AngleWithinCheck");
}

/*
 * Check if the angle between two stars is actually less than a given angle theta.
 */
void TestStar::test_angle_out_check() {
    Star a(1, 1, 1);
    Star b(-1, 1, 1);

    assert_false(Star::within_angle(a, b, 15), "AngleOutCheck");
}

/*
 * Check that the HR number of a star is set to 0.
 */
void TestStar::test_hr_clear() {
    Star a(0, 0, 0, 5);

    assert_equal(Star::reset_hr(a).hr, 0, "WithoutBSC0Check");
}

/*
 * Check that the when calculating the angle between the same stars, NaN is not returned.
 */
void TestStar::test_angle_same() {
    Star a(1, 1, 1), b(1, 1, 1);

    assert_false(std::isnan(Star::angle_between(a, b)), "NaNTestSameAngle");
}

/*
 * Check that the components returned by the get methods are as expected.
 */
void TestStar::test_get_operators() {
    Star a(1, 2, 3, 4);

    assert_equal(a[0], a.i, "StarGetI");
    assert_equal(a[1], a.j, "StarGetJ");
    assert_equal(a[2], a.k, "StarGetK");
    assert_equal(a.get_hr(), a.hr, "StarGetBSC");
}

/*
 * Enumerate all tests in TestStar.
 *
 * @return -1 if the test case does not exist. 0 otherwise.
 */
int TestStar::enumerate_tests(int test_case) {
    switch (test_case) {
        case 0: test_constructor_no_unit();
            break;
        case 1: test_constructor_unit();
            break;
        case 2: test_plus_operator();
            break;
        case 3: test_minus_operator();
            break;
        case 4: test_scale_operator();
            break;
        case 5: test_norm_computation();
            break;
        case 6: test_unit_norm();
            break;
        case 7: test_unit_zero_star();
            break;
        case 8: test_equality_same();
            break;
        case 9: test_equality_precision();
            break;
        case 10: test_chance_unit();
            break;
        case 11: test_chance_hr();
            break;
        case 12: test_chance_duplicate();
            break;
        case 13: test_dot_computation_1();
            break;
        case 14: test_dot_computation_2();
            break;
        case 15: test_cross_computation_1();
            break;
        case 16: test_cross_computation_2();
            break;
        case 17: test_angle_computation_1();
            break;
        case 18: test_angle_computation_2();
            break;
        case 19: test_angle_within_check();
            break;
        case 20: test_angle_out_check();
            break;
        case 21: test_angle_same();
            break;
        case 22: test_hr_clear();
            break;
        case 23: test_get_operators();
            break;
        default: return -1;
    }

    return 0;
}

/*
 * Run the tests in TestStar. Currently set to log all results.
 */
int main() {
    return TestStar().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
