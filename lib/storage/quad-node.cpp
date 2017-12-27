/// @file quad-node.cpp
/// @author Glenn Galvizo
///
/// Source file for QuadNode class, which represents a node and associated functions for the Mercator quadtree. This 
/// **cannot** handle the entire Hipparcos catalog.

#include "storage/quad-node.h"

/// Sets the X and Y coordinates of the node, and the width of the quadrant directly.
///
/// @param x X coordinate of the node to set.
/// @param y Y coordinate of the node to set.
/// @param w_i Quadrant width of the node to set.
QuadNode::QuadNode (const double x, const double y, const double w_i) {
    this->x = x, this->y = y, this->w_i = w_i;
}

/// Constructor for projecting a star. Unlike the Mercator base constructor, this adds the w_i as well.
///
/// @param s Star to project.
/// @param w_n Width to project star with.
/// @param w_i Boundary box width of the point.
QuadNode::QuadNode (const Star &s, const double w_n, const double w_i) {
    project_star(s, w_n);
    this->w_i = w_i;
}

/// Determine if the two QuadNode's **components** are within QUALITY_PRECISION_DEFAULT units of each other.
///
/// @param q QuadNode to check against the current.
/// @return True if all components are the same. False otherwise.
bool QuadNode::operator== (const QuadNode &q) const {
    const double E = EQUALITY_PRECISION_DEFAULT;
    return fabs(x - q.x) < E && fabs(y - q.y) < E && fabs(w_i - q.w_i) < E && fabs(w_n - q.w_n) < E;
}

/// Return all components in the current point as a string object.
///
/// @return String of components in form of (x:y:w_n:w_i:hr:is_green).
std::string QuadNode::str () const {
    std::stringstream components;
    
    // Need to use stream here to set precision.
    components << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed << "(";
    components << x << ":" << y << ":" << w_n << ":" << w_i << ":" << label << (is_green ? ":1" : ":0") << ")";
    return components.str();
}

/// Define a root node at the coordinates (0, 0). The quadrant width w_i is the same as the projection width w_n.
///
/// @param w_n Projection width of point. Refers to the projection width of all other stars used to build the tree.
/// @return QuadNode with coordinates (0, 0) and w_n = w_i.
QuadNode QuadNode::root (const double w_n) {
    QuadNode q(0, 0, w_n);
    q.w_n = w_n;
    
    return q;
}

/// Returns the condition in which a node as no children (i.e. all children pointers are null).
///
/// @return 4 element STL array of null pointers.
QuadNode::child_edges QuadNode::no_children () {
    return {nullptr, nullptr, nullptr, nullptr};
}

/// **Roughly** determine the width of a box given an angle in degrees.
///
/// @param theta Angle to determine width of box from.
/// @return The width the given angle roughly translates to.
double QuadNode::width_given_angle (const double theta) {
    return (theta / 360.0) * w_n;
}

/// Branch off from the given node. Construct a new node based off of p and attach the given children to it.
///
/// @param p QuadNode to branch from.
/// @param children Children to attach to the new node.
/// @return QuadNode whose attributes are identical to p, except having the children defined in the parameters.
QuadNode QuadNode::branch (const QuadNode &p, const child_edges &children) {
    QuadNode q = p;
    q.children = children;
    
    return q;
}

/// Find the centers of all four quadrants from the current node. The new boundary box width for the child is half
/// that of the current width (w_i). Pointers to these centers are then returned in the following order: top left ->
/// top right -> bottom left -> bottom right.
///
/// @code{.cpp}
/// 0 -> *find_quadrant_centers[0]
/// 1 -> *find_quadrant_centers[1]
/// 2 -> *find_quadrant_centers[2]
/// 3 -> *find_quadrant_centers[3]
///
/// x-----------+-----------x
/// |-----0-----+-----1-----|
/// |-----------+-----------|
/// |+++++++++++++++++++++++|
/// |-----------+-----------|
/// |-----2-----+-----3-----|
/// x-----------+-----------x
/// @endcode
///
/// @return Array of pointers to QuadNode centers.
QuadNode::child_edges QuadNode::find_quadrant_centers () const {
    double a = this->w_i / 4.0, child_width = this->w_i / 2.0;
    return {std::make_shared<QuadNode>(QuadNode(x - a, y + a, child_width)),
        std::make_shared<QuadNode>(QuadNode(x + a, y + a, child_width)),
        std::make_shared<QuadNode>(QuadNode(x - a, y - a, child_width)),
        std::make_shared<QuadNode>(QuadNode(x + a, y - a, child_width))};
}

/// Remove all stars in the given list that aren't within an a*a box around the current star.
///
/// @param t QuadNode list to reduce.
/// @param a Box width around node that all stars in t must be within.
/// @return All stars in t that are around the current star with a box of width a.
QuadNode::list QuadNode::reduce_to_quadrant (const QuadNode::list &t, const double a) {
    QuadNode::list t_within;
    t_within.reserve(t.size());
    
    for (const QuadNode &q : t) {
        // Check if the current point "q" is within the bounds of the "this" star.
        if (q.is_within_bounds(this->find_corners(a))) {
            t_within.push_back(q);
        }
    }
    
    return t_within;
}

/// Determine if the current node is within the current boundary box defined by q.
///
/// @param q The boundary box that the current point must reside in.
/// @return True if the current point resides in q's boundary box. False otherwise.
bool QuadNode::within_quadrant (const QuadNode &q) const {
    return this->is_within_bounds(q.find_corners(q.w_i));
}

/// Determine if the child "c" exists.
///
/// @param c Number of the current node's child to check.
/// @return True if child "c" does not exist. False otherwise.
bool QuadNode::is_dead_child (const int c) const {
    return children[c] == nullptr;
}

/// Return child "c" of the current node. If no child exists at "c", return a node with w_n = ROOT_GLOBAL_WIDTH.
///
/// @param c Number of the current node's children to return.
/// @return Node to child number c.
QuadNode QuadNode::to_child (const int c) const {
    return !is_dead_child(c) ? *children[c] : root(ROOT_GLOBAL_WIDTH);
}

/// Determine if the current node is a terminal **branch**. This is not a leaf test! This means that there exists one
/// child of the current node who is colored green (AKA is a leaf).
///
/// @return True if the current node is terminal. False otherwise.
bool QuadNode::is_terminal_branch () {
    return this->to_child(0).is_green || this->to_child(1).is_green || this->to_child(2).is_green
           || this->to_child(3).is_green;
}

/// Determine if the quadrant of the current node intersects the given quadrant. Conditions are graphically
/// represented in link (the direction of Y is opposite in the link): https://silentmatt.com/rectangle-intersection/
///
/// @param q Quadrant to determine intersection of.
/// @return True if this intersects quadrant Q. False otherwise.
bool QuadNode::quadrant_intersects_quadrant (const QuadNode &q) const {
    double current_half_w = this->w_i / 2.0, q_half_w = q.w_i / 2.0;
    
    return (this->x - current_half_w < q.x + q_half_w) && (this->x + current_half_w > q.x - q_half_w)
           && (this->y + current_half_w > q.y - q_half_w) && (this->y - current_half_w < q.y + q_half_w);
}

/// Recursively populate a quad-node, built out of QuadNode structures. Only external nodes represent actual stars.
///
/// Order of population is as such: upper left (0) -> upper right (1) -> bottom left (2) -> bottom right (3).
///
/// @code{.cpp}
/// x-----------+-----------x
/// |-----0-----+-----1-----|
/// |-----------+-----------|
/// |+++++++++++++++++++++++|
/// |-----------+-----------|
/// |-----2-----+-----3-----|
/// x-----------+-----------x
/// @endcode
///
/// @param c Parent node, holds center of represented quadrant.
/// @param w_i Size of the quadrant.
/// @param t Stars (projected to Mercator as QuadNodes) that fit within the given quadrant.
/// @return The root node of the quad tree.
QuadNode QuadNode::find_quad_leaves (const QuadNode &c, const double w_i, const QuadNode::list &t) {
    QuadNode::child_edges local_children = QuadNode::no_children();
    QuadNode::child_edges centers = c.find_quadrant_centers();
    
    // Populate the tree in pre-order (from upper left corner to bottom right corner).
    for (int i = 0; i < 4; i++) {
        // Find all stars within a given quadrant.
        QuadNode::list in_quadrant = (*centers[i]).reduce_to_quadrant(t, w_i / 2.0);
        
        // Base case: There exists no more than four stars in the quadrant.
        if (in_quadrant.size() <= 4 && !in_quadrant.empty()) {
            QuadNode::child_edges leaves = QuadNode::no_children();
            QuadNode terminal_branch = *centers[i];
            
            // Attach XY values to leaves. Color them green and keep these in the heap.
            for (unsigned int j = 0; j < in_quadrant.size(); j++) {
                in_quadrant[j].is_green = true;
                leaves[j] = std::make_shared<QuadNode>(in_quadrant[j]);
            }
            
            // Attach parent to leaves.
            terminal_branch = QuadNode::branch(*centers[i], leaves);
            local_children[i] = std::make_shared<QuadNode>(terminal_branch);
        }
        else if (in_quadrant.empty()) {
            // If the quadrant is empty, we do nothing. The default child edge to our working node is a null pointer.
        }
        else {
            // Otherwise, recurse with a smaller boundary box.
            QuadNode m = find_quad_leaves(*centers[i], w_i / 2.0, t);
            local_children[i] = std::make_shared<QuadNode>(m);
        }
    }
    
    // Return the local parent node.
    return QuadNode::branch(c, local_children);
}

/// Public wrapper method for find_quad_leaves. Return the root node and keep the tree in RAM.
///
/// @param w_n Projection width to use for all stars in hip table.
/// @param m_bar Maximum apparent magnitude to load.
/// @return The root node of the quadtree.
QuadNode QuadNode::load_tree (const double w_n, const double m_bar) {
    QuadNode r = QuadNode::root(w_n);
    QuadNode::list projected;
    Chomp ch;
    
    // Find the Mercator projection for all bright stars.
    projected.reserve(ch.HIP_TABLE_LENGTH);
    for (const Star &s : ch.hip_as_list()) {
        // From full start to finish: (ra, dec) -> <i, j, k> -> (r, lat, lon) -> (x, y).
        if (s.get_magnitude() < m_bar) {
            projected.push_back(QuadNode(s, w_n, 1));
        }
    }
    
    // Populate the tree. The root is the center of projection.
    return r.find_quad_leaves(r, w_n, projected);
}

/// Given a parent node, explore every child for leaf nodes that are within the boundary box.
///
/// @param ch Chomp object. Gives us access to the Nibble database.
/// @param focus The star we are searching for.
/// @param parent Parent node of the current search.
/// @param t Current stars that are within the search box.
/// @return List of stars that fit inside the given search box.
Star::list QuadNode::query_quadtree (Chomp &ch, const QuadNode &focus, const QuadNode &parent, Star::list &t) {
    for (int i = 0; i < 4; i++) {
        QuadNode working = parent.to_child(i);
        
        // Do not attempt if there is no child.
        if (parent.is_dead_child(i)) {
            break;
        }
        
        // Current child is within the boundary box and is not terminal. Traverse down.
        if (focus.quadrant_intersects_quadrant(working) && !working.is_terminal_branch()) {
            query_quadtree(ch, focus, working, t);
        }
        else if (working.is_terminal_branch()) {
            // Child is within the boundary box and holds leaf nodes. Append the leaves.
            for (int j = 0; j < 4; j++) {
                // Do not append dead children. Search for stars using HR number.
                if (!working.is_dead_child(j)) {
                    t.push_back(ch.query_hip(working.to_child(j).get_label()));
                }
            }
        }
        else {
            // Move to next node. 0 = upper left, 1 = upper right, 2 = bottom left, 3 = bottom right.
        }
    }
    
    return t;
}

/// Wrapper method for query_quadtree. Query the quad-node for nearby stars to focus using the given fov. In theory,
/// this is a O(lgn) operation as opposed to the regular nearby_stars growth of O(n).
///
/// The node this is operating on **MUST** be the root node itself.
///
/// @param q Star to search around.
/// @param fov Limit a star must be separated from the focus by.
/// @param expected Expected number of stars around the focus. Better to overshoot.
/// @return Array with all nearby stars.
Star::list QuadNode::nearby_stars (const Star &q, const double fov, const unsigned int expected) {
    double search_width = width_given_angle(fov);
    Star::list nearby;
    Chomp ch;
    
    // Operating node MUST be the root, with coordinates at (0, 0).
    if (this->x != 0 || this->y != 0) {
        throw "\"nearby_stars\" not operating on root node.";
    }
    
    nearby.reserve(expected);
    return query_quadtree(ch, QuadNode(q, this->w_n, search_width), *this, nearby);
}