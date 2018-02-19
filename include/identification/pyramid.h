/// @file pyramid.h
/// @author Glenn Galvizo
///
/// Header file for Pyramid class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#ifndef HOKU_PYRAMID_H
#define HOKU_PYRAMID_H

#include "benchmark/benchmark.h"
#include "identification/angle.h"

/// @brief Star identification class using pyramids.
///
/// The pyramid class is an implementation of Motari's Pyramid method for identification. This is one of the
/// six star identification procedures being tested.
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
/// Star::list a = Pyramid(b, Pyramid::DEFAULT_PARAMETERS).identify();
/// for (const Star &s : a) { std::cout << s.str(); << std::endl; }
///
/// // Extract an attitude instead of an identification.
/// Rotation q = Pyramid(b, Pyramid::DEFAULT_PARAMETERS).align();
/// @endcode
class Pyramid : public Identification {
  public:
    Pyramid (const Benchmark &input, const Parameters &p);
    
    virtual std::vector<labels_list> query (const Star::list &s);
    labels_list reduce ();
    Star::list identify ();
    
    static int generate_table (INIReader &cf);
    
    static const Parameters DEFAULT_PARAMETERS;
  
  public:
    /// Exact number of query stars required for query experiment.
    static constexpr unsigned int QUERY_STAR_SET_SIZE = 3;

#if !defined ENABLE_TESTING_ACCESS
  protected:
    /// Alias for a list of catalog ID lists (STL vector of vectors of integers).
    using labels_list_list = std::vector<labels_list>;
#endif

#if !defined ENABLE_TESTING_ACCESS
  protected:
    Star::list common (const labels_list_list &big_r_ab_ell, const labels_list_list &big_r_ac_ell,
                       const Star::list &removed);
    virtual bool verification (const Star::trio &r, const Star::trio &b);
    virtual Star::trio find_catalog_stars (const Star::trio &);
    Star::list identify_as_list (const Star::list &b);
    
    static const Star::trio NO_CONFIDENT_R_FOUND;
#endif

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Alias for a pair of catalog IDs (2-element STL array of integers).
    using label_pair = std::array<int, 2>;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    static const Star::list NO_COMMON_FOUND;
    
    labels_list_list query_for_pairs (double);
};

#endif /* HOKU_PYRAMID_H */