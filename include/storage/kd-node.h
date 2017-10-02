/// @file kd-node.cpp
/// @author Glenn Galvizo
///
/// Header file for KdNode class, which represents a node and associated functions for the Mercator kd-tree.

#ifndef HOKU_KD_NODE_H
#define HOKU_KD_NODE_H

#include "math/mercator.h"
#include "storage/nibble.h"
#include <memory>
#include <algorithm>

/// The KdNode represents a node for the Mercator kd-tree, a structure for spatial indexing. This structure is
/// currently being used by the Astrometry.net and Pyramid identification methods.
///
/// @example
/// @code{.cpp}
/// // Construct the kd-tree for all stars in BSC5. Project every star to a square of 1000x1000 size.
/// Star::list a = Nibble().all_bsc5_stars();
/// KdNode k_root = KdNode::load_tree(a, 1000);
///
/// // Find all nearby stars that are within 15 degrees of a random star (Star::chance()). Expecting 90 stars.
/// for (const Star &s : k_root.nearby_stars(Star::chance(), 15, 90)) {
///     printf("%s", s.str().c_str());
/// }
/// @endcode
class KdNode : private Mercator {
  private:
    friend class TestKdNode;
    friend class BaseTest;
  
  public:
    static KdNode load_tree (const Star::list &, double);
    
    Star::list nearby_stars (const Star &, double, unsigned int, const Star::list &);
  
  private:
    /// Precision default for '==' method.
    constexpr static double KDNODE_EQUALITY_PRECISION_DEFAULT = 0.000000000001;
    
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
  
  private:
    KdNode (unsigned int, unsigned int, int,  const bounds_set &, list &);
    static void sort_by_dimension (unsigned int, unsigned int, int, list &);
    
    double width_given_angle (double);
    bool does_intersect_quad (const Mercator::quad &) const;
    static void box_query (const Mercator::quad &, const KdNode &, KdNode::list &);
    
    std::string str () const override;
    
    bool operator== (const KdNode &) const;
  
  private:
    /// Minimum values for this node's represented box. Used for box queries.
    bounds b_min = {0, 0};
    
    /// Maximum values for this node's represented box. Used for box queries.
    bounds b_max = {0, 0};
    
    /// Edge to left child node.
    child_edge left_child;
    
    /// Edge to right child node.
    child_edge right_child;
    
    /// To generalize this tree to other data. This is a general identification number (as opposed to just HR values).
    int origin_index = -1;
};

#endif /* HOKU_KD_NODE_H */
