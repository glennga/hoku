/// @file test-star.cpp
/// @author Glenn Galvizo
///
/// Source file for all Star class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "math/star.h"
#include "gtest/gtest.h"

/// Check that the components are not altered without set_unit being set.
TEST(StarConstructor, NoUnit) {
    Star a(1, 1, 1);
    EXPECT_DOUBLE_EQ(a.i, 1.0);
    EXPECT_DOUBLE_EQ(a.j, 1.0);
    EXPECT_DOUBLE_EQ(a.k, 1.0);
}

/// Check that the norm of the generated vector with set_unit = true is equal to 1.0.
TEST(StarConstructor, WithUnit) {
    Star a(1, 1, 1, 0, 1.0, true);
    EXPECT_DOUBLE_EQ(a.norm(), 1.0);
}

/// Check if two stars are correctly added together.
TEST(StarOperator, Plus) {
    Star a(1, 1, 1), b(0.5, 0.5, 0.5);
    EXPECT_EQ(b + b, a);
}

/// Check if two stars are correctly subtracted together.
TEST(StarOperator, Minus) {
    Star a(1, 1, 1), b(0.5, 0.5, 0.5);
    EXPECT_EQ(a - b, b);
}

/// Check if a star is scaled correctly.
TEST(StarOperator, Scalar) {
    Star a(0, 0, 1);
    EXPECT_DOUBLE_EQ((a * 2).norm(), 2.0);
}

/// Check if the norm is correctly computed for a star. Answers checked through WolframAlpha.
TEST(StarComputation, Norm) {
    Star a(1.2, 6.5, 1.8);
    double b = 6.85055;
    EXPECT_NEAR(a.norm(), b, 0.00001);
}

/// Check if the norm of a generated unit vector is equal to one.
TEST(StarNorm, Unit) {
    Star a = Star::chance() * 85.0, b = a.as_unit();
    EXPECT_DOUBLE_EQ(b.norm(), 1.0);
}

/// Check if an attempt to find the length of a <0, 0, 0> star is made.
TEST(StarNorm, UnitZeroStar) {
    Star a = Star::zero(), b = a.as_unit();
    EXPECT_EQ(a, b);
}

/// Check if two identical stars (component-wise) are determined to be equal.
TEST(StarEquality, Same) {
    Star a(1, 1, 1), b(1, 1, 1);
    EXPECT_EQ(a, b);
}

/// Check if two similar stars are equal in the given precision.
TEST(StarEquality, Precision) {
    Star a(0, 0, 1), b(0, 0, 1.001);
    EXPECT_TRUE(Star::is_equal(a, b, 0.0011));
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
    Star a(1, 1, 1), b(4, 0.8, 123);
    EXPECT_DOUBLE_EQ(Star::dot(a, b), 127.8);
}

/// Check if the cross product of two stars is computed correctly (test one). Answers checked through WolframAlpha.
TEST(StarComputation, CrossOne) {
    Star a(1, 1, 1), b(0, 0, 0), c = Star::cross(a, a);
    EXPECT_EQ(c, b);
}

/// Check if the cross product of two stars is computed correctly (test two). Answers checked through WolframAlpha.
TEST(StarComputation, CrossTwo) {
    Star a(1, 1, 1), b(4, 0.8, 123), c(-122.2, 119, 3.2), d = Star::cross(b, a);
    EXPECT_EQ(d, c);
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

/// Check that the label number of a star is set to 0.
TEST(StarLabel, Clear) {
    EXPECT_EQ(Star::reset_label(Star(0, 0, 0, 5)).label, 0);
}

/// Check that the when calculating the angle between the same stars, NaN is not returned.
TEST(StarAngle, Same) {
    Star a(1, 1, 1), b(1, 1, 1);
    EXPECT_FALSE(std::isnan(Star::angle_between(a, b)));
}

/// Check that the components returned by the get methods are as expected.
TEST(StarOperator, Get) {
    Star a(1, 2, 3, 4);
    EXPECT_DOUBLE_EQ(a[0], a.i);
    EXPECT_DOUBLE_EQ(a[1], a.j);
    EXPECT_DOUBLE_EQ(a[2], a.k);
    EXPECT_EQ(a.get_label(), a.label);
}

/// Check that the correct result is returned with within_angle function.
TEST(StarAngle, WithinMultipleStars) {
    Star::list a = {Star(1, 1, 1), Star(1.1, 1, 1), Star(1.00001, 1, 1)};
    Star::list b = {Star(1, 1, 1), Star(1.1, 1, 1), Star(-1, 1, 1)};
    EXPECT_TRUE(Star::within_angle(a, 15));
    EXPECT_FALSE(Star::within_angle(b, 15));
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