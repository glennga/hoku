/// @file base-triangle.h
/// @author Glenn Galvizo
///
/// Header file for BaseTriangle class, which holds all common methods between the Planar and Spherical triangle
/// methods (they are very similar).

#ifndef HOKU_BASE_TRIANGLE_H
#define HOKU_BASE_TRIANGLE_H

#include "identification/identification.h"
#include "math/trio.h"
#include <deque>
#include <iostream>
#include <iterator>

/// @brief Abstract base class for both triangle star identification methods.
///
/// The base triangle class is a base class for Crassidis and Cole's Planar and Spherical Pattern Recognition Process.
/// These are two of the five star identification procedures being tested.
class BaseTriangle : public Identification {
  public:
    /// Alias for a trio of catalog IDs (3-element STL array of doubles).
    using label_trio = std::array<int, 3>;
  
  public:
    BaseTriangle ();

#if !defined ENABLE_TESTING_ACCESS
  protected:
#endif
    /// Alias for a trio of index numbers for the input star list (3-element STL array of integers).
    using index_trio = std::array<int, 3>;
    
    /// Alias for an area function in the Trio class.
    using area_function = double (*) (const Star &, const Star &, const Star &);
    
    /// Alias for a moment function in the Trio class.
    using moment_function =  double (*) (const Star &, const Star &, const Star &);
    
    /// Return the first element, and deque the first element.
    ///
    /// @tparam T Type of the stack.
    /// @param p Reference to the stack to operate on.
    /// @return The top element of p.
    template <typename T>
    T ptop (std::deque<T> &p) {
        T t = p.front();
        p.pop_front();
        return t;
    }

#if !defined ENABLE_TESTING_ACCESS
  protected:
#endif
    std::vector<label_trio> e_query (double a, double i);
    labels_list e_reduction ();
    Star::list e_alignment ();
    
    static int generate_triangle_table (double fov, const std::string &table_name, area_function compute_area,
                                        moment_function compute_moment);
    std::vector<Star::trio> m_stars (const index_trio &i_b, area_function compute_area, moment_function compute_moment);
    
    static const index_trio STARTING_INDEX_TRIO;
    
    static const std::vector<BaseTriangle::label_trio> NO_CANDIDATE_TRIOS_FOUND;
    static const std::vector<Star::trio> NO_CANDIDATE_STARS_FOUND;
    static const Star::trio NO_CANDIDATE_STAR_SET_FOUND;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Our index series that we pivot with. Set before each pivot call.
    std::deque<int> p;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    void generate_pivot_list (const index_trio &);
    std::vector<label_trio> query_for_trio (double a, double i);
    virtual std::vector<Star::trio> match_stars (const index_trio &) = 0;
    Star::trio pivot (const index_trio &, const std::vector<Star::trio> & = {});
    Star::list singular_alignment (const Star::list &candidates, const Star::trio &r, const Star::trio &b);
    
};

#endif /* HOKU_BASE_TRIANGLE_H */