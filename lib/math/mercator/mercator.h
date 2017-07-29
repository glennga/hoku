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
        using mercator_list = std::vector<Mercator>;

        Mercator(const Star &, const double);
        Mercator(const double, const double);

        mercator_list reduce_far_stars(const mercator_list &, const double);

    private:
        // project 3D vector to plane
        void project_star(const Star &, const double);

        // coordinates and projection width
        double x, y, w;

        // Harvard revised number
        int hr;
};

#endif /* HOKU_MERCATOR_H */
