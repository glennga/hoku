/// @file kd-node.cpp
/// @author Glenn Galvizo
///
/// Header file for KdNode class, which represents a node and associated functions for the Mercator kd-tree.

#ifndef HOKU_KD_NODE_H
#define HOKU_KD_NODE_H

#include <memory>

#include "math/mercator.h"

/// @brief Class for spatial indexing of a list of stars. Typically smaller than QuadNode.
///
/// The KdNode represents a node for the Mercator KD-tree, a structure for spatial indexing.
///
/// @example
/// @code{.cpp}
/// // Construct the kd-tree for all stars in the catalog. Project every star to a square of 1000x1000 size.
/// Star::list a = Chomp().hip_as_list();
/// KdNode k_root = KdNode::load_tree(a, 1000);
///
/// // Find all nearby stars that are within 15 degrees of a random star (Star::chance()). Expecting 90 stars.
/// for (const Star &s : k_root.nearby_stars(Star::chance(), 15, 90)) {
///     std::cout << s.str() << std::endl;
/// }
/// @endcode
class KdNode : public Mercator {
  public:
    static KdNode load_tree (const Star::list &, double);
    Star::list nearby_stars (const Star &, double, unsigned int, const Star::list &);
    
    /// For the origin_index property. This suggests that an origin has not been set (or is a median point).
    static constexpr int NO_ORIGIN = -1;
    
    /// For the label property. This suggests that a given point is the root of the tree.
    static constexpr int ROOT_LABEL = -1;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Alias for edge to nodes (there should only be a left and right).
    using child_edge = std::shared_ptr<KdNode>;
    
    /// Alias for a list of KdNodes (STL vector of KdNodes).
    using list = std::vector<KdNode>;
    
    /// Alias for the max and min bounds (2D STL array of doubles).
    using bounds = std::array<double, 2>;
    
    /// Alias for the set of bounds (2D STL array of two 2D STL arrays of doubles).
    using bounds_set = std::array<bounds, 2>;
    
    // Inherit Mercator's star projection constructor.
    KdNode (const Star &s, const double w_n) : Mercator(s, w_n) {
    };
    
    /// Default precision for node component comparisons.
    static constexpr double EQUALITY_PRECISION_DEFAULT = 0.000000000001;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    KdNode (unsigned int, unsigned int, int, const bounds_set &, list &);
    static void sort_by_dimension (unsigned int, unsigned int, int, list &);
    
    double width_given_angle (double);
    bool does_intersect_quad (const Mercator::quad &) const;
    static void box_query (const Mercator::quad &, const KdNode &, KdNode::list &);
    
    std::string str () const override;
    bool operator== (const KdNode &) const;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Minimum values for this node's represented box. Used for box queries.
    bounds b_min = {0, 0};
    
    /// Maximum values for this node's represented box. Used for box queries.
    bounds b_max = {0, 0};
    
    /// Edge to left child node.
    child_edge left_child;
    
    /// Edge to right child node.
    child_edge right_child;
    
    /// To generalize this tree to other data. This is a general ID number (as opposed to just catalog IDs).
    int origin_index = -1;
};

#endif /* HOKU_KD_NODE_H */
