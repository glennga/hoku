/// @file trio.h
/// @author Glenn Galvizo
///
/// Header file for Trio class, which retrieves attributes based off of three Stars.

#ifndef HOKU_TRIO_H
#define HOKU_TRIO_H

#include "math/star.h"

/// @brief Feature class for planar and spherical triangle star identification procedures.
///
/// The trio class is used with the planar and spherical star identification procedures. Given three vectors, one
/// can find the area and polar moment of the formed planar or spherical triangle.
///
/// @example
/// @code{.cpp}
/// // Trio: {1, 1, 1}, {1, -1, 1}, and {-1, -1, 5}.
/// // Draw a planar triangle between the trio. Compute this area and polar moment.
/// std::cout << Trio::planar_area(Star(1, 1, 1), Star(1, -1, 1), Star(-1, -1, 5)) << std::endl;
/// std::cout << Trio::planar_moment(Star(1, 1, 1), Star(1, -1, 1), Star(-1, -1, 5)) << std::endl;
///
/// // Draw a spherical triangle between the trio. Compute this area and polar moment.
/// std::cout << Trio::spherical_area(Star(1, 1, 1), Star(1, -1, 1), Star(-1, -1, 5)) << std::endl;
/// std::cout << Trio::spherical_moment(Star(1, 1, 1), Star(1, -1, 1), Star(-1, -1, 5)) << std::endl;
/// @endcode
///
class Trio {
  public:
    /// Ensure default constructor is **not** generated.
    Trio () = delete;
  public:
    static double planar_area (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3);
    static double planar_moment (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3);
    
    static double spherical_area (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3);
    static double spherical_moment (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3, int td_h = 3);
    
    static double dot_angle (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &central);
    
    static const double INVALID_TRIO_A;
    static const double INVALID_TRIO_M;
    static const double DUPLICATE_STARS_IN_TRIO;

#if !defined ENABLE_TESTING_ACCESS
    private:
#endif
    // Alias for the distances between each star.
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
    /// Star one of the trio.
    Vector3 b_1;
    
    /// Star two of the trio.
    Vector3 b_2;
    
    /// Star three of the trio.
    Vector3 b_3;
};

#endif /* HOKU_TRIO_H */
