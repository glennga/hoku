/// @file test-random-draw.cpp
/// @author Glenn Galvizo
///
/// Source file for all RandomDraw class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "math/random-draw.h"
#include "gmock/gmock.h"

// Create an in-between matcher for Google Mock.
using testing::PrintToString;
using testing::Not;
MATCHER_P2(IsBetween, a, b,
           std::string(negation ? "isn't" : "is") + " between " + PrintToString(a) + " and " + PrintToString(b)) {
    return a <= arg && arg <= b;
}

/// Check that real numbers are unique, and that they exist between their defined bounds.
TEST(RealNumbers, DrawReal) {
    std::array<double, 10> a = {}, b = {};
    
    for (unsigned int i = 0; i < 10; i++) {
        a[i] = RandomDraw::draw_real(-10, 11), b[i] = RandomDraw::draw_real(0.00001, 0.001);
        EXPECT_THAT(a[i], IsBetween(-10, 11));
        EXPECT_THAT(b[i], IsBetween(0.00001, 0.001));
    }
    
    EXPECT_NE(a[0], a[1]);
    EXPECT_NE(b[0], b[1]);
}

/// Check that the generated normal numbers are unique, and that they are somewhat clustered.
TEST(NormalNumbers, DrawNormalClustered) {
    std::array<double, 20> a = {};
    double mu = 0, sigma = 0;
    for (unsigned int i = 0; i < 20; i++) {
        a[i] = RandomDraw::draw_normal(9, 0.000000001);
        EXPECT_THAT(a[i], IsBetween(5, 14));
        mu += a[i];
    }

    // Note: mu is needs to be divided by N here.
    for (const double &a_i : a) {
        sigma += pow(a_i - (mu / 20.0), 2);
    }
    sigma = sqrt((1 / 20.0) * sigma);
    
    EXPECT_THAT(sigma, IsBetween(0, 0.1));
    EXPECT_THAT(mu / 20.0, IsBetween(5, 14));
    EXPECT_THAT(mu / 20.0, IsBetween(5, 14));
    
    EXPECT_NE(a[0], a[1]);
}

/// Check that the generated normal numbers are unique, and that they are not clustered.
TEST(NormalNumbers, DrawNormalNotClustered) {
    std::array<double, 20> a = {};
    double mu = 0, sigma = 0;
    for (unsigned int i = 0; i < 20; i++) {
        a[i] = RandomDraw::draw_normal(9, 1000);
        EXPECT_THAT(a[i], Not(IsBetween(5, 14)));
        mu += a[i];
    }
    
    // Note: mu is needs to be divided by N here.
    for (const double &a_i : a) {
        sigma += pow(a_i - (mu / 20.0), 2);
    }
    sigma = sqrt((1 / 20.0) * sigma);
    
    EXPECT_THAT(sigma, Not(IsBetween(0, 0.1)));
    EXPECT_THAT(mu / 20.0, Not(IsBetween(5, 14)));
    EXPECT_THAT(mu / 20.0, Not(IsBetween(5, 14)));
    
    EXPECT_NE(a[0], a[1]);
}

/// Check that the integer numbers are unique, and that they exist between their defined boundaries.
TEST(IntegerNumbers, DrawInteger) {
    std::array<int, 10> a = {}, b = {};
    
    for (unsigned int i = 0; i < 10; i++) {
        a[i] = RandomDraw::draw_integer(-10, 11), b[i] = RandomDraw::draw_integer(1, 2);
        EXPECT_THAT(a[i], IsBetween(-10, 11));
        EXPECT_THAT(b[i], IsBetween(1, 2));
    }
    
    EXPECT_NE(a[0], a[1]);
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