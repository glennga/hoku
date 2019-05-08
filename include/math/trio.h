/// @file trio.h
/// @author Glenn Galvizo
///
/// Header file for Trio class, which retrieves attributes based off of three Stars.

#ifndef HOKU_TRIO_H
#define HOKU_TRIO_H

#include <memory>

#include "math/star.h"

/// @brief Feature class for planar and spherical triangle star identification procedures.
///
/// The trio class is used with the planar and spherical star identification procedures. Given three vectors, one
/// can find the area and polar moment of the formed planar or spherical triangle.
class Trio {
public:
    Trio () = delete;

    struct either {
        double result = 0;
        int error = 0;
    };

public:
    static double planar_area (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3);
    static double planar_moment (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3);
    static either spherical_area (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3);
    static either spherical_moment (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3, int td_h = 3);
    static double dot_angle (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &central);

    static const int INVALID_TRIO_A_EITHER;
    static const int INVALID_TRIO_M_EITHER;

#if !defined ENABLE_TESTING_ACCESS
    private:
#endif
    using side_lengths = std::array<double, 3>;

#if !defined ENABLE_TESTING_ACCESS
    private:
#endif

    Trio (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3);

    side_lengths planar_lengths () const;
    side_lengths spherical_lengths () const;
    static double semi_perimeter (double a, double b, double c);
    Vector3 planar_centroid () const;
    double recurse_spherical_moment (const Vector3 &c, int td_n, int td_i);
    static Trio cut_triangle (const Vector3 &c_1, const Vector3 &c_2, const Vector3 &c_3, int k);

#if !defined ENABLE_TESTING_ACCESS
    private:
#endif
    std::shared_ptr<Vector3> b_1;
    std::shared_ptr<Vector3> b_2;
    std::shared_ptr<Vector3> b_3;
};

#endif /* HOKU_TRIO_H */
