/// @file spherical-triangle.h
/// @author Glenn Galvizo
///
/// Header file for SphericalTriangle class, which matches a set of body vectors (stars) to their inertial counter-parts
/// in the database.

#ifndef HOKU_TRIANGLE_SPHERICAL_H
#define HOKU_TRIANGLE_SPHERICAL_H

#include "benchmark.h"
#include "chomp.h"
#include "quad-node.h"
#include "trio.h"
#include <iostream>

/// The triangle spherical class is an implementation of Crassidis and Cole's Spherical Triangle Pattern Recognition
/// Process. This is one of the five star identification procedures being tested.
///
/// @example
/// @code{.cpp}
/// // Populate a table named "SPHERE20" in Nibble.db of all distinct trios of stars whose angle of separation is
/// // less than 20 degrees of each. The entries stored are the HR numbers, the spherical area between each star, and
/// // the spherical polar moment between each star.
/// SphericalTriangle::generate_triangle_table(20, "SPHERE20");
///
/// /* The snippet above should only be run ONCE. The snippet below is run with every different test. */
///
/// // Find all stars around a random star within 7.5 degrees of it. Rotate all stars by same random rotation.
/// Benchmark b(15, Star::chance(), Rotation::chance());
///
/// // Append 2 extra stars to the data-set above.
/// b.add_extra_light(2);
///
/// Sphere::Parameters p;
/// // The minimum number of stars to match the body and inertial is now 7.
/// p.match_minimum = 7;
///
/// // Print all matches (the key here is the 'identify' method).
/// for (const Star &s : Sphere::identify(b, p)) {
///     printf("%s", s.str().c_str());
/// }
/// @endcode
class SphericalTriangle {
  private:
    friend class TestSphericalTriangle;
  
  public:
    /// Defines the query and match operations, user can tweak for custom performance.
    struct Parameters {
        double sigma_a = 0.00000000001; ///< An area query must be within 3 * sigma_a of a given search.
        double sigma_i = 0.00000000001; ///< A moment query must be within 3 * sigma_i of a given search.
        unsigned int query_expected = 10; ///< Expected number of stars to be found with query. Better to overshoot.
        double match_sigma = 0.00001; ///< Resultant of inertial->body rotation must within 3 * match_sigma of *a* body.
        unsigned int match_minimum = 10; ///< The minimum number of body-inertial matches.
        unsigned int bsc5_quadtree_w = 1000; ///< The size of the square to project all BSC5 stars onto (for quadtree).
        int moment_td_h = 3; ///< Maximum level of recursion to generate polar moment.
        std::string table_name = "SPHERE20"; ///< Name of the Nibble table created with 'generate_triangle_table'.
    };
    
    /// User should **NOT** be creating instances of SphericalTriangle manually. Use the static 'identify' function.
    SphericalTriangle () = delete;
  
  public:
    static Star::list identify (const Benchmark &, const Parameters &);
    static int generate_triangle_table (const int, const std::string &);
  
  private:
    /// Alias for a list of Harvard Revised numbers (STL vector of doubles).
    using hr_list = std::vector<double>;
    
    /// Alias for a trio of Harvard Revised numbers (3-element STL array of doubles).
    using hr_trio = std::array<double, 3>;
    
    /// Alias for a trio of index numbers for the input star list (3-element STL array of doubles).
    using index_trio = std::array<double, 3>;
    
    /// The star set we are working with. The HR values are all set to 0 here.
    Star::list input;
    
    /// Current working parameters.
    Parameters parameters;
    
    /// Chomp instance. This is where multi-threading 'might' fail, with repeated access to database.
    Chomp ch;
    
    /// Quadtree root. Used for finding nearby stars.
    std::shared_ptr<QuadNode> q_root;
    
    /// Current focus of the star set 'input'.
    Star focus;
    
    /// All stars in 'input' are fov degrees from the focus.
    double fov;
  
  private:
    SphericalTriangle (const Benchmark &, const Parameters &);
    
    std::vector<hr_trio> query_for_trio (const double, const double);
    std::vector<Trio::stars> match_stars (const index_trio &);
    Trio::stars pivot (const index_trio &, const std::vector<Trio::stars> & = {{}});
    Star::list find_matches (const Star::list &, const Rotation &);
    Star::list check_assumptions (const Star::list &, const Trio::stars &, const index_trio &);
};

/// Alias for the SphericalTriangle class. 'Sphere' distinguishes the process I am testing here enough from the 4 other
/// methods.
typedef SphericalTriangle Sphere;

#endif /* HOKU_TRIANGLE_SPHERICAL_H */