/// @file pyramid.h
/// @author Glenn Galvizo
///
/// Header file for Pyramid class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#ifndef HOKU_PYRAMID_H
#define HOKU_PYRAMID_H

#include "identification/angle.h"
#include "storage/chomp.h"
#include <iostream>

/// The pyramid class is an implementation of the identification portion of the Pyramid
/// process. This is one of the five star identification procedures being tested.
///
/// @example
/// @code{.cpp}
/// // Populate a table named "SEP20" in Nibble.db of all distinct pair of stars whose angle of separation is
/// // less than 20 degrees of each. The entries stored are the HR numbers, and the separation angle.
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
class Pyramid {
  public:
    /// Defines the query and match operations, user can tweak for custom performance.
    struct Parameters {
        double query_sigma = 0.00000000001; ///< A query must be within 3 * query_sigma of a given search.
        unsigned int query_limit = 5; ///< While performing a basic bound query, limit results by this number.
        double match_sigma = 0.00001; ///< Resultant of inertial->body rotation must within 3 * match_sigma of *a* body.
        std::string table_name = "PYRA_20"; ///< Name of the pyramid table created with 'generate_pyramid_table'.
    };
    
    /// User should **NOT** be creating instances of Pyramid manually. Instead, use the static 'identify' function.
    Pyramid () = delete;
  
  public:
    static Star::list identify (const Benchmark &, const Parameters &, unsigned int &);
    static Star::list identify (const Benchmark &, const Parameters &);
    static int generate_sep_table (double, const std::string &);

#if !defined ENABLE_IDENTIFICATION_ACCESS && !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Alias for a list of Harvard Revised numbers (STL vector of doubles).
    using hr_list = std::vector<int>;
    
    /// Alias for a quad of Harvard Revised numbers (4-element STL array of doubles).
    using hr_quad = std::array<int, 4>;
    
    /// Alias for a list of Harvard Revised number quads (STL vector of 3-element arrays of integers).
    using hr_list_trio = std::vector<hr_quad>;
    
    /// Alias for a pair of Harvard Revised numbers (2-element STL array of doubles).
    using hr_pair = std::array<int, 2>;
    
    /// Alias for a list of Harvard Revised number pairs (STL vector of 2-element arrays of integers).
    using hr_list_pair = std::vector<hr_pair>;
    
    /// Alias for a quad of index numbers for the input star list (4-element STL array of doubles).
    using index_quad = std::array<int, 4>;
    
    /// The star set we are working with. The HR values are all set to 0 here.
    Star::list input;
    
    /// Current working parameters.
    Parameters parameters;
    
    /// Chomp instance. This is where multi-threading 'might' fail, with repeated access to database.
    Chomp ch;
    
    /// All stars in 'input' are fov degrees from the focus.
    double fov;

#if !defined ENABLE_IDENTIFICATION_ACCESS && !defined ENABLE_TESTING_ACCESS
  private:
#endif
    Pyramid (const Benchmark &, const Parameters &);
    
    Star find_reference (const hr_list_pair &, const hr_list_pair &, const hr_list_pair &);
    hr_list_pair query_for_pairs (double);
    Star::list find_matches (const Star::list &, const Rotation &);
    hr_quad find_candidate_quad (const index_quad &);
    Star::list match_remaining (const Star::list &, const index_quad &, const hr_quad &);
};

#endif /* HOKU_PYRAMID_H */