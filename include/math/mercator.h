/// @file mercator.h
/// @author Glenn Galvizo
///
/// Header file for Mercator class, which represents two-dimensional projections of three-dimensional unit vectors
/// (i.e. Stars).

#ifndef HOKU_MERCATOR_H
#define HOKU_MERCATOR_H

#include "math/star.h"

/// @brief Class to project 2D stars to 3D using Mercator projections.
///
/// @example
/// @code{.cpp}
/// // Project the point 1000, 1000 out of a 5000x5000 square to 3D space.
/// std::cout << Star::wrap(Mercator::transform_point(1000, 1000, 1/5000.0) << std::endl;
/// @endcode
class Mercator {
  public:
    static Vector3 transform_point (double x, double y, double dpp);
};

#endif /* HOKU_MERCATOR_H */
