/// @file quad-node.cpp
/// @author Glenn Galvizo
///
/// Header file for QuadNode class, which represents a node and associated functions for the Mercator quadtree.

#ifndef HOKU_QUAD_NODE_H
#define HOKU_QUAD_NODE_H

#include "math/mercator.h"
#include "storage/chomp.h"
#include <memory>

/// The QuadNode represents a node for the Mercator quadtree, a structure for spatial indexing. This structure is
/// currently being used by the Planar and Spherical Triangle identification methods.
///
/// @example
/// @code{.cpp}
/// // Construct the quadtree. Project every star in BSC5 to a square of 1000x1000 size.
/// QuadNode q_root = QuadNode::load_tree(1000);
///
/// // Find all nearby stars that are within 15 degrees of a random star (Star::chance()). Expecting 90 stars.
/// for (const Star &s : q_root.nearby_stars(Star::chance(), 15, 90)) {
///     printf("%s", s.str().c_str());
/// }
/// @endcode
#if !defined ENABLE_TESTING_ACCESS
class QuadNode : private Mercator {
#else
    class QuadNode : public Mercator {
#endif
  public:
    Star::list nearby_stars (const Star &, double, unsigned int);
    
    static QuadNode load_tree (double);

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Precision default for '==' method.
    constexpr static double QUADNODE_EQUALITY_PRECISION_DEFAULT = 0.000000000001;
    
    /// Alias for a list of QuadNodes (STL vector of QuadNodes).
    using list = std::vector<QuadNode>;
    
    /// Alias for edges to QuadNode's children (4-element STL array of shared pointers).
    using child_edges = std::array<std::shared_ptr<QuadNode>, 4>;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    QuadNode (const Star &, double, double);
    QuadNode (double, double, double);
    static QuadNode root (double);
    
    std::string str () const override;
    
    bool operator== (const QuadNode &) const;
    
    bool is_terminal_branch ();
    bool is_dead_child (int) const;
    static child_edges no_children ();
    
    double width_given_angle (double);
    bool quadrant_intersects_quadrant (const QuadNode &) const;
    
    static QuadNode branch (const QuadNode &, const child_edges & = no_children());
    QuadNode to_child (int c) const;
    
    QuadNode::list reduce_to_quadrant (const QuadNode::list &, double);
    bool within_quadrant (const QuadNode &) const;
    child_edges find_quadrant_centers () const;
    
    QuadNode find_quad_leaves (const QuadNode &, double, const QuadNode::list &);
    Star::list query_quadtree (Chomp &, const QuadNode &, const QuadNode &, Star::list &);

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Children of the node itself. Defaults to having no children.
    QuadNode::child_edges children = no_children();
    
    /// Signals if leaf node or not. Defaults to false.
    bool is_green = false;
    
    /// Width of the quadrant represented.
    double w_i = 1;
};

#endif /* HOKU_QUAD_NODE_H */
