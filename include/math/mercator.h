/// @file mercator.h
/// @author Glenn Galvizo
///
/// Header file for Mercator class, which represents two-dimensional projections of three-dimensional unit vectors
/// (i.e. Stars).

#ifndef HOKU_MERCATOR_H
#define HOKU_MERCATOR_H

#include "math/star.h"

/// @brief Class to project 3D stars to 2D, using Mercator projections.
///
/// The mercator class is meant to reduce a dimension off of a star. This class is used to flatten the points in
/// the bright stars table to a square and as the main data in the bright stars quadtree. The advantage using this
/// projection over the Gnomonic projections (treating right ascension and declination as it's own axis) is that we
/// have a square to use as opposed to a circle. The quadtree and kd-tree that use this class divide space into
/// several equal parts- If we treat the Gnomonic projection as a circle inscribed in a square, we would end up with
/// a lot of empty space around the edges. By using the entire square, we end up with a shorter and easier to query
/// tree.
///
/// @example
/// @code{.cpp}
/// // Project star {1, 1, 1} to a 1000x1000 square (the jist of it).
/// Mercator a(Star(1, 1, 1), 1000);
/// std::cout << a.str() << std::endl;
/// @endcode
class Mercator {
  public:
    /// Alias for a quartet of Mercator points.
    using quad = std::array<Mercator, 4>;
    
    /// Force default constructor. Default point is (0, 0) with w_n = 0 and hr = 0.
    Mercator () = default;
    
    /// Returned when a user attempts to access an item using the [] operator for n > 1.
    static constexpr double INVALID_ELEMENT_ACCESSED = 0;
    
  public:
    Mercator (const Star &s, double w_n);
    Mercator (double x, double y, double w_n, int label = 0);
    
    static Mercator zero ();
    double operator[] (unsigned int n) const;
    
    Mercator::quad find_corners (double a) const;
    static double distance_between (const Mercator &m_1, const Mercator &m_2);
    int get_label () const;
    
    virtual std::string str () const;
    
#if !defined ENABLE_TESTING_ACCESS
  protected:
#endif
    bool is_within_bounds (const quad &corners) const;

#if !defined ENABLE_TESTING_ACCESS
  protected:
#endif
    /// For the label property. Indicates that the label has not been set.
    static constexpr int NO_LABEL = 0;

    /// X coordinate of the projected point. Default point is (0, 0).
    double x = 0;
    
    /// Y coordinate of the projected point. Default point is (0, 0).
    double y = 0;
    
    /// Width of the map the point is projected onto. Default width is 0.
    double w_n = 0;
    
    /// Catalog ID for the point.
    int label = 0;

#if !defined ENABLE_TESTING_ACCESS
  protected:
#endif
    void project_star (const Star &s, double w_n);
};

#endif /* HOKU_MERCATOR_H */
