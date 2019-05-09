/// @file test-random-draw.cpp
/// @author Glenn Galvizo
///
/// Source file for all RandomDraw class unit tests.

#define ENABLE_TESTING_ACCESS

#include <array>
#include "math/random-draw.h"
#include "gmock/gmock.h"

using testing::PrintToString;
using testing::Not;
MATCHER_P2(IsBetweenRandomDraw, a, b,
           std::string(negation ? "isn't" : "is") + " between " + PrintToString(a) + " and " + PrintToString(b)) {
    return a <= arg && arg <= b;
}

TEST(RandomDraw, DrawReal) {
    std::array<double, 10> a = {}, b = {};

    for (unsigned int i = 0; i < 10; i++) {
        a[i] = RandomDraw::draw_real(-10, 11), b[i] = RandomDraw::draw_real(0.00001, 0.001);
        EXPECT_THAT(a[i], IsBetweenRandomDraw(-10, 11));
        EXPECT_THAT(b[i], IsBetweenRandomDraw(0.00001, 0.001));
    }

    EXPECT_NE(a[0], a[1]);
    EXPECT_NE(b[0], b[1]);
}
TEST(RandomDraw, DrawNormalClustered) {
    std::array<double, 20> a = {};
    double mu = 0, sigma = 0;
    for (unsigned int i = 0; i < 20; i++) {
        a[i] = RandomDraw::draw_normal(9, 0.000000001);
        EXPECT_THAT(a[i], IsBetweenRandomDraw(5, 14));
        mu += a[i];
    }

    // Note: mu is needs to be divided by N here.
    for (const double &a_i : a) {
        sigma += pow(a_i - (mu / 20.0), 2);
    }
    sigma = sqrt((1 / 20.0) * sigma);

    EXPECT_THAT(sigma, IsBetweenRandomDraw(0, 0.1));
    EXPECT_THAT(mu / 20.0, IsBetweenRandomDraw(5, 14));
    EXPECT_THAT(mu / 20.0, IsBetweenRandomDraw(5, 14));

    EXPECT_NE(a[0], a[1]);
}
TEST(RandomDraw, DrawNormalNotClustered) {
    std::array<double, 20> a = {};
    double mu = 0, sigma = 0;
    for (unsigned int i = 0; i < 20; i++) {
        a[i] = RandomDraw::draw_normal(9, 1000);
        EXPECT_THAT(a[i], Not(IsBetweenRandomDraw(5, 14)));
        mu += a[i];
    }

    // Note: mu is needs to be divided by N here.
    for (const double &a_i : a) {
        sigma += pow(a_i - (mu / 20.0), 2);
    }
    sigma = sqrt((1 / 20.0) * sigma);

    EXPECT_THAT(sigma, Not(IsBetweenRandomDraw(0, 0.1)));
    EXPECT_THAT(mu / 20.0, Not(IsBetweenRandomDraw(5, 14)));
    EXPECT_THAT(mu / 20.0, Not(IsBetweenRandomDraw(5, 14)));

    EXPECT_NE(a[0], a[1]);
}
TEST(RandomDraw, DrawInteger) {
    std::array<int, 10> a = {}, b = {};

    for (unsigned int i = 0; i < 10; i++) {
        a[i] = RandomDraw::draw_integer(-10, 11), b[i] = RandomDraw::draw_integer(1, 2);
        EXPECT_THAT(a[i], IsBetweenRandomDraw(-10, 11));
        EXPECT_THAT(b[i], IsBetweenRandomDraw(1, 2));
    }

    EXPECT_NE(a[0], a[1]);
}