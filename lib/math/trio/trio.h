/*
 * @file: trio.h
 *
 * @brief: Header file for Trio class, which retrieves attributes based off of three Stars.
 */

#ifndef HOKU_TRIO_H
#define HOKU_TRIO_H

#include "star.h"
#include <math.h>

/*
 * @class Trio
 * @brief Trio class, which determines attributes given three 3D vectors.
 *
 * The trio class is used with the planar and spherical star identification procedures. Given
 * three vectors, one can find the area and polar moment of the formed planar or spherical triangle.
 */
class Trio {
    public:
        // ensure default constructor is **not** generated
        Trio() = delete;

        // treat trio as planar triangle, determine area/moment
        static double planar_area(const Star &, const Star &, const Star &);
        static double planar_moment(const Star &, const Star &, const Star &);

        // treat trio as spherical triangle, determine area/moment
        static double spherical_area(const Star &, const Star &, const Star &);
        static double spherical_moment(const Star &, const Star &, const Star &, const int=3);

#ifndef DEBUGGING_MODE_IS_ON
    private:
#endif
        // user is not meant to create Trio object, keep it private
        Trio(const Star &, const Star &, const Star &);

        // find the triangle lengths required for each operation
        std::array<double, 3> planar_lengths() const;
        std::array<double, 3> spherical_lengths() const;

        // find the semi perimeter given side lengths
        static double semi_perimeter(const double, const double, const double);

        // treat trio as planar triangle, find the centroid star
        Star planar_centroid() const;

        // recursively determine spherical moment, helper function to cut triangle
        double recurse_spherical_moment(const Star &, const int, const int);
        static Trio cut_triangle(const Star &, const Star &, const Star &,
                                 const Star & = Star(0, 0, 0));

        // individual stars, enumerated a, b, c
        Star a = Star(0, 0, 0);
        Star b = Star(0, 0, 0);
        Star c = Star(0, 0, 0);
};

#endif /* HOKU_TRIO_H */
