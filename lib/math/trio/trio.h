/// @file trio.h
/// @author Glenn Galvizo
///
/// Header file for Trio class, which retrieves attributes based off of three Stars.

#ifndef HOKU_TRIO_H
#define HOKU_TRIO_H

#include "star.h"

/// The trio class is used with the planar and spherical star identification procedures. Given three vectors, one
/// can find the area and polar moment of the formed planar or spherical triangle.
///
/// @example
/// @code{.cpp}
/// // Trio: {1, 1, 1}, {1, -1, 1}, and {-1, -1, 5}.
/// // Draw a planar triangle between the trio. Compute this area and polar moment.
/// printf("%f", Trio::planar_area(Star(1, 1, 1), Star(1, -1, 1), Star(-1, -1, 5)));
/// printf("%f", Trio::planar_moment(Star(1, 1, 1), Star(1, -1, 1), Star(-1, -1, 5)));
///
/// // Draw a spherical triangle between the trio. Compute this area and polar moment.
/// printf("%f", Trio::spherical_area(Star(1, 1, 1), Star(1, -1, 1), Star(-1, -1, 5)));
/// printf("%f", Trio::spherical_moment(Star(1, 1, 1), Star(1, -1, 1), Star(-1, -1, 5)));
/// @endcode
///
class Trio {
  private:
    friend class TestTrio;
  
  public:
    /// Common alias for a trio of stars.
    using stars = std::array<Star, 3>;
    
    /// Ensure default constructor is **not** generated.
    Trio () = delete;
  
  public:
    static double planar_area (const Star &, const Star &, const Star &);
    static double planar_moment (const Star &, const Star &, const Star &);
    
    static double spherical_area (const Star &, const Star &, const Star &);
    static double spherical_moment (const Star &, const Star &, const Star &, const int= 3);
  
  private:
    // Alias for the distances between each star.
    using side_lengths = std::array<double, 3>;
  
  private:
    Trio (const Star &, const Star &, const Star &);
    
    side_lengths planar_lengths () const;
    side_lengths spherical_lengths () const;
    
    static double semi_perimeter (const double, const double, const double);
    
    Star planar_centroid () const;
    
    double recurse_spherical_moment (const Star &, const int, const int);
    static Trio cut_triangle (const Star &, const Star &, const Star &, const Star & = Star());
  
  private:
    /// Star one of the trio.
    Star b_1;
    
    /// Star two of the trio.
    Star b_2;
    
    /// Star three of the trio.
    Star b_3;
};

#endif /* HOKU_TRIO_H */
