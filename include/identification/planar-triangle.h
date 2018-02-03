/// @file planar-triangle.h
/// @author Glenn Galvizo
///
/// Header file for PlanarTriangle class, which matches a set of body vectors (stars) to their inertial counter-parts
/// in the database.

#ifndef HOKU_PLANAR_TRIANGLE_H
#define HOKU_PLANAR_TRIANGLE_H

#include "base-triangle.h"

/// @brief Star identification class using planar triangles.
///
/// The planar triangle class is an implementation of Cole and Crassidus's Planar Triangle method with Tappe's DMT
/// process for alignment determination. This is one of the six star identification procedures being tested.
///
/// @example
/// @code{.cpp}
/// // Find all stars around a random star within 7.5 degrees of it. Rotate all stars by same random rotation.
/// Benchmark b(15, Star::chance(), Rotation::chance());
///
/// // Append 2 extra stars to the data-set above.
/// b.add_extra_light(2);
///
/// // Determine an identification. 'A' contains the body set with catalog label attached.
/// Star::list a = Plane(b, Plane::DEFAULT_PARAMETERS).identify();
/// for (const Star &s : a) { std::cout << s.str(); << std::endl; }
///
/// // Extract an attitude instead of an identification.
/// Rotation q = Plane(b, Plane::DEFAULT_PARAMETERS).align();
/// @endcode
class PlanarTriangle : public BaseTriangle {
  public:
    using BaseTriangle::Parameters;
    static const Parameters DEFAULT_PARAMETERS;
    
    PlanarTriangle (const Benchmark &, const Parameters &);
    
    std::vector<labels_list> query (const Star::list &s);
    labels_list reduce ();
    Star::list identify ();
    
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