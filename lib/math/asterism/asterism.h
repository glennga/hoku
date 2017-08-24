/// @file asterism.h
/// @author Glenn Galvizo
///
/// Header file for Asterism class, which retrieves attributes based off of four Stars.

#ifndef HOKU_ASTERISM_H
#define HOKU_ASTERISM_H

#include <algorithm>
#include "mercator.h"
#include "star.h"

/// The mercator class is meant to reduce a dimension off of a star. This class is used to flatten the points in
/// the BSC5 table and as the main data in the BSC5 quadtree.
///
/// @example
/// @code{.cpp}
/// // Project star {1, 1, 1} to a 1000x1000 square (the jist of it).
/// Mercator a(Star(1, 1, 1), 1000);
/// printf("%s", a.str().c_str());
/// @endcode
class Asterism {
  private:
    friend class TestAsterism;
  
  public:
    /// Common alias for an asterism (quad) of stars in 3D.
    using stars = std::array<Star, 4>;
    
    /// Common alias for an asterism (quad) of 2D stars (points).
    using points = std::array<Mercator, 4>;
    
    /// Alias for the 4-vector of two-dimensional star positions.
    using points_cd = std::array<double, 4>;
    
    /// Ensure default constructor is **not** generated.
    Asterism () = delete;
  
  public:
    static points_cd hash (const stars &, const double);
    
    static Star center (const stars &);
  
  private:
    Asterism (const stars &, const double);
    
    bool cd_property_met ();
  
  private:
    /// Point one of the asterism. Points A and B define the local coordinate system. Defaults to (0, 0).
    Mercator a = Mercator::zero();
    
    /// Point two of the asterism. Points A and B define the local coordinate system. Defaults to (0, 0).
    Mercator b = Mercator::zero();
    
    /// Point three of the asterism. Positions of points C and D are used as the hash code. Defaults to (0, 0).
    Mercator c = Mercator::zero();
    
    /// Point four of the asterism. Positions of points C and D are used as the hash code. Defaults to (0, 0).
    Mercator d = Mercator::zero();
    
    /// Point three of the asterism in the AB coordinate system. Defaults to (0, 0).
    Mercator c_prime = Mercator::zero();
    
    /// Point four of the asterism in the AB coordinate system. Defaults to (0, 0).
    Mercator d_prime = Mercator::zero();
};

#endif /* HOKU_ASTERISM_H */
