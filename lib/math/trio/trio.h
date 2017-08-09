/// @file trio.h
/// @author Glenn Galvizo
///
/// Header file for Trio class, which retrieves attributes based off of three Stars.

#ifndef HOKU_TRIO_H
#define HOKU_TRIO_H

#include "star.h"

/// The trio class is used with the planar and spherical star identification procedures. Given three vectors, one
/// can find the area and polar moment of the formed planar or spherical triangle.
class Trio
{
    friend class TestTrio;

public:
    /// Common alias for a trio of stars.
    using stars = std::array<Star, 3>;

    /// Ensure default constructor is **not** generated.
    Trio() = delete;

    static double planar_area(const Star &, const Star &, const Star &);
    static double planar_moment(const Star &, const Star &, const Star &);

    static double spherical_area(const Star &, const Star &, const Star &);
    static double spherical_moment(const Star &, const Star &, const Star &, const int= 3);

private:
    // Alias for the distances between each star.
    using side_lengths = std::array<double, 3>;

    Trio(const Star &, const Star &, const Star &);

    side_lengths planar_lengths() const;
    side_lengths spherical_lengths() const;

    static double semi_perimeter(const double, const double, const double);

    Star planar_centroid() const;

    double recurse_spherical_moment(const Star &, const int, const int);
    static Trio cut_triangle(const Star &, const Star &, const Star &, const Star & = Star());

    /// Star one of the trio.
    Star b_1;

    /// Star two of the trio.
    Star b_2;

    /// Star three of the trio.
    Star b_3;
};

#endif /* HOKU_TRIO_H */
