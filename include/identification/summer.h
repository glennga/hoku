/// @file coin.h
/// @author Glenn Galvizo
///
/// Header file for Summer class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#ifndef HOKU_SUMMER_H
#define HOKU_SUMMER_H

#include "identification/planar-triangle.h"
#include "benchmark/benchmark.h"
#include <iostream>

// TODO: fix documentation below
// TODO: this is named after the summer triangle constellation
/// The summer class is an implementation of the proposed identification procedure for this project. This is one of the
/// five star identification procedures being tested.
///
/// @example
/// @code{.cpp}
/// // Populate a table named "SUMMER_20" in Nibble.db of all distinct asterisms of stars whose angle of separation is
/// // less than 20 degrees of each other. The entries stored are the catalog ID numbers, and the four hash codes.
/// Hoku::generate_triangle_table(20, "COIN_20");
///
/// /* The snippet above should only be run ONCE. The snippet below is run with every different test. */
///
/// // Find all stars around a random star within 7.5 degrees of it. Rotate all stars by same random rotation.
/// std::random_device seed;
/// Benchmark b(15, Star::chance(seed), Rotation::chance(seed));
///
/// // Append 2 extra stars to the data-set above.
/// b.add_extra_light(2);
///
/// Summer::Parameters p;
/// // The minimum number of stars to match the body and inertial is now 7.
/// p.match_minimum = 7;
///
/// // Print all matches (the key here is the 'identify' method).
/// for (const Star &s : Summer::identify(b, p)) {
///     printf("%s", s.str().c_str());
/// }
/// @endcode
class Summer : public Identification {
  public:
    explicit Summer (const Benchmark &input, const Parameters &p);
    
    std::vector<labels_list> experiment_query (const Star::list &s);
    Star::list experiment_first_alignment (const Star::list &candidates, const Star::list &r, const Star::list &b);
    labels_list experiment_reduction ();
    Star::list experiment_alignment ();
    Star::list experiment_crown ();
    
    static int generate_table (double fov, const std::string &table_name);
    
    static const Parameters DEFAULT_PARAMETERS;
  
  public:
    /// Exact number of query stars required.
    static constexpr unsigned int QUERY_STAR_SET_SIZE = 3;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Alias for a pair of catalog IDs (3-element STL array of integers).
    using label_trio = std::array<int, 3>;
    
    /// Alias for a list of catalog ID pairs (STL vector of 3-element arrays of integers).
    using label_list_trio = std::vector<label_trio>;
    
    /// Alias for a quad of stars (4-element STL array of stars).
    using star_quad = std::array<Star, 4>;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    static const star_quad NO_CANDIDATE_QUAD_FOUND;
    static const label_list_trio NO_CANDIDATE_TRIOS_FOUND;
    static const labels_list NO_COMMON_RESTRICTIONS;
    static const Star NO_COMMON_STAR_FOUND;
    
    Star find_common (const label_list_trio &, const label_list_trio &, const label_list_trio &, const labels_list &);
    label_list_trio query_for_trios (double, double);
    star_quad find_candidate_quad (const star_quad &b_f);
    Star::list match_remaining (const Star::list &, const star_quad &, const star_quad &);
};

#endif /* HOKU_SUMMER_H */