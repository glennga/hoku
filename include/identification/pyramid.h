/// @file pyramid.h
/// @author Glenn Galvizo
///
/// Header file for Pyramid class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#ifndef HOKU_PYRAMID_H
#define HOKU_PYRAMID_H

#include "identification/angle.h"
#include <iostream>

// TODO: fix these docs
/// The pyramid class is an implementation of the identification portion of the Pyramid
/// process. This is one of the five star identification procedures being tested.
///
/// @example
/// @code{.cpp}
/// // Populate a table named "SEP20" in Nibble.db of all distinct pair of stars whose angle of separation is
/// // less than 20 degrees of each. The entries stored are the catalog IDs, and the separation angle.
/// Pyramid::generate_sep_table(20, "SEP20");
///
/// /* The snippet above should only be run ONCE. The snippet below is run with every different test. */
///
/// // Find all stars around a random star within 7.5 degrees of it. Rotate all stars by same random rotation.
/// Benchmark b(15, Star::chance(), Rotation::chance());
///
/// // Append 2 extra stars to the data-set above.
/// b.add_extra_light(2);
///
/// // Print all matches (the key here is the 'identify' method).
/// for (const Star &s : Pyramid::identify(b, p)) {
///     printf("%s", s.str().c_str());
/// }
/// @endcode
class Pyramid : public Identification {
  public:
    explicit Pyramid (const Benchmark &input, const Parameters &p);
    
    std::vector<labels_list> experiment_query (const Star::list &s);
    Star::list experiment_first_alignment (const Star::list &candidates, const Star::list &r, const Star::list &b);
    labels_list experiment_reduction ();
    Star::list experiment_alignment ();
    Star::list experiment_crown ();
    
    static int generate_table(double fov, const std::string &table_name);
    
    static const Parameters DEFAULT_PARAMETERS;
  
  public:
    /// Exact number of query stars required.
    static constexpr unsigned int QUERY_STAR_SET_SIZE = 3;
    
#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Alias for a pair of catalog IDs (2-element STL array of integers).
    using label_pair = std::array<int, 2>;

    /// Alias for a list of catalog ID pairs (STL vector of 2-element arrays of integers).
    using label_list_pair = std::vector<label_pair>;

    /// Alias for a quad of stars (4-element STL array of stars).
    using star_quad = std::array<Star, 4>;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    static const Star NO_REFERENCE_FOUND;
    static const star_quad NO_CANDIDATE_QUAD_FOUND;

    Star find_reference (const label_list_pair &, const label_list_pair &, const label_list_pair &);
    label_list_pair query_for_pairs (double);
    star_quad find_candidate_quad (const star_quad &);
    Star::list match_remaining (const Star::list &, const star_quad &, const star_quad &);
};

#endif /* HOKU_PYRAMID_H */