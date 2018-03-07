/// @file composite-pyramid.h
/// @author Glenn Galvizo
///
/// Header file for CompositePyramid class, which matches a set of body vectors (stars) to their inertial
/// counter-parts in the database.

#ifndef HOKU_COMPOSITE_PYRAMID_H
#define HOKU_COMPOSITE_PYRAMID_H

#include "benchmark/benchmark.h"
#include "identification/pyramid.h"

/// @brief Star identification class using pyramids and planar triangles.
///
/// The composite pyramid class is an implementation of Tololei's Composite Pyramid method for identification. This is
/// one of the six star identification procedures being tested.
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
/// Star::list a = Composite(b, Composite::DEFAULT_PARAMETERS).identify();
/// for (const Star &s : a) { std::cout << s.str(); << std::endl; }
///
/// // Extract an attitude instead of an identification.
/// Rotation q = Composite(b, Composite::DEFAULT_PARAMETERS).align();
/// @endcode
class CompositePyramid : public Pyramid {
  public:
    CompositePyramid (const Benchmark &input, const Parameters &p);
    
    std::vector<labels_list> query (const Star::list &s) override;
    static int generate_table (INIReader &cf);
    
    static const Parameters DEFAULT_PARAMETERS;
  
  public:
    /// Exact number of query stars required for query experiment.
    static constexpr unsigned int QUERY_STAR_SET_SIZE = 3;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    labels_list_list query_for_trios (double, double);
    bool verification (const  Star::trio &r, const  Star::trio &b) override;
    Star::trio find_catalog_stars (const Star::trio &) override;
};

/// Alias for the CompositePyramid class. 'Composite' distinguishes the process I am testing here enough from the 5
/// other methods.
typedef CompositePyramid Composite;

#endif /* HOKU_COMPOSITE_PYRAMID_H */