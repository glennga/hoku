/// @file interior-angle.h
/// @author Glenn Galvizo
///
/// Header file for InteriorAngle class, which matches a set of body vectors (stars) to their inertial counter-parts in
/// the database.

#ifndef HOKU_INTERIOR_ANGLE_H
#define HOKU_INTERIOR_ANGLE_H

#include "benchmark/benchmark.h"
#include "identification.h"

/// @brief Star identification class using interior angles.
///
/// The angle class is an implementation of Liebe's Interior Angle method for alignment determination. This is one of
/// the six star identification procedures being tested.
///
/// @example
/// @code{.cpp}
/// // Find all stars around a random star within 7.5 degrees of it.
/// // Rotate all stars by same random rotation.
/// Benchmark b(15, Star::chance(), Rotation::chance());
///
/// // Append 2 extra stars to the data-set above.
/// b.add_extra_light(2);
///
/// // Determine an identification. 'A' contains the body set with catalog label attached.
/// Star::list a = Interior(b, Interior::DEFAULT_PARAMETERS).identify();
/// for (const Star s : a) { std::cout << s.str(); << std::endl; }
///
/// // Extract an attitude instead of an identification.
/// Rotation q = Interior(b, Interior::DEFAULT_PARAMETERS).align();
/// @endcode
class InteriorAngle : public Identification {
  public:
    explicit InteriorAngle (const Benchmark &input, const Parameters &p);
    
    std::vector<labels_list> query (const Star::list &s);
    labels_list reduce ();
    Star::list identify ();
    
    static int generate_table (INIReader &cf);
    
    static const Parameters DEFAULT_PARAMETERS;
    static const Star::pair NO_CANDIDATE_PAIR_FOUND;
  
  public:
    /// Exact number of query stars required for query experiment.
    static constexpr unsigned int QUERY_STAR_SET_SIZE = 2;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    labels_list query_for_pair (double theta);
    Star::pair find_candidate_pair (const Star &b_a, const Star &b_b);
    Star::list direct_match_test (const Star::list &big_p, const Star::list &r, const Star::list &b);
};

/// Alias for the InteriorAngle class. 'Interior' distinguishes the process I am testing here enough from the 5 other
/// methods.
typedef InteriorAngle Interior;

#endif /* HOKU_INTERIOR_ANGLE_H */