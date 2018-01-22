/// @file test-mercator.cpp
/// @author Glenn Galvizo
///
/// Source file for all Mercator class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "math/mercator.h"
#include "gmock/gmock.h"

// Create an in-between matcher for Google Mock.
using testing::PrintToString;
MATCHER_P2(IsBetween, a, b,
           std::string(negation ? "isn't" : "is") + " between " + PrintToString(a) + " and " + PrintToString(b)) {
    return a <= arg && arg <= b;
}

/// Check that the conversion from cartesian to mercator is one that produces coordinates with bounds of w.
TEST(MeractorProperty, ProjectionWithinBounds) {
    Star a(3, 4, 5), b = Star::chance();
    EXPECT_THAT(Mercator(a, 200).x, IsBetween(-100, 100));
    EXPECT_THAT(Mercator(a, 200).y, IsBetween(-100, 100));
    EXPECT_THAT(Mercator(b, 500).x, IsBetween(-250, 250));
    EXPECT_THAT(Mercator(b, 500).y, IsBetween(-250, 250));
}

/// Check that the corners returned actually form a box.
TEST(MercatorProperty, CornersFormBox) {
    Mercator a(Star::chance(), 1000);
    Mercator::quad b = a.find_corners(100);
    EXPECT_DOUBLE_EQ(b[0].y, b[1].y);
    EXPECT_DOUBLE_EQ(b[2].y, b[3].y);
    EXPECT_DOUBLE_EQ(b[0].x, b[2].x);
    EXPECT_DOUBLE_EQ(b[1].x, b[3].x);
}

/// Check that points are correctly distinguished from being outside and inside a given boundary.
TEST(MercatorProperty, IsWithinBounds) {
    Mercator::quad a = Mercator(0, 0, 1000).find_corners(100);
    EXPECT_FALSE(Mercator(5000, 5000, 1000).is_within_bounds(a));
    EXPECT_TRUE(Mercator(1, 1, 1000).is_within_bounds(a));
}

/// Test the distance_between method. Answers checked with WolframAlpha.
TEST(MercatorProperty, DistanceBetween) {
    Mercator a(500, 500, 1), b(0, 0, 1), c(-800, -450, 2);
    EXPECT_DOUBLE_EQ(Mercator::distance_between(a, b), 500 * sqrt(2));
    EXPECT_DOUBLE_EQ(Mercator::distance_between(b, c), 50 * sqrt(337));
    EXPECT_DOUBLE_EQ(Mercator::distance_between(a, c), 50 * sqrt(1037));
}
/// Tests the [] operator. These access the X and Y components of the star.
TEST(MercatorOperator, Bracket) {
    double a = Mercator(500, 1, 4)[0], b = Mercator(500, 1, 4)[1];
    EXPECT_DOUBLE_EQ(a, 500);
    EXPECT_DOUBLE_EQ(b, 1);
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