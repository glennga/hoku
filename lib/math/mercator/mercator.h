/// @file mercator.h
/// @author Glenn Galvizo
///
/// Header file for Mercator class, which represents two-dimensional projections of three-dimensional unit vectors
/// (i.e. Stars).

#ifndef HOKU_MERCATOR_H
#define HOKU_MERCATOR_H

#include "star.h"

/// The mercator class is meant to reduce a dimension off of a star. This class is used to flatten the points in
/// the BSC5 table and as the main data in the BSC5 quadtree.
class Mercator
{
    friend class TestMercator;
    friend class TestChomp;

public:
    /// Alias for a vector of Mercator points.
    using list = std::vector<Mercator>;

    /// Alias for a quartet of Mercator points.
    using quad = std::array<Mercator, 4>;

    /// Force default constructor. Default point is (0, 0) with w_n = 0 and hr = 0.
    Mercator() = default;

    Mercator(const Star &, const double);
    Mercator(const double, const double, const double, const int = 0);

    Mercator::list reduce_far_points(const Mercator::list &, const double);

    void present_projection(double &, double &, double &, int &) const;

    int get_hr();

protected:
    Mercator::quad find_corners(const double) const;

    bool is_within_bounds(const quad &) const;

protected:
    /// X coordinate of the projected point. Default point is (0, 0).
    double x = 0;

    /// Y coordinate of the projected point. Default point is (0, 0).
    double y = 0;

    /// Width of the map the point is projected onto. Default width is 0.
    double w_n = 0;

    /// Harvard Revised number for the point. Default is 0.
    int hr = 0;

private:
    void project_star(const Star &, const double);
};

#endif /* HOKU_MERCATOR_H */
