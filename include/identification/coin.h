/// @file coin.h
/// @author Glenn Galvizo
///
/// Header file for Coin class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#ifndef HOKU_COIN_H
#define HOKU_COIN_H

#include "identification/planar-triangle.h"
#include "benchmark/benchmark.h"
#include "math/asterism.h"
#include <iostream>

/// The coin class is an implementation of the proposed identification procedure for this project. This is one of the
/// five star identification procedures being tested.
///
/// @example
/// @code{.cpp}
/// // Populate a table named "COIN_20" in Nibble.db of all distinct asterisms of stars whose angle of separation is
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
/// Coin::Parameters p;
/// // The minimum number of stars to match the body and inertial is now 7.
/// p.match_minimum = 7;
///
/// // Print all matches (the key here is the 'identify' method).
/// for (const Star &s : Coin::identify(b, p)) {
///     printf("%s", s.str().c_str());
/// }
/// @endcode
class Coin {
  public:
    /// Defines the query and match operations, user can tweak for custom performance.
    struct Parameters {
        double sigma_a = std::numeric_limits<double>::epsilon() * 10; ///< Area query must be in 3 * sigma_a.
        double sigma_i = std::numeric_limits<double>::epsilon() * 10000; ///< Moment query must be in 3 * sigma_i.
        unsigned int query_expected = 100; ///< Expected number of stars to be found with query. Better to overshoot.
        double match_sigma = 0.00001; ///< Resultant of inertial->body rotation must within 3 * match_sigma of *a* body.
        unsigned int match_minimum = 5; ///< The minimum number of body-inertial matches.
        unsigned int z_max = 1000; ///< Maximum number of comparisons before returning an empty list.
        std::string table_name = "HOKU_20"; ///< Name of the Nibble database table.
    };
    
    /// User should **NOT** be creating instances of Coin manually. Instead, use the static 'identify' function.
    Coin () = delete;
  
  public:
    static Star::list identify (const Benchmark &, const Parameters &, unsigned int &);
    static Star::list identify (const Benchmark &, const Parameters &);
    static int generate_triangle_table (double, const std::string &);

#if !defined ENABLE_IDENTIFICATION_ACCESS && !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Alias for a quad of catalog IDs (4-element STL array of integers).
    using label_quad = std::array<int, 4>;
    
    /// Alias for a trio of catalog IDs (3-element STL array of integers).
    using label_trio = std::array<int, 3>;
    
    /// Alias for a list of trios of catalog IDs (STL vector of 3-element STL arrays of integers).
    using label_list_trio = std::vector<label_trio>;
    
    /// Alias for a list of catalog IDs (STL vector of integers).
    using label_list = std::vector<int>;
    
    /// Alias for a quad of index numbers for the input star list (4-element STL array of integers).
    using index_quad = std::array<int, 4>;
    
    /// The star set we are working with. The catalog ID values are all set to 0 here.
    Star::list input;
    
    /// Current working parameters.
    Parameters parameters;
    
    /// Chomp instance, gives us access to the Nibble database.
    Chomp ch;
    
    /// All stars in 'input' are fov degrees from the focus.
    double fov;

#if !defined ENABLE_IDENTIFICATION_ACCESS && !defined ENABLE_TESTING_ACCESS
  private:
#endif
    Coin (const Benchmark &, const Parameters &);
    
    label_list_trio query_for_trios (double, double);
    Star find_common (const label_list_trio &, const label_list_trio &, const label_list_trio &,
                      const label_list & = {});
    label_quad find_candidate_quad (const index_quad &);
    Star::list find_matches (const Star::list &, const Rotation &);
    Star::list match_remaining (const Star::list &, const index_quad &, const label_quad &);
    
    Rotation trial_attitude_determine (const std::array<Star, 4> &, const std::array<Star, 4> &);
};

#endif /* HOKU_COIN_H */