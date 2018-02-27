/// @file test-mercator.cpp
/// @author Glenn Galvizo
///
/// Source file for all Mercator class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#define _USE_MATH_DEFINES
#include <cmath>
#include "third-party/gmath/Vector2.hpp"
#include "gmock/gmock.h"

#include "math/mercator.h"
#include "math/random-draw.h"

/// Check that the conversion between 2D and 3D retains the given pixel distance. We are looking for **very**
/// ballpark estimates.
TEST(MercatorTransform, Point) {
    for (unsigned int i = 0; i < 20; i++) {
        Vector2 a(RandomDraw::draw_real(-2500, 2500), RandomDraw::draw_real(-2500, 2500));
        Vector2 b(RandomDraw::draw_real(-2500, 2500), RandomDraw::draw_real(-2500, 2500));
        double distance = Vector2::Distance(a, b) * (5.0 / 5000.0);
        
        Vector3 c = Mercator::transform_point(a.X, a.Y, 5.0 / 5000.0);
        Vector3 d = Mercator::transform_point(b.X, b.Y, 5.0 / 5000.0);
        double theta = (180.0 / M_PI) * Vector3::Angle(c, d);
        EXPECT_NEAR(theta, distance, 5.0);
    }
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