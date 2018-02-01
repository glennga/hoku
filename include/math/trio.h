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
    
    /// Returned if we cannot compute a spherical area for a given trio.
    static constexpr double INVALID_TRIO_NEGATIVE_F = -1;
    
    /// Returned if there exists duplicate stars for a given trio.
    static constexpr double DUPLICATE_STARS_IN_TRIO = 0;
    
  public:
    static double planar_area (const Star &b_1, const Star &b_2, const Star &b_3);
    static double planar_moment (const Star &b_1, const Star &b_2, const Star &b_3);
    
    static double spherical_area (const Star &b_1, const Star &b_2, const Star &b_3);
    static double spherical_moment (const Star &b_1, const Star &b_2, const Star &b_3, int td_h= 3);

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    // Alias for the distances between each star.
    using side_lengths = std::array<double, 3>;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    Trio (const Star &b_1, const Star &b_2, const Star &b_3);
    
    side_lengths planar_lengths () const;
    side_lengths spherical_lengths () const;
    
    static double semi_perimeter (double a, double b, double c);
    
    Star planar_centroid () const;
    
    double recurse_spherical_moment (const Star &c, int td_n, int td_i);
    static Trio cut_triangle (const Star &c_1, const Star &c_2, const Star &c_3, const Star &keep = Star::zero());

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Star one of the trio.
    Star b_1;
    
    /// Star two of the trio.
    Star b_2;
    
    /// Star three of the trio.
    Star b_3;
};

#endif /* HOKU_TRIO_H */
