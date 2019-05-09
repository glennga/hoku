/// @file test-star.cpp
/// @author Glenn Galvizo
///
/// Source file for all Star class unit tests.

#define _USE_MATH_DEFINES

#include <cmath>
#include "gtest/gtest.h"

#include "math/star.h"

TEST(Star, ConstructorNoUnit) {
    Star a(1, 1, 1);
    EXPECT_DOUBLE_EQ(a.data[0], 1.0);
    EXPECT_DOUBLE_EQ(a.data[1], 1.0);
    EXPECT_DOUBLE_EQ(a.data[2], 1.0);
}

TEST(Star, OperatorStream) {
    std::stringstream s;
    s << Star(1, 1, 1, 8, 10);
    EXPECT_EQ(s.str(), "(1.0000000000000000:1.0000000000000000:1.0000000000000000:8:10.0000000000000000)");
}

TEST(Star, OperatorGet) {
    Star a(1, 2, 3, 4);
    EXPECT_DOUBLE_EQ(a[0], a.data[0]);
    EXPECT_DOUBLE_EQ(a[1], a.data[1]);
    EXPECT_DOUBLE_EQ(a[2], a.data[2]);
}

TEST(Star, GetterLabel) {
    EXPECT_EQ(Star(1, 1, 1).get_label(), Star::NO_LABEL);
    EXPECT_EQ(Star(1, 1, 1, 2).get_label(), 2);
}

TEST(Star, GetterMagnitude) {
    EXPECT_EQ(Star(1, 1, 1).get_magnitude(), Star::NO_MAGNITUDE);
    EXPECT_EQ(Star(1, 1, 1, 2, 5).get_magnitude(), 5);
}

TEST(Star, OperatorPlus) {
    Star a(1, 1, 1, 4, 10), b(0.5, 0.5, 0.5, 5, 11);
    EXPECT_EQ(Star::wrap(b.get_vector() + b.get_vector(), b.get_label(), b.get_magnitude()), a);
}

TEST(Star, OperatorMinus) {
    Star a(1, 1, 1, 4, 10), b(0.5, 0.5, 0.5, 5, 11);
    EXPECT_EQ(Star::wrap(a.get_vector() - b.get_vector(), a.get_label(), a.get_magnitude()), b);
}

TEST(Star, OperatorScalar) {
    Star a(0, 0, 1, 4, 10);
    EXPECT_DOUBLE_EQ(Vector3::Magnitude(a.get_vector() * 2.0), 2.0);
}

TEST(Star, ComputationNorm) {
    Vector3 a(1.2, 6.5, 1.8), c = Vector3::Normalized(Vector3(0.1, 0.2, 0.3));
    double b = 6.85055;
    EXPECT_NEAR(Vector3::Magnitude(a), b, 0.00001);
    EXPECT_FLOAT_EQ(Vector3::Magnitude(c), 1.0);
}

TEST(Star, Normalize) {
    Star a(3, 5, 1, 5, 10);
    Vector3 b = Vector3::Normalized(a.get_vector());

    EXPECT_DOUBLE_EQ(Vector3::Magnitude(b), 1.0);
    EXPECT_DOUBLE_EQ(b.data[0], 3.0 / sqrt(35.0));
    EXPECT_DOUBLE_EQ(b.data[1], sqrt(5.0 / 7.0));
    EXPECT_DOUBLE_EQ(b.data[2], 1.0 / sqrt(35.0));
}

TEST(Star, NormUnitZeroStar) {
    Vector3 b = Vector3::Normalized(Vector3(0, 0, 0));
    EXPECT_EQ(Vector3(0, 0, 0), b);
}

TEST(Star, EqualitySame) {
    Star a(1, 1, 1), b(1, 1, 1), c(1, 1, 1, 123, 0.5);
    EXPECT_EQ(a, b);
    EXPECT_EQ(b, c);
}

TEST(Star, ChanceUnit) { EXPECT_DOUBLE_EQ(Vector3::Magnitude(Star::chance().get_vector()), 1.0); }

TEST(Star, ChanceLabel) { EXPECT_EQ(Star::chance(-100).get_label(), -100); }

TEST(Star, ChanceDuplicate) {
    Star a = Star::chance(), b = Star::chance();
    EXPECT_FALSE(a.get_vector() == b.get_vector());
}

TEST(Star, ComputationDotOne) {
    Star a(1, 1, 1);
    EXPECT_DOUBLE_EQ(Vector3::Dot(a.get_vector(), a.get_vector()), 3);
}

TEST(Star, ComputationDotTwo) {
    Star a(1, 1, 1, 5, 2), b(4, 0.8, 123);
    EXPECT_DOUBLE_EQ(Vector3::Dot(a.get_vector(), b.get_vector()), 127.8);
}

TEST(Star, ComputationCrossOne) {
    Star a(1, 1, 1), b(0, 0, 0);
    Vector3 c = Vector3::Cross(a.get_vector(), a.get_vector());
    EXPECT_EQ(c, b);
}

TEST(Star, ComputationCrossTwo) {
    Star a(1, 1, 1), b(4, 0.8, 123), c(-122.2, 119, 3.2);
    Vector3 d = Vector3::Cross(b.get_vector(), a.get_vector());
    EXPECT_EQ(d, c);
}

TEST(Star, ComputationAngleOne) {
    Star a(1, 1, 1, 0, true), b(-1, 1, -1, 0, true);
    EXPECT_NEAR(Vector3::Angle(a.get_vector(), b.get_vector()) * 180.0 / M_PI, 109.5, 0.1);
}

TEST(Star, ComputationAngleTwo) {
    Star a(1, 1, 1.1, 0, true), b(-1, -1, -1, 0, true);
    EXPECT_NEAR(Vector3::Angle(a.get_vector(), b.get_vector()) * 180.0 / M_PI, 177.4, 0.1);
}

TEST(Star, AngleSame) {
    Star a(1, 1, 1), b(1, 1, 1);
    EXPECT_FALSE(std::isnan(Vector3::Angle(a.get_vector(), b.get_vector())));
}

TEST(Star, AngleWithinMultipleStars) {
    Star::list a = {Star(1, 1, 1), Star(1.1, 1, 1), Star(1.00001, 1, 1)};
    Star::list b = {Star(1, 1, 1), Star(1.1, 1, 1), Star(-1, 1, 1)};
    Star::list c = {Star(1, 1, 1)};
    Star::list d = {};
    EXPECT_TRUE(Star::within_angle(a, 15));
    EXPECT_FALSE(Star::within_angle(b, 15));
    EXPECT_TRUE(Star::within_angle(c, 15));
    EXPECT_TRUE(Star::within_angle(d, 15));
}

TEST(Star, AngleWithinCheck) {
    Star a(1, 1, 1), b(1.1, 1, 1);
    EXPECT_TRUE(Star::within_angle(a, b, 15));
}

TEST(Star, AngleOutCheck) {
    Star a(1, 1, 1), b(-1, 1, 1);
    EXPECT_FALSE(Star::within_angle(a, b, 15));
}

TEST(Star, AngleMultipleCheck) {
    EXPECT_TRUE(Star::within_angle({Star(1, 1, 1), Star(1.1, 1, 1), Star(1.001, 1, 1)}, 15));
    EXPECT_FALSE(Star::within_angle({Star(1, 1, 1), Star(-1, 1, 1), Star(-1.1, 1, 1)}, 15));
}

TEST(Star, LabelClear) { EXPECT_EQ(Star::reset_label(Star(0, 0, 0, 5)).get_label(), 0); }

TEST(Star, LabelDefine) { EXPECT_EQ(Star::define_label(Star(0, 0, 0, 5), 9).get_label(), 9); }