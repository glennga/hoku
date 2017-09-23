/// @file planar-triangle.h
/// @author Glenn Galvizo
///
/// Header file for PlanarTriangle class, which matches a set of body vectors (stars) to their inertial counter-parts
/// in the database.

#ifndef HOKU_PLANAR_TRIANGLE_H
#define HOKU_PLANAR_TRIANGLE_H

#include "base-triangle.h"
#include "benchmark/benchmark.h"
#include "storage/chomp.h"
#include "storage/quad-node.h"
#include "math/trio.h"
#include <iostream>

/// The triangle planar class is an implementation of Crassidis and Cole's Planar Triangle Pattern Recognition
/// Process. This is one of the five star identification procedures being tested.
///
/// @example
/// @code{.cpp}
/// // Populate a table named "PLAN20" in Nibble.db of all distinct trios of stars whose angle of separation is
/// // less than 20 degrees of each. The entries stored are the HR numbers, the planar area between each star, and the
/// // planar polar moment between each star.
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
class PlanarTriangle : private BaseTriangle {
  private:
    friend class TestPlanarTriangle;
  
  public:
    static Star::list identify (const Benchmark &, const Parameters &, unsigned int &,
                                const std::shared_ptr<QuadNode> & = nullptr);
    static Star::list identify (const Benchmark &, const Parameters &, const std::shared_ptr<QuadNode> & = nullptr);
    static int generate_triangle_table (const double, const std::string &);
    using BaseTriangle::Parameters;
  
  private:
    PlanarTriangle (const Benchmark &, const Parameters &, const std::shared_ptr<QuadNode> & = nullptr);
    std::vector<Trio::stars> match_stars (const index_trio &);
};

/// Alias for the PlanarTriangle class. 'Plane' distinguishes the process I am testing here enough from the 4 other
/// methods.
typedef PlanarTriangle Plane;

#endif /* HOKU_PLANAR_TRIANGLE_H */