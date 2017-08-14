/// @file quad-node.cpp
/// @author Glenn Galvizo
///
/// Header file for QuadNode class, which represents a node and associated functions for the Mercator quadtree.

#ifndef HOKU_QUAD_NODE_H
#define HOKU_QUAD_NODE_H

#include "mercator.h"
#include "nibble.h"
#include <memory>

/// The QuadNode represents a node for the Mercator quadtree, a structure for spatial indexing. This structure is
/// currently being used by the Planar and Spherical Triangle identification methods.
///
/// @example
/// @code{.cpp}
/// @endcode
class QuadNode : public Mercator {
  private:
    friend class TestQuadNode;
  
  public:
    Star::list nearby_stars (const Star &, const double, const unsigned int);
    
    QuadNode load_tree (const double);
  
  private:
    /// Precision default for '==' method.
    constexpr static double QUADNODE_EQUALITY_PRECISION_DEFAULT = 0.000000000001;
    
    /// Alias for a list of QuadNodes (STL vector of QuadNodes).
    using list = std::vector<QuadNode>;
    
    /// Alias for edges to QuadNode's children (4-element STL array of shared pointers).
    using child_edges = std::array<std::shared_ptr<QuadNode>, 4>;
    
    /// Inherit the Mercator(const Star &, const double) constructor.
    explicit QuadNode (const Star &s, const double w_n) : Mercator(s, w_n) {
    }
  
  private:
    QuadNode (const double, const double, const double);
    static QuadNode root (const double);
    
    bool operator== (const QuadNode &) const;
    
    std::string str() const;
    
    bool is_leaf ();
    bool is_dead_child (const int) const;
    static child_edges no_children ();
    
    double width_given_angle (const double);
    
    static QuadNode branch (const QuadNode &, const child_edges & = no_children());
    QuadNode to_child (const int c) const;
    
    QuadNode::list reduce_to_quadrant (const QuadNode::list &, const double);
    bool within_quadrant (const QuadNode &) const;
    child_edges find_quadrant_centers () const;
    
    QuadNode find_quad_leaves (const QuadNode &, const double, const QuadNode::list &);
    Star::list query_quadtree (Nibble &, const QuadNode &, const QuadNode &, Star::list &);
  
  private:
    /// Children of the node itself. Defaults to having no children.
    QuadNode::child_edges children = no_children();
    
    /// Width of the quadrant represented.
    double w_i = 1;
};

#endif /* HOKU_QUAD_NODE_H */
