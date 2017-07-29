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
        friend class QuadNode;

    public:
        using list = std::vector<Mercator>;
        using quad = std::array<Mercator, 4>;

        Mercator() = default;

        // constructors given a star, or a set of coordinates
        Mercator(const Star &, const double);
        Mercator(const double, const double);

        // given a list of points, reduce to only the closest points to current
        virtual Mercator::list reduce_far_stars(const Mercator::list &, const double);

        // check if a point is within a defined quadrilateral
        bool is_within_bounds(const quad &);

    private:
        // project 3D vector to plane
        void project_star(const Star &, const double);

        // coordinates and projection width
        double x = 0, y = 0, w = 0;

        // Harvard revised number
        int hr = 0;
};

#endif /* HOKU_MERCATOR_H */
