/// @file test-mercator.cpp
/// @author Glenn Galvizo
///
/// Source file for all Mercator class unit tests.

#define ENABLE_TESTING_ACCESS

#define _USE_MATH_DEFINES

#include <cmath>
#include "gmock/gmock.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#include "third-party/gmath/Vector2.hpp"

#pragma GCC diagnostic pop

#include "math/mercator.h"
#include "math/random-draw.h"

/// Check that the conversion between 2D and 3D retains the given pixel distance. We are looking for ballpark estimates.
TEST(Mercator, TransformPoint) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    for (unsigned int i = 0; i < 20; i++) {
        Vector2 a(RandomDraw::draw_real(-2500, 2500), RandomDraw::draw_real(-2500, 2500));
        Vector2 b(RandomDraw::draw_real(-2500, 2500), RandomDraw::draw_real(-2500, 2500));
        double distance = Vector2::Distance(a, b) * (5.0 / 5000.0);

        Vector3 c = Mercator::transform_point(a.X, a.Y, 5.0 / 5000.0);
        Vector3 d = Mercator::transform_point(b.X, b.Y, 5.0 / 5000.0);
        double theta = (180.0 / M_PI) * Vector3::Angle(c, d);
        EXPECT_NEAR(theta, distance, 0.01);
    }
}