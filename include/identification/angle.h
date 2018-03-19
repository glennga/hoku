/// @file angle.h
/// @author Glenn Galvizo
///
/// Header file for Angle class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#ifndef HOKU_ANGLE_H
#define HOKU_ANGLE_H

#include "benchmark/benchmark.h"
#include "identification.h"

/// @brief Star identification class using angles.
///
/// The angle class is an implementation of Gottlieb's Angle method with Tappe's DMT process for alignment
/// determination. This is one of the six star identification procedures being tested.
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
/// Star::list a = Angle(b, Angle::DEFAULT_PARAMETERS).identify();
/// for (const Star s : a) { std::cout << s.str(); << std::endl; }
///
/// // Extract an attitude instead of an identification.
/// Rotation q = Angle(b, Angle::DEFAULT_PARAMETERS).align();
/// @endcode
class Angle : public Identification {
  public:
    explicit Angle (const Benchmark &input, const Parameters &p);
    
    std::vector<labels_list> query (const Star::list &s);
    Star::list reduce ();
    Star::list identify ();
    
    static int generate_table (INIReader &cf, const std::string &id_name = "angle");
    
    static const Parameters DEFAULT_PARAMETERS;
    static const Star::pair NO_CANDIDATE_PAIR_FOUND;
    static const labels_list NO_CANDIDATES_FOUND;
    
    static const unsigned int QUERY_STAR_SET_SIZE;
#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    labels_list query_for_pair (double theta);
    Star::pair find_candidate_pair (const Star &b_i, const Star &b_j);
    Star::list direct_match_test (const Star::list &big_p, const Star::list &r, const Star::list &b);
};

#endif /* HOKU_ANGLE_H */