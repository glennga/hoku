/// @file spherical-triangle.h
/// @author Glenn Galvizo
///
/// Header file for SphericalTriangle class, which matches a set of body vectors (stars) to their inertial counter-parts
/// in the database.

#ifndef HOKU_SPHERICAL_TRIANGLE_H
#define HOKU_SPHERICAL_TRIANGLE_H

#include "base-triangle.h"
#include "benchmark/benchmark.h"
#include "storage/chomp.h"
#include "storage/quad-node.h"
#include "math/trio.h"
#include <iostream>

/// The triangle spherical class is an implementation of Crassidis and Cole's Spherical Triangle Pattern Recognition
/// Process. This is one of the five star identification procedures being tested.
///
/// @example
/// @code{.cpp}
/// // Populate a table named "SPHERE20" in Nibble.db of all distinct trios of stars whose angle of separation is
/// // less than 20 degrees of each. The entries stored are the HR numbers, the spherical area between each star, and
/// // the spherical polar moment between each star.
/// SphericalTriangle::generate_triangle_table(20, "SPHERE20");
///
/// /* The snippet above should only be run ONCE. The snippet below is run with every different test. */
///
/// // Find all stars around a random star within 7.5 degrees of it. Rotate all stars by same random rotation.
/// Benchmark b(15, Star::chance(), Rotation::chance());
///
/// // Append 2 extra stars to the data-set above.
/// b.add_extra_light(2);
///
/// Sphere::Parameters p;
/// // The minimum number of stars to match the body and inertial is now 7.
/// p.match_minimum = 7;
///
/// // Print all matches (the key here is the 'identify' method).
/// for (const Star &s : Sphere(b, p).identify()) {
///     printf("%s", s.str().c_str());
/// }
/// @endcode
class SphericalTriangle : private BaseTriangle {
  private:
    friend class TestSphericalTriangle;
  
  public:
    static int generate_triangle_table (double, unsigned int, const std::string &);
    static Star::list identify (const Benchmark &, const Parameters &, unsigned int &,
                                const std::shared_ptr<QuadNode> & = nullptr);
    static Star::list identify (const Benchmark &, const Parameters &, const std::shared_ptr<QuadNode> & = nullptr);
    using BaseTriangle::Parameters;
  
  private:
    SphericalTriangle (const Benchmark &, const Parameters &, const std::shared_ptr<QuadNode> & = nullptr);
    std::vector<Trio::stars> match_stars (const index_trio &);
};

/// Alias for the SphericalTriangle class. 'Sphere' distinguishes the process I am testing here enough from the 4 other
/// methods.
typedef SphericalTriangle Sphere;

#endif /* HOKU_SPHERICAL_TRIANGLE_H */