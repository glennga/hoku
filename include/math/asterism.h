/// @file asterism.h
/// @author Glenn Galvizo
///
/// Header file for Asterism class, which retrieves attributes based off of four Stars.

#ifndef HOKU_ASTERISM_H
#define HOKU_ASTERISM_H

#include "math/mercator.h"
#include "math/star.h"

/// @brief Feature and identification class for a Astrometry.net star identification method.
/// 
/// The asterism class holds all feature and identification computations necessary for the Astrometry.net method. A four
/// element hash code is retrieved from a quad of stars, and the order is determined using `find_order`.
///
/// @example
/// @code{.cpp}
/// // Output the hash code for a quad of random stars.
/// std::cout << Asterism::hash(Star::quad {Star::chance(), Star::chance(), Star::chance(), Star::chance()})
///           << std::endl;
/// @endcode
class Asterism {
  public:
    /// Common alias for an asterism (quad) of 2D stars (points).
    using points = std::array<Mercator, 4>;
    
    /// Alias for the 4-vector of two-dimensional star positions.
    using points_cd = std::array<double, 4>;
    
    /// Ensure default constructor is **not** generated.
    Asterism () = delete;
  
  public:
    static points_cd hash (const Star::quad &s);
    static Star::quad find_order (const Star::quad &s);
    
    static Star center (const Star::quad &s);
    
    static const points_cd MALFORMED_HASH;
    static const Star::quad INDETERMINATE_ORDER;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    explicit Asterism (const Star::quad &s);
    
    void verify_ab_stars ();
    Asterism::points_cd compute_cd_prime ();
    bool cd_property_met ();

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
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
