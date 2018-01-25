/// @file angle.h
/// @author Glenn Galvizo
///
/// Header file for Angle class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#ifndef HOKU_ANGLE_H
#define HOKU_ANGLE_H

#include "identification.h"
#include <iostream>

// TODO: Fix the Angle documentation.
/// The angle class is an implementation of the identification portion of the LIS Stellar Attitude Acquisition
/// process. This is one of the five star identification procedures being tested.
///
/// @example
/// @code{.cpp}
/// // Populate a table named "SEP20" in Nibble.db of all distinct pair of stars whose angle of separation is
/// // less than 20 degrees of each. The entries stored are the catalog ID numbers, and the separation angle.
/// Angle::generate_sep_table(20, "SEP20");
///
/// /* The snippet above should only be run ONCE. The snippet below is run with every different test. */
///
/// // Find all stars around a random star within 7.5 degrees of it. Rotate all stars by same random rotation.
/// Benchmark b(15, Star::chance(), Rotation::chance());
///
/// // Append 2 extra stars to the data-set above.
/// b.add_extra_light(2);
///
/// Angle::Parameters p;
/// // The minimum number of stars to match the body and inertial is now 7.
/// p.match_minimum = 7;
///
/// // Print all matches (the key here is the 'identify' method).
/// for (const Star &s : Angle::identify(b, p)) {
///     printf("%s", s.str().c_str());
/// }
/// @endcode
class Angle : public Identification {
  public:
    explicit Angle (const Benchmark &input, const Parameters &p);
    
    std::vector<labels_list> experiment_query (const Star::list &s);
    labels_list experiment_reduction ();
    Star::list experiment_alignment ();
    
    static int generate_table (double fov, const std::string &table_name);
    
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
    Star::list singular_alignment (const Star::list &candidates, const Star::list &r, const Star::list &b);
};

#endif /* HOKU_ANGLE_H */