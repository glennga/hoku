/// @file base-triangle.h
/// @author Glenn Galvizo
///
/// Header file for BaseTriangle class, which holds all common methods between the Planar and Spherical triangle
/// methods (they are very similar).

#ifndef HOKU_BASE_TRIANGLE_H
#define HOKU_BASE_TRIANGLE_H

#include "benchmark/benchmark.h"
#include "storage/chomp.h"
#include "storage/quad-node.h"
#include "math/trio.h"
#include <iostream>

/// The base triangle class is a base class for Crassidis and Cole's Planar and Spherical Pattern Recognition Process.
/// These are two of the five star identification procedures being tested.
class BaseTriangle {
  public:
    /// Defines the query and match operations, user can tweak for custom performance.
    struct Parameters {
        double sigma_a = std::numeric_limits<double>::epsilon() * 10; ///< Area query must be in 3 * sigma_a.
        double sigma_i = std::numeric_limits<double>::epsilon() * 10000; ///< Moment query must be in 3 * sigma_i.
        unsigned int query_expected = 100; ///< Expected number of stars to be found with query. Better to overshoot.
        double match_sigma = 0.00001; ///< Resultant of inertial->body rotation must within 3 * match_sigma of *a* body.
        unsigned int match_minimum = 4; ///< The minimum number of body-inertial matches.
        unsigned int quadtree_w = 5000; ///< The size of the square to project all BSC5 stars onto (for quadtree).
        int moment_td_h = 3; ///< Maximum level of recursion to generate polar moment (spherical only).
        std::string table_name; ///< Name of the Nibble table created with 'generate_triangle_table'.
    };

#if !defined ENABLE_IDENTIFICATION_ACCESS && !defined ENABLE_TESTING_ACCESS
  protected:
#endif
    /// The star set we are working with. The HR values are all set to 0 here.
    Star::list input;
    
    /// All stars in 'input' are fov degrees from the focus.
    double fov;
    
    /// Current working parameters.
    Parameters parameters;
    
    /// Chomp instance. This is where multi-threading 'might' fail, with repeated access to database.
    Chomp ch;
    
    /// Quadtree root. Used for finding nearby stars.
    std::shared_ptr<QuadNode> q_root;
    
    /// Alias for a list of Harvard Revised numbers (STL vector of doubles).
    using hr_list = std::vector<double>;
    
    /// Alias for a trio of Harvard Revised numbers (3-element STL array of doubles).
    using hr_trio = std::array<double, 3>;
    
    /// Alias for a trio of index numbers for the input star list (3-element STL array of doubles).
    using index_trio = std::array<double, 3>;

#if !defined ENABLE_IDENTIFICATION_ACCESS && !defined ENABLE_TESTING_ACCESS
  protected:
#endif
    Star::list identify_stars (unsigned int &);
    std::vector<hr_trio> query_for_trio (double, double);

#if !defined ENABLE_IDENTIFICATION_ACCESS && !defined ENABLE_TESTING_ACCESS
  private:
#endif
    virtual std::vector<Trio::stars> match_stars (const index_trio &) = 0;
    Trio::stars pivot (const index_trio &, const std::vector<Trio::stars> & = {});
    Star::list rotate_stars (const Star::list &, const Rotation &);
    Star::list check_assumptions (const Star::list &, const Trio::stars &, const index_trio &);
};

#endif /* HOKU_BASE_TRIANGLE_H */