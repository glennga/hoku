/// @file angle.h
/// @author Glenn Galvizo
///
/// Header file for Angle class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#ifndef HOKU_ANGLE_H
#define HOKU_ANGLE_H

#include "identification.h"
#include <iostream>

// TODO: fix these docs
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
    /// User should **NOT** be creating instances of Angle manually. Instead, use the provided static functions.
    Angle () = delete;
    
    /// Alias for a pair of catalog IDs (2-element STL array of doubles).
    using label_pair = std::array<int, 2>;
  
  public:
    static std::vector<label_pair> experiment_query (Chomp &ch, const Star &s_1, const Star &s_2, double sigma_query);
    static Rotation experiment_alignment (Chomp &ch, const Benchmark &input, const Star::list &candidates,
                                          const Star::pair &r, const Star::pair &b, double sigma_overlay);
    static label_pair experiment_reduction (Chomp &ch, const Star &s_1, const Star &s_2, double sigma_query);
    static Rotation experiment_attitude (const Benchmark &input, const Parameters &p);
    static Star::list experiment_crown (const Benchmark &input, const Parameters &p);
    
    static int generate_ang_table (double fov, const std::string &table_name);
  
  private:
    Angle (const Benchmark &input, const Parameters &p);
    
    label_pair query_for_pair (double theta);
    Star::pair find_candidate_pair (const Star &b_a, const Star &b_b);
    Star::list check_assumptions (const Star::list &candidates, const Star::pair &r, const Star::pair &b);
};

#endif /* HOKU_ANGLE_H */