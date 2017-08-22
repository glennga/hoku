/// @file kd-node.cpp
/// @author Glenn Galvizo
///
/// Header file for KdNode class, which represents a node and associated functions for the Mercator kd-tree.

#ifndef HOKU_KD_NODE_H
#define HOKU_KD_NODE_H

#include "mercator.h"
#include "nibble.h"
#include <memory>
#include <algorithm>

/// The KdNode represents a node for the Mercator kd-tree, a structure for spatial indexing. This structure is
/// currently being used by the Astrometry.net and Pyramid identification methods.
///
/// @example
/// @code{.cpp}
/// // Construct the kd-tree for all stars in BSC5. Project every star to a square of 1000x1000 size.
/// KdNode k_root = KdNode::load_tree(Nibble().all_bsc5_stars(), 1000);
///
/// // Find all nearby stars that are within 15 degrees of a random star (Star::chance()). Expecting 90 stars.
/// for (const Star &s : k_root.nearby_stars(Star::chance(), 15, 90)) {
///     printf("%s", s.str().c_str());
/// }
/// @endcode
class KdNode : public Mercator {
  private:
    friend class TestKdNode;
    friend class BaseTest; // Friend to BaseTest for access to '==' operator.
  
  public:
    Star::list nearby_stars (const Star &, const double, const unsigned int);
    
    static KdNode load_tree (const Star::list &, const double);
  
  private:
    /// Precision default for '==' method.
    constexpr static double KDNODE_EQUALITY_PRECISION_DEFAULT = 0.000000000001;
    
    /// Alias for a list of KdNodes (STL vector of KdNodes).
    using list = std::vector<KdNode>;
    
    /// Alias for edges to nodes (there should only be a left and right).
    using child_edges = std::array<std::shared_ptr<KdNode>, 2>;
  
  private:
    KdNode (const Star &, const double, const int = -1);
    KdNode (const double, const double, const int = -1);
    
    static KdNode root (const double);
    static KdNode dead_child ();
    
    std::string str (const bool = false) const;
    
    bool operator== (const KdNode &) const;
    
    static void shuffle (list &);
    
    static KdNode populate_list (const unsigned int, const std::array<double, 4> &, const KdNode::list &);
    static std::array<double, 4> find_bounds (const unsigned int, const std::array<double, 4> &, const KdNode::list &);
    
    bool is_terminal_branch ();
    bool is_dead_child (const int) const;
    static child_edges no_children ();
    
    double width_given_angle (const double);
    bool bounds_cross_bounds (const KdNode &, const int) const;
    
    static KdNode branch (const KdNode &, const child_edges & = no_children());
    KdNode to_child (const int c) const;
    
    static KdNode::list reduce_using_bounds (const KdNode::list &, const std::array<double, 4> &);
    bool within_quadrant (const KdNode &, const double) const;
    
    Star::list query_kd_tree (const KdNode &, const double, const KdNode &, Star::list &);
  
  private:
    /// Pointer to list the tree was constructed out of. Only the root should have a non-null value.
    std::shared_ptr<Star::list> origin_list = nullptr;
    
    /// Children of the node itself. Defaults to having no children.
    KdNode::child_edges children = no_children();
    
    /// Boundaries for root and branch nodes. Defaults to {{0, 0, 0, 0}} (suggests leaf node).
    std::array<std::array<double, 4>, 2> bounds = {{0, 0, 0, 0}};
    
    /// Dimension of split axis. Defaults to -1 (suggests leaf node), but non-terminal nodes should be either 0 or 1.
    int axis = -1;
    
    /// To generalize this tree to other data. This is a general identification number (as opposed to just HR values).
    unsigned int origin_index = 0;
};

#endif /* HOKU_KD_NODE_H */
