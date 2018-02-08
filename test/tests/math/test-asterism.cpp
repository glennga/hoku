/// @file test-asterism.cpp
/// @author Glenn Galvizo
///
/// Source file for all Asterism class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "math/asterism.h"
#include "gmock/gmock.h"

// Import several matchers from Google Mock.
using testing::Contains;
using testing::Not;

/// Check that stars A, B, C, and D are found correctly.
TEST(AsterismProperty, ABCDStarFind) {
    Star::quad m = {Star::chance(1), Star::chance(2), Star::chance(3), Star::chance(4)};
    Asterism::points n = {Mercator(m[0], 1), Mercator(m[1], 1), Mercator(m[2], 1), Mercator(m[3], 1)};
    Asterism p(m);
    double d_max = 0;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            double d = Mercator::distance_between(n[i], n[j]);
            d_max = (d > d_max) ? d : d_max;
        }
    }
    
    // C should not be A or B. D should not be A, B, or C.
    auto e = {p.a.get_label(), p.b.get_label()}, f = {p.a.get_label(), p.b.get_label(), p.c.get_label()};
    EXPECT_DOUBLE_EQ(Mercator::distance_between(p.a, p.b), d_max);
    EXPECT_THAT(e, Not(Contains(p.c.get_label())));
    EXPECT_THAT(f, Not(Contains(p.d.get_label())));
}

/// Local coordinates returned should be inside [-1, 1]. Run this test 50 times.
TEST(AsterismProperty, HashNormalized) {
    bool is_not_normal = false;
    
    for (int i = 0; i < 50; i++) {
        Asterism::points_cd m = Asterism::hash(
            {Star::chance(), Star::chance(), Star::chance(), Star::chance()});
        if (fabs(m[0]) > 1 || fabs(m[1]) > 1 || fabs(m[2]) > 1 || fabs(m[3]) > 1) {
            is_not_normal = true;
        }
    }
    EXPECT_FALSE(is_not_normal);
}

/// Ensure the conditions x_c <= x_d and x_c + x_d <= 1 hold true. Run test 50 times.
TEST(AsterismProperty, CDSymmetry) {
    bool is_not_symmetrical = false;
    
    for (int i = 0; i < 50; i++) {
        Asterism::points_cd m = Asterism::hash(
            {Star::chance(), Star::chance(), Star::chance(), Star::chance()});
        if ((m[0] < m[2] || m[0] + m[2] > 1) && m[0] + m[1] + m[2] + m[3] != 0) {
            is_not_symmetrical = true;
        }
    }
    EXPECT_FALSE(is_not_symmetrical);
}

/// Ensure that the center of a n=4 group of stars is **unique**. The fact that the stars actually lie dead in the
/// center of the asterism isn't important.
TEST(AsterimProperty, UniqueCenter) {
    Star::list a;
    a.reserve(10000);
    for (int i = 0; i < 10000; i++) {
        Star::quad b = {Star::chance(), Star::chance(), Star::chance(), Star::chance()};
        a.push_back(Asterism::center(b));
    }
    
    bool assertion = true;
    for (unsigned int i = 0; i < a.size(); i++) {
        for (unsigned int j = 0; j < a.size(); j++) {
            if (a[i] == a[j] && i != j) {
                assertion = false;
            }
        }
    }
    EXPECT_TRUE(assertion);
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