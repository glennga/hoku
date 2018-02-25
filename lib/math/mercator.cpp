/// @file mercator.h
/// @author Glenn Galvizo
///
/// Source file for Mercator class, which represents two-dimensional projections of three-dimensional unit vectors
/// (i.e. Stars).

#define _USE_MATH_DEFINES
#include <cmath>

#include "math/mercator.h"

/// Given a pixel point (X, Y) in an image and the ratio of pixels to degrees, project the star into 3D
/// Cartesian space. The ratio of pixels to degrees must be passed, and it is assumed that the image itself is a
/// square. Solution found here: https://stackoverflow.com/a/12734509
///
/// @param x X coordinate of the image point, using (0, 0) as the *image* center.
/// @param y Y coordinate of the image point, using (0, 0) as the *image* center.
/// @param dpp Degrees per pixel.
/// @return The given point (X, Y) as a normalized vector in a 3D Cartesian frame.
Vector3 Mercator::transform_point (const double x, const double y, const double dpp) {
    return Vector3::FromSpherical(1.0, (x * dpp / 90.0), 2 * atan(exp(y * dpp / 90.0)) - (M_PI / 2));
}