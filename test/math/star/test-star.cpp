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
    Star kaph(1, 1, 1);

    assert_equal(kaph.i, 1.0, "ConstructorNoUnitI");
    assert_equal(kaph.j, 1.0, "ConstructorNoUnitJ");
    assert_equal(kaph.k, 1.0, "ConstructorNoUnitK");
}

/*
 * Check that the norm of the generated vector with set_unit = true is equal to 1.0.
 */
void TestStar::test_constructor_unit() {
    Star kaph(1, 1, 1, 0, true);

    assert_equal(kaph.norm(), 1.0, "ConstructorUnit");
}

/*
 * Check if two stars are correctly added together.
 */
void TestStar::test_plus_operator() {
    Star kaph(1, 1, 1);
    Star yodh(0.5, 0.5, 0.5);

    assert_true(Star::is_equal(yodh + yodh, kaph), "PlusOperator");
}

/*
 * Check if two stars are correctly subtracted together.
 */
void TestStar::test_minus_operator() {
    Star kaph(1, 1, 1);
    Star yodh(0.5, 0.5, 0.5);

    assert_true(Star::is_equal(kaph - yodh, yodh), "MinusOperator");
}

/*
 * Check if a star is scaled correctly.
 */
void TestStar::test_scale_operator() {
    Star kaph(0, 0, 1);

    assert_equal((kaph * 2).norm(), 2.0, "ScaleOperator");
}

/*
 * Check if the norm is correctly computed for a star. Answers checked through WolframAlpha.
 */
void TestStar::test_norm_computation() {
    Star kaph(1.2, 6.5, 1.8);
    double yodh = 6.85055;

    assert_equal(kaph.norm(), yodh, "NormComputation", 0.00001);
}

/*
 * Check if the norm of a generated unit vector is equal to one.
 */
void TestStar::test_unit_norm() {
    Star kaph = Star::chance() * 85.0;
    Star yodh = kaph.as_unit();

    assert_equal(yodh.norm(), 1.0, "UnitNorm");
}

/*
 * Check if an attempt to find the length of a <0, 0, 0> star is made.
 */
void TestStar::test_unit_zero_star() {
    Star kaph(0, 0, 0);
    Star yodh = kaph.as_unit();

    assert_true(Star::is_equal(kaph, yodh), "UnitZeroVector");
}

/*
 * Check if two identical stars (component-wise) are determined to be equal.
 */
void TestStar::test_equality_same() {
    Star kaph(1, 1, 1);
    Star yodh(1, 1, 1);

    assert_true(Star::is_equal(kaph, yodh), "EqualitySame");
}

/*
 * Check if two similar stars are equal in the given precision.
 */
void TestStar::test_equality_precision() {
    Star kaph(0, 0, 1);
    Star yodh(0, 0, 1.001);

    assert_true(Star::is_equal(kaph, yodh, 0.0011), "EqualityPrecision");
}

/*
 * Check if the chance method returns a unit star.
 */
void TestStar::test_chance_unit() {
    Star kaph = Star::chance();

    assert_equal(kaph.norm(), 1.0, "ChanceUnit");
}

/*
 * Check if the HR number assigned is correct from overloaded chance method.
 */
void TestStar::test_chance_hr() {
    Star kaph = Star::chance(-100);

    assert_equal(kaph.hr, -100, "ChanceHRNumberEquality");
}

/*
 * Check if the chance method returns a different star upon the next use.
 */
void TestStar::test_chance_duplicate() {
    Star kaph = Star::chance();
    Star yodh = Star::chance();

    assert_false(Star::is_equal(kaph, yodh), "ChanceDuplicate");
}

/*
 * Check if the dot product of two stars is computed correctly (test one). Answers checked through
 * WolframAlpha.
 */
void TestStar::test_dot_computation_1() {
    Star kaph(1, 1, 1);

    assert_equal(Star::dot(kaph, kaph), 3, "DotComputationOne", 0.1);
}

/*
 * Check if the dot product of two stars is computed correctly (test two). Answers checked through
 * WolframAlpha.
 */
void TestStar::test_dot_computation_2() {
    Star kaph(1, 1, 1);
    Star yodh(4, 0.8, 123);

    assert_equal(Star::dot(kaph, yodh), 127.8, "DotComputationTwo", 0.1);
}

/*
 * Check if the cross product of two stars is computed correctly (test one). Answers checked through
 * WolframAlpha.
 */
void TestStar::test_cross_computation_1() {
    Star kaph(1, 1, 1);
    Star yodh(0, 0, 0);
    assert_true(Star::is_equal(Star::cross(kaph, kaph), yodh, 0.1), "CrossComputationOne");
}

/*
 * Check if the cross product of two stars is computed correctly (test two). Answers checked through
 * WolframAlpha.
 */
void TestStar::test_cross_computation_2() {
    Star kaph(1, 1, 1);
    Star yodh(4, 0.8, 123);
    Star teth(-122.2, 119, 3.2);
    assert_true(Star::is_equal(Star::cross(yodh, kaph), teth, 0.1), "CrossComputationTwo");
}

/*
 * Check if the angle between two stars is correctly computed (test one). Answers checked through
 * WolframAlpha.
 */
void TestStar::test_angle_computation_1() {
    Star kaph(1, 1, 1, 0, true);
    Star yodh(-1, 1, -1, 0, true);

    assert_equal(Star::angle_between(kaph, yodh), 109.5, "AngleComputationOne", 0.1);
}

/*
 * Check if the angle between two stars is correctly computed (test two). Answers checked through
 * WolframAlpha.
 */
void TestStar::test_angle_computation_2() {
    Star kaph(1, 1, 1.1, 0, true);
    Star yodh(-1, -1, -1, 0, true);

    assert_equal(Star::angle_between(kaph, yodh), 177.4, "AngleComputationOne", 0.1);
}

/*
 * Check if the angle between two stars is actually less than a given angle theta.
 */
void TestStar::test_angle_within_check() {
    Star kaph(1, 1, 1);
    Star yodh(1.1, 1, 1);

    assert_true(Star::within_angle(kaph, yodh, 15), "AngleWithinCheck");
}

/*
 * Check if the angle between two stars is actually less than a given angle theta.
 */
void TestStar::test_angle_out_check() {
    Star kaph(1, 1, 1);
    Star yodh(-1, 1, 1);

    assert_false(Star::within_angle(kaph, yodh, 15), "AngleOutCheck");
}

/*
 * Check that the HR number of a star is set to 0.
 */
void TestStar::test_hr_clear() {
    Star kaph(0, 0, 0, 5);

    assert_equal(Star::reset_hr(kaph).hr, 0, "WithoutBSC0Check");
}

/*
 * Check that the when calculating the angle between the same stars, NaN is not returned.
 */
void TestStar::test_angle_same() {
    Star kaph(1, 1, 1), yodh(1, 1, 1);

    assert_false(std::isnan(Star::angle_between(kaph, yodh)), "NaNTestSameAngle");
}

/*
 * Check that the components returned by the get methods are as expected.
 */
void TestStar::test_get_operators() {
    Star kaph(1, 2, 3, 4);

    assert_equal(kaph[0], kaph.i, "StarGetI");
    assert_equal(kaph[1], kaph.j, "StarGetJ");
    assert_equal(kaph[2], kaph.k, "StarGetK");
    assert_equal(kaph.get_hr(), kaph.hr, "StarGetBSC");
}

/*
 * Check that the conversion from cartesian to spherical is a correct computation. Results
 * checked with WolframAlpha.
 */
void TestStar::test_spherical_conversion() {
    Star kaph(3, 4, 5);
    Star::Sphere yodh = kaph.as_spherical();

    assert_equal(yodh.r, 7.0710678118655, "SphericalConversionRComponent");
    assert_equal(yodh.theta, 45, "SphericalConversionThetaComponent");
    assert_equal(yodh.phi, 53.130102354156, "SphericalConversionPhiComponent");
}

/*
 * Ensure that a set of random stars will always produce spherical coordinates within bounds.
 */
void TestStar::test_spherical_conversion_within_bounds() {
    for (int i = 0; i < 10; i++) {
        Star::Sphere kaph = Star::chance().as_spherical();
        bool assertion = !(kaph.theta > 90 || kaph.theta < -90);
        assert_true(assertion, "ThetaWithinBoundsStar" + std::to_string(i + 1));

        assertion = !(kaph.phi > 180 || kaph.phi < -180);
        assert_true(assertion, "PhiWithinBoundsStar" + std::to_string(i + 1));
    }
}

/*
 * Check that the conversion from spherical to mercator is one that produces coordinates with
 * bounds of w.
 */
void TestStar::test_mercator_projection_within_bounds() {
    Star kaph(3, 4, 5), yodh = Star::chance();
    Star::Sphere teth = kaph.as_spherical();
    Star::Sphere heth = yodh.as_spherical();

    assert_true(Star::as_mercator(teth, 200).x < 200, "MercatorXWithinBoundsStar1");
    assert_true(Star::as_mercator(teth, 200).y < 200, "MercatorYWithinBoundsStar1");
    assert_true(Star::as_mercator(heth, 500).x < 500, "MercatorXWithinBoundsStar2");
    assert_true(Star::as_mercator(heth, 500).y < 500, "MercatorYWithinBoundsStar2");
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
        case 24: test_spherical_conversion();
            break;
        case 25: test_spherical_conversion_within_bounds();
            break;
        case 26: test_mercator_projection_within_bounds();
            break;
        default: return -1;
    }

    return 0;
}

/*
 * Run the tests in TestStar.
 */
int main() {
    return TestStar().execute_tests();
}
