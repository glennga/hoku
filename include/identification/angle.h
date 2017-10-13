/// @file angle.h
/// @author Glenn Galvizo
///
/// Header file for Angle class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#ifndef HOKU_ANGLE_H
#define HOKU_ANGLE_H

#include "benchmark/benchmark.h"
#include <iostream>

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
class Angle {
  public:
    /// Defines the query and match operations, user can tweak for custom performance.
    struct Parameters {
        double query_sigma = std::numeric_limits<double>::epsilon() * 100; ///< Query must be within 3 * query_sigma.
        unsigned int query_limit = 100; ///< While performing a basic bound query, limit results by this number.
        double match_sigma = 0.00001; ///< Resultant of inertial->body rotation must within 3 * match_sigma of *a* body.
        unsigned int match_minimum = 3; ///< The minimum number of body-inertial matches.
        std::string table_name = "SEP_20"; ///< Name of the Nibble database table created with 'generate_sep_table'.
    };
    
    /// User should **NOT** be creating instances of Angle manually. Instead, use the static 'identify' function.
    Angle () = delete;
  
  public:
    static Star::list identify (const Benchmark &, const Parameters &, unsigned int &);
    static Star::list identify (const Benchmark &, const Parameters &);
    static int generate_sep_table (double, const std::string &);

#if !defined ENABLE_IDENTIFICATION_ACCESS && !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Alias for a pair of catalog IDs (2-element STL array of doubles).
    using label_pair = std::array<int, 2>;
    
    /// The star set we are working with. The catalog ID values are all set to 0 here.
    Star::list input;
    
    /// Current working parameters.
    Parameters parameters;
    
    /// Nibble instance. This is where multi-threading 'might' fail, with repeated access to database.
    Nibble nb;
    
    /// All stars in 'input' are fov degrees from the focus.
    double fov;

#if !defined ENABLE_IDENTIFICATION_ACCESS && !defined ENABLE_TESTING_ACCESS
  private:
#endif
    Angle (const Benchmark &, const Parameters &);
    
    label_pair query_for_pair (double);
    Star::list find_matches (const Star::list &, const Rotation &);
    Star::pair find_candidate_pair (const Star &, const Star &);
    Star::list check_assumptions (const Star::list &, const Star::pair &, const Star::pair &);
    
    static std::vector<label_pair> trial_query (Nibble &, const Star &, const Star &, double);
    Rotation trial_attitude_determine (const Star::list &, const Star::pair &, const Star::pair &);
};

#endif /* HOKU_ANGLE_H */