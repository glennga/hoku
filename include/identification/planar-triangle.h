/// @file planar-triangle.h
/// @author Glenn Galvizo
///
/// Header file for PlanarTriangle class, which matches a set of body vectors (stars) to their inertial counter-parts
/// in the database.

#ifndef HOKU_PLANAR_TRIANGLE_H
#define HOKU_PLANAR_TRIANGLE_H

#include "base-triangle.h"

// TODO: Fix the PlanarTriangle documentation.
/// The triangle planar class is an implementation of Crassidis and Cole's Planar Triangle Pattern Recognition
/// Process. This is one of the five star identification procedures being tested.
///
/// @example
/// @code{.cpp}
/// // Populate a table named "PLAN20" in Nibble.db of all distinct trios of stars whose angle of separation is
/// // less than 20 degrees of each. The entries stored are the BSC5 catalog IDs, the planar area between each star,
/// // and the planar polar moment between each star.
/// PlanarTriangle::generate_triangle_table(20, "PLAN20");
///
/// /* The snippet above should only be run ONCE. The snippet below is run with every different test. */
///
/// // Find all stars around a random star within 7.5 degrees of it. Rotate all stars by same random rotation.
/// Benchmark b(15, Star::chance(), Rotation::chance());
///
/// // Append 2 extra stars to the data-set above.
/// b.add_extra_light(2);
///
/// Plane::Parameters p;
/// // The minimum number of stars to match the body and inertial is now 7.
/// p.match_minimum = 7;
///
/// // Print all matches (the key here is the 'identify' method).
/// for (const Star &s : Plane(b, p).identify()) {
///     printf("%s", s.str().c_str());
/// }
/// @endcode
class PlanarTriangle : public BaseTriangle {
  public:
    using BaseTriangle::Parameters;
    static const Parameters DEFAULT_PARAMETERS;
    
    PlanarTriangle (const Benchmark &, const Parameters &);
    
    std::vector<labels_list> query (const Star::list &s);
    labels_list reduce ();
    Star::list align ();
    
    static int generate_table(double fov, const std::string &table_name);
  
  public:
    /// Exact number of query stars required for query experiment.
    static constexpr unsigned int QUERY_STAR_SET_SIZE = 3;
    
#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    std::vector<Star::trio> match_stars (const index_trio &);
};

/// Alias for the PlanarTriangle class. 'Plane' distinguishes the process I am testing here enough from the 4 other
/// methods.
typedef PlanarTriangle Plane;

#endif /* HOKU_PLANAR_TRIANGLE_H */