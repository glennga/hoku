/*
 * @file: mercator.h
 *
 * @brief: Header file for Mercator class, which represents two-dimensional projections of three-
 * dimensional unit vectors (Stars).
 */

#ifndef HOKU_MERCATOR_H
#define HOKU_MERCATOR_H

#include "star.h"

/*
 * @class Mercator
 * @brief Mercator class, which represents a Star in two dimensions.
 *
 * The mercator class is meant to reduce a dimension off of a star. This class is used to
 * flatten the points in the BSC5 table and as the main data in the BSC5 quadtree.
 */
class Mercator {
        friend class TestMercator;

    public:
        using list = std::vector<Mercator>;
        using quad = std::array<Mercator, 4>;

        Mercator() = default;

        // constructors given a star, or a set of coordinates
        Mercator(const Star &, const double);
        Mercator(const double, const double, const double, const int = 0);

        // given a list of points, reduce to only the closest points to current
        Mercator::list reduce_far_points(const Mercator::list &, const double);

        // access method coordinates, width, and hr
        void present_projection(double &, double &, double &, int &) const;

        // another access method for hr only
        int get_hr();

    protected:
        // find the corners of a box, given size 'a'
        Mercator::quad find_corners(const double) const;

        // check if a point is within a defined quadrilateral
        bool is_within_bounds(const quad &) const;

        // coordinates and projection width
        double x = 0, y = 0, w_n = 0;

        // Harvard revised number
        int hr = 0;

    private:
        // project 3D vector to plane
        void project_star(const Star &, const double);
};

#endif /* HOKU_MERCATOR_H */
