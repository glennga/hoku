/// @file test-star.cpp
/// @author Glenn Galvizo
///
/// Source file for all Star class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include <cmath>
#include "gtest/gtest.h"

#include "math/star.h"

/// Check that the components are not altered without apply_normalize being set.
TEST(StarConstructor, NoUnit) {
    Star a(1, 1, 1);
    EXPECT_DOUBLE_EQ(a.i, 1.0);
    EXPECT_DOUBLE_EQ(a.j, 1.0);
    EXPECT_DOUBLE_EQ(a.k, 1.0);
}

/// Check that the norm of the generated vector with apply_normalize = true is equal to 1.0.
TEST(StarConstructor, WithUnit) {
    Star a(1, 1, 1, 0, 1.0, true);
    EXPECT_DOUBLE_EQ(a.norm(), 1.0);
}

/// Check that the string returned by the stream method is correct.
TEST(StarOperator, Stream) {
    std::stringstream s;
    s << Star(1, 1, 1, 8, 10, false);
    EXPECT_EQ(s.str(), "(1.0000000000000000:1.0000000000000000:1.0000000000000000:8:10.0000000000000000)");
}

/// Check that the components returned by the get methods are as expected.
TEST(StarOperator, Get) {
    Star a(1, 2, 3, 4);
    EXPECT_DOUBLE_EQ(a[0], a.i);
    EXPECT_DOUBLE_EQ(a[1], a.j);
    EXPECT_DOUBLE_EQ(a[2], a.k);
    EXPECT_EQ(a.get_label(), a.label);

    EXPECT_DOUBLE_EQ(a[3], Star::INVALID_ELEMENT_ACCESSED);
}

/// Check that the get label method returns the correct label.
TEST(StarGetter, Label) {
    EXPECT_EQ(Star(1, 1, 1).get_label(), Star::NO_LABEL);
    EXPECT_EQ(Star(1, 1, 1, 2).get_label(), 2);
}

/// Check that the get magnitude method returns the correct magitude.
TEST(StarGetter, Magnitude) {
    EXPECT_EQ(Star(1, 1, 1).get_magnitude(), Star::NO_MAGNITUDE);
    EXPECT_EQ(Star(1, 1, 1, 2, 5).get_magnitude(), 5);
}

/// Check if two stars are correctly added together.
TEST(StarOperator, Plus) {
    Star a(1, 1, 1, 4, 10), b(0.5, 0.5, 0.5, 5, 11);
    
    EXPECT_EQ(b + b, a);
    EXPECT_EQ((b + a).get_label(), b.get_label());
    EXPECT_EQ((b + a).get_magnitude(), b.get_magnitude());
    EXPECT_EQ((a + b).get_label(), a.get_label());
    EXPECT_EQ((a + b).get_magnitude(), a.get_magnitude());
}

/// Check if two stars are correctly subtracted together.
TEST(StarOperator, Minus) {
    Star a(1, 1, 1, 4, 10), b(0.5, 0.5, 0.5, 5, 11);
    EXPECT_EQ(a - b, b);
    
    EXPECT_EQ((b - a).get_label(), b.get_label());
    EXPECT_EQ((b - a).get_magnitude(), b.get_magnitude());
    EXPECT_EQ((a - b).get_label(), a.get_label());
    EXPECT_EQ((a - b).get_magnitude(), a.get_magnitude());
}

/// Check if a star is scaled correctly.
TEST(StarOperator, Scalar) {
    Star a(0, 0, 1, 4, 10);
    
    EXPECT_DOUBLE_EQ((a * 2).norm(), 2.0);
    EXPECT_DOUBLE_EQ((a * 2).get_label(), a.get_label());
    EXPECT_DOUBLE_EQ((a * 2).get_magnitude(), a.get_magnitude());
}

/// Check if the norm is correctly computed for a star. Answers checked through WolframAlpha.
TEST(StarComputation, Norm) {
    Star a(1.2, 6.5, 1.8), c = Star(0.1, 0.2, 0.3).normalize();
    double b = 6.85055;
    EXPECT_NEAR(a.norm(), b, 0.00001);
    EXPECT_FLOAT_EQ(c.norm(), 1.0);
}

/// Check if the normalize function returns the expected star. Answers checked with WolframAlpha.
TEST(StarNorm, Normalize) {
    Star a(3, 5, 1, 5, 10), b;
    b = a.normalize();
    
    EXPECT_DOUBLE_EQ(b.norm(), 1.0);
    EXPECT_DOUBLE_EQ(b[0], 3.0 / sqrt(35.0));
    EXPECT_DOUBLE_EQ(b[1], sqrt(5.0 / 7.0));
    EXPECT_DOUBLE_EQ(b[2], 1.0 / sqrt(35.0));
    EXPECT_EQ(b.get_label(), a.get_label());
    EXPECT_EQ(b.get_magnitude(), a.get_magnitude());
}

/// Check if the norm of a generated unit vector is equal to one.
TEST(StarNorm, Unit) {
    Star a = Star::chance() * 85.0, b = a.normalize();
    EXPECT_DOUBLE_EQ(b.norm(), 1.0);
}

/// Check if an attempt to find the length of a <0, 0, 0> star is made.
TEST(StarNorm, UnitZeroStar) {
    Star a = Star::zero(), b = a.normalize();
    EXPECT_EQ(a, b);
}

/// Check if two identical stars (component-wise) are determined to be equal.
TEST(StarEquality, Same) {
    Star a(1, 1, 1), b(1, 1, 1), c(1, 1, 1, 123, 0.5);
    EXPECT_EQ(a, b);
    EXPECT_EQ(b, c);
}

/// Check if two similar stars are equal in the given precision.
TEST(StarEquality, Precision) {
    Star a(0, 0, 1), b(0, 0, 1.001);
    EXPECT_TRUE(Star::is_equal(a, b, 0.0011));
}

/// Check that the zero star actually contains the vector [0, 0, 0].
TEST(StarZero, Zero) {
    Star a = Star::zero();
    EXPECT_EQ(a[0], 0);
    EXPECT_EQ(a[1], 0);
    EXPECT_EQ(a[2], 0);
    EXPECT_EQ(a.get_label(), Star::NO_LABEL);
    EXPECT_EQ(a.get_magnitude(), Star::NO_MAGNITUDE);
}

/// Check if the chance method returns a unit star.
TEST(StarChance, Unit) {
    EXPECT_DOUBLE_EQ(Star::chance().norm(), 1.0);
}

/// Check if the label number assigned is correct from overloaded chance method.
TEST(StarChance, Label) {
    EXPECT_EQ(Star::chance(-100).label, -100);
}

/// Check if the chance method returns a different star upon the next use.
TEST(StarChance, Duplicate) {
    Star a = Star::chance(), b = Star::chance();
    EXPECT_FALSE(a == b);
}

/// Check if the dot product of two stars is computed correctly (test one). Answers checked through WolframAlpha.
TEST(StarComputation, DotOne) {
    Star a(1, 1, 1);
    EXPECT_DOUBLE_EQ(Star::dot(a, a), 3);
}

/// Check if the dot product of two stars is computed correctly (test two). Answers checked through WolframAlpha.
TEST(StarComputation, DotTwo) {
    Star a(1, 1, 1, 5, 2), b(4, 0.8, 123);
    EXPECT_DOUBLE_EQ(Star::dot(a, b), 127.8);
}

/// Check if the cross product of two stars is computed correctly (test one). Answers checked through WolframAlpha.
TEST(StarComputation, CrossOne) {
    Star a(1, 1, 1), b(0, 0, 0), c = Star::cross(a, a);
    EXPECT_EQ(c, b);
    EXPECT_EQ(c.get_label(), Star::NO_LABEL);
    EXPECT_EQ(c.get_magnitude(), Star::NO_MAGNITUDE);
}

/// Check if the cross product of two stars is computed correctly (test two). Answers checked through WolframAlpha.
TEST(StarComputation, CrossTwo) {
    Star a(1, 1, 1), b(4, 0.8, 123), c(-122.2, 119, 3.2), d = Star::cross(b, a);
    EXPECT_EQ(d, c);
    EXPECT_EQ(d.get_label(), Star::NO_LABEL);
    EXPECT_EQ(d.get_magnitude(), Star::NO_MAGNITUDE);
}

/// Check if the angle between two stars is correctly computed (test one). Answers checked through WolframAlpha.
TEST(StarComputation, AngleOne) {
    Star a(1, 1, 1, 0, true), b(-1, 1, -1, 0, true);
    EXPECT_NEAR(Star::angle_between(a, b), 109.5, 0.1);
}

/// Check if the angle between two stars is correctly computed (test two). Answers checked through WolframAlpha.
TEST(StarComputation, AngleTwo) {
    Star a(1, 1, 1.1, 0, true), b(-1, -1, -1, 0, true);
    EXPECT_NEAR(Star::angle_between(a, b), 177.4, 0.1);
}

/// Check that the when calculating the angle between the same stars, NaN is not returned.
TEST(StarAngle, Same) {
    Star a(1, 1, 1), b(1, 1, 1);
    EXPECT_FALSE(std::isnan(Star::angle_between(a, b)));
}

/// Check that the correct result is returned with within_angle function.
TEST(StarAngle, WithinMultipleStars) {
    Star::list a = {Star(1, 1, 1), Star(1.1, 1, 1), Star(1.00001, 1, 1)};
    Star::list b = {Star(1, 1, 1), Star(1.1, 1, 1), Star(-1, 1, 1)};
    EXPECT_TRUE(Star::within_angle(a, 15));
    EXPECT_FALSE(Star::within_angle(b, 15));
}

/// Check if the angle between two stars is actually less than a given angle theta.
TEST(StarAngle, WithinCheck) {
    Star a(1, 1, 1), b(1.1, 1, 1);
    EXPECT_TRUE(Star::within_angle(a, b, 15));
}

/// Check if the angle between two stars is actually less than a given angle theta.
TEST(StarAngle, OutCheck) {
    Star a(1, 1, 1), b(-1, 1, 1);
    EXPECT_FALSE(Star::within_angle(a, b, 15));
}

/// Check that the overloaded 'within_angle' method for a list of stars actually returns the correct assesment.
TEST(StarAngle, MultipleCheck) {
    EXPECT_TRUE(Star::within_angle({Star(1, 1, 1), Star(1.1, 1, 1), Star(1.001, 1, 1)}, 15));
    EXPECT_FALSE(Star::within_angle({Star(1, 1, 1), Star(-1, 1, 1), Star(-1.1, 1, 1)}, 15));
}

/// Check that the label number of a star is set to 0.
TEST(StarLabel, Clear) {
    EXPECT_EQ(Star::reset_label(Star(0, 0, 0, 5)).get_label(), 0);
}

/// Check that the label number of a star is set to a defined number.
TEST(StarLabel, Define) {
    EXPECT_EQ(Star::define_label(Star(0, 0, 0, 5), 9).get_label(), 9);
}

/// Runs all tests defined in this file.
///
/// @param argc Argument count. Used in Google Test initialization.
/// @param argv Argument vector. Used in Google Test initialization.
/// @return The result of running all tests.
int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}