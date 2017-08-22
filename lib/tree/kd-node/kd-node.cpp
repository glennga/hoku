/// @file kd-node.cpp
/// @author Glenn Galvizo
///
/// Source file for KdNode class, which represents a node and associated functions for the Mercator kd-tree.

#include "kd-node.h"

/// Sets the X and Y coordinates of the node, and the axis of the node directly.
///
/// @param x X coordinate of the node to set.
/// @param y Y coordinate of the node to set.
/// @param axis Split dimension of the node (domain: 0 or 1, -1 otherwise).
KdNode::KdNode (const double x, const double y, const int axis) {
    this->axis = (axis == 0 || axis == 1) ? axis : -1;
    this->x = x, this->y = y;
}

/// Constructor for projecting a star. Unlike the Mercator base constructor, this adds the split dimension as well.
///
/// @param s Star to project.
/// @param w_n Width to project star with.
/// @param axis Split dimension of the node (domain: 0 or 1, -1 otherwise).
KdNode::KdNode (const Star &s, const double w_n, const int axis) {
    this->axis = (axis == 0 || axis == 1) ? axis : -1;
    project_star(s, w_n);
}

/// Determine if the two KdNode's **components** are within KdNode_EQUALITY_PRECISION_DEFAULT units of each other. We
///
/// @param q KdNode to check against the current.
/// @return True if all components are the same. False otherwise.
bool KdNode::operator== (const KdNode &q) const {
    const double E = KDNODE_EQUALITY_PRECISION_DEFAULT;
    bool is_equal_bounds = true;
    
    for (int i = 0; i < 3; i++) {
        is_equal_bounds *= fabs(bounds[0][i] - q.bounds[0][i]) < E;
        is_equal_bounds *= fabs(bounds[1][i] - q.bounds[1][i]) < E;
    }
    
    return fabs(x - q.x) < E && fabs(y - q.y) < E && fabs(axis - q.axis) < E && fabs(w_n - q.w_n) < E
           && is_equal_bounds;
}

/// Return all components in the current point as a string object. Has flags to print boundaries. Off by default.
///
/// @param b Flag to output bounds or not.
/// @return String of components in form of (x:y:w_n:axis:bounds:hr) if b is true. (x:y:w_n:axis:hr) otherwise.
std::string KdNode::str (const bool b) const {
    std::stringstream components;
    
    // Need to use stream here to set precision.
    components << std::setprecision(16) << std::fixed << "(" << x << ":" << y << ":" << w_n << ":" << axis << ":";
    if (b) {
        components << bounds[0][0] << ":" << bounds[0][1] << ":" << bounds[0][2] << ":" << bounds[0][3] << ":"
                   << bounds[1][0] << ":" << bounds[1][1] << ":" << bounds[1][2] << ":" << bounds[1][3] << ":";
    }
    components << hr << ")";
    
    return components.str();
}

/// Define a root node at the coordinates (0, 0). The split dimension stars with x (dimension 0), and the left/right
/// boundaries are defined as such:
/// @code{.cpp}
/// bounds[0][0] = Left X Minimum, bounds[0][1] = Left Y Minimum
/// bounds[0][2] = Left X Maximum, bounds[0][3] = Left Y Maximum
/// bounds[1][0] = Right X Minimum, bounds[1][1] = Right Y Minimum
/// bounds[1][2] = Right X Maximum, bounds[1][3] = Right Y Maximum
///
///   +---------------(02,03)---------(12,13)
///   |                  |                  |
///   |                  |                  |
///   |                  |                  |
///   |                  |                  |
/// (00, 01)---------(10, 11)---------------+
/// @endcode
///
/// @param w_n Projection width of point. Refers to the projection width of all other stars used to build the tree.
/// @return KdNode with coordinates (0, 0) and axis = 0.
KdNode KdNode::root (const double w_n) {
    KdNode q(0, 0, 0);
    
    // Split dimension is X, split the projected square into two halves by the dimension 0.
    q.bounds[0] = {-w_n / 2.0, -w_n / 2.0, 0, w_n / 2.0};
    q.bounds[1] = {0, -w_n / 2.0, w_n / 2.0, w_n / 2.0};
    q.w_n = w_n;
    
    return q;
}

///
///
/// @return
KdNode KdNode::dead_child () {
    KdNode q(0, 0, -1);
    
    
    
}


/// Returns the condition in which a node as no children (i.e. all children pointers are null).
///
/// @return 2 element STL array of null pointers.
KdNode::child_edges KdNode::no_children () {
    return {nullptr, nullptr};
}

/// **Roughly** determine the width of a box given an angle in degrees.
///
/// @param theta Angle to determine width of box from.
/// @return The width the given angle roughly translates to.
double KdNode::width_given_angle (const double theta) {
    return (theta / 360.0) * w_n;
}

/// @param p
/// Shuffle the given set of nodes. Uses C++11 random library.
///
/// @param t List of nodes to shuffle.
void KdNode::shuffle (KdNode::list &t) {
    // Need to keep random device static to avoid starting with same seed.
    static std::random_device seed;
    static std::mt19937_64 mersenne_twister(seed());
    
    std::shuffle(t.begin(), t.end(), mersenne_twister);
}

/// Create a new branch based of the current node. Attach the given children and return this node.
///
/// @param children Children to attach to the new node.
/// @return KdNode with children attached.
KdNode KdNode::branch (const KdNode &c, const child_edges &children) {
    KdNode m = c;
    m.children = children;
    
    return m;
}

/// Remove all stars in the given list that aren't within the box defined by "bounds".
///
/// @param t KdNode list to reduce.
/// @param bounds Boundary set to reduce with.
/// @return All stars in t that are around the current star's boundaries.
KdNode::list KdNode::reduce_using_bounds (const KdNode::list &t, const std::array<double, 4> &bounds) {
    KdNode::list t_within;
    t_within.reserve(t.size());
    
    for (const KdNode &p : t) {
        // The working node is placed in the right list (strictly less than).
        if (p[0] >= bounds[0] && p[0] < bounds[2] && p[1] >= bounds[1] && p[1] < bounds[3]) {
            t_within.push_back(p);
        }
    }
    
    return t_within;
}

// Determine if the current node is within the current boundary box defined by q and w_s.
///
/// @param q The boundary box that the current point must reside in.
/// @param w_s Size of one dimension of the box. Area of box = w_s * w_s.
/// @return True if the current point resides in q's boundary box. False otherwise.
bool KdNode::within_quadrant (const KdNode &q, const double w_s) const {
    return this->is_within_bounds(q.find_corners(w_s));
}

/// Determine if the child "c" exists.
///
/// @param c Number of the current node's child to check.
/// @return True if child "c" does not exist. False otherwise.
bool KdNode::is_dead_child (const int c) const {
    return (c == 0 || c == 1) ? children[c] == nullptr : false;
}

/// Return child "c" of the current node. If no child exists at "c", return a node with w_n = -1.
///
/// @param c Number of the current node's children to return.
/// @return Node to child number c.
KdNode KdNode::to_child (const int c) const {
    return !is_dead_child(c) ? *children[c] : root(-1);
}

/// Determine if the current node is a terminal **branch**. This is not a leaf test! This means that there exists one
/// child of the current node who has the properties of a child (axis is -1, bounds[0] is {0, 0, 0, 0).
///
/// @return True if the current node is terminal. False otherwise.
bool KdNode::is_terminal_branch () {
    bool is_child_boundless = true;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            is_child_boundless *= to_child(0).bounds[i][j] == 0;
            is_child_boundless *= to_child(1).bounds[i][j] == 0;
        }
    }
    
    return is_child_boundless && (to_child(0).axis == -1 || to_child(1).axis == -1);
}

/// Determine if the bounds of the current node intersects the given node's bounds. Conditions are graphically
/// represented in link (the direction of Y is opposite in the link): https://silentmatt.com/rectangle-intersection/
///
/// @code{.cpp}
/// double ax1 = this->bounds[i][0];
/// double ax2 = this->bounds[i][2];
/// double ay1 = this->bounds[i][3];
/// double ay2 = this->bounds[i][1];
///
/// double bx1 = q.bounds[i][0];
/// double bx2 = q.bounds[i][2];
/// double by1 = q.bounds[i][3];
/// double by2 = q.bounds[i][1];
/// @endcode
///
/// @param q Node to determine intersection of.
/// @param i Which set of boundaries to use.
/// @return True if this intersects quadrant Q. False otherwise.
bool KdNode::bounds_cross_bounds (const KdNode &q, const int i) const {
    return this->bounds[i][0] < q.bounds[i][2] && this->bounds[i][2] > q.bounds[i][0]
           && this->bounds[i][3] < q.bounds[i][1] && this->bounds[i][1] > q.bounds[i][3];
}

// TODO: fix this SHITTT
/// Determine the child branch node given a set of boundaries. To generate balanced trees, we take the median of the
/// **opposite** split axis for all nodes that fit in the bounds.  We then define new boundaries for our branch node,
/// and clear the coordinates (property of all non-leaf nodes is to have the coordinates (0, 0)).
///
/// @param axis Current working axis.
/// @param bounds Boundaries of the current working set.
/// @param t List of points to build tree from.
/// @return If reduced list is empty, return a null root node. Otherwise, a branched node from c_i, containing
/// updated boundaries and axis.
std::array<double, 4> KdNode::find_bounds (const unsigned int axis, const std::array<double, 4> &bounds,
                                           const KdNode::list &t) {
    KdNode::list t_s = KdNode::reduce_using_bounds(t, bounds);
    
    // Do not procedure if there exists no t_s.
    if (t_s.size() == 0) {
        return {0, 0, 0, 0};
    }
    
    // Sort the list by the dimension given by C.
    std::sort(t_s.begin(), t_s.end(), [axis] (const KdNode &m_1, const KdNode &m_2) -> bool {
        return m_1[axis] < m_2[axis];
    });
    
    // Find the median element. If list is even, get the lower element.
    KdNode median = t_s[(t_s.size() % 2 == 1) ? (floor(t_s.size() / 2.0)) : t_s.size() / 2.0 - 1];
    
    if (!axis) {
        // Current axis is 0, define new split axis as 1 and split by Y.
        median.bounds[0] = {bounds[0], bounds[1], bounds[2], median[1]};
        median.bounds[1] = {bounds[0], median[1], bounds[2], bounds[3]};
    }
    else {
        // Current axis is 1, define new split axis as 0 and split by X.
        median.axis = 0;
        median.bounds[0] = {bounds[0], bounds[1], median[0], bounds[3]};
        median.bounds[1] = {median[0], bounds[1], bounds[2], bounds[3]};
    }
    
    return median.bounds[axis];
}

/// Recursively populate a kd-node, built out of KdNode structures. Only external nodes represent actual stars.
///
/// @param axis Current working axis.
/// @param bounds Bounds to reduce with.
/// @param t Stars (projected to Mercator as KdNodes) to build tree with.
KdNode KdNode::populate_list (const unsigned int axis, const std::array<double, 4> &bounds, const KdNode::list &t) {
    
    
    child_edges local_children = KdNode::no_children();
    for (int i = 0; i < 2; i++) {
        KdNode::list t_s = KdNode::reduce_using_bounds(t, bounds);
        
        // Base case: With only one node in the list, we have our child.
        if (t_s.size() == 1) {
            local_children[i] = std::make_shared<KdNode>(t_s[i]);
        }
        else if (t_s.size() == 2) {
            // Base case: With two nodes in the list, create branch here and attach children.
            KdNode child = KdNode(0, 0, (axis == 0) ? 1 : 0);
            
            // Insert smaller element according to new axis to left leaf.
            child.children[t_s[0][child.axis] >= t_s[1][child.axis]] = std::make_shared<KdNode>(t_s[0]);
            child.children[t_s[0][child.axis] < t_s[1][child.axis]] = std::make_shared<KdNode>(t_s[1]);
            
            if (child.axis) {
                child.bounds[0] = {bounds[0], bounds[1], bounds[2], (*child.children[0])[child.axis]};
                child.bounds[1] = {bounds[0], (*child.children[0])[child.axis], bounds[2], bounds[3]};
            }
            else {
                child.bounds[0] = {bounds[0], bounds[1], (*child.children[0])[child.axis], bounds[3]};
                child.bounds[1] = {(*child.children[0])[child.axis], bounds[1], bounds[2], bounds[3]};
            }
            
            // Attach the local child.
            local_children[i] = std::make_shared<KdNode>(child);
        }
        else if (t_s.size() > 2) {
            // Otherwise, find median of list and recurse with a sorted, divided list.
            KdNode m = KdNode::populate_list((axis == 0) ? 1 : 0, find_bounds(axis, bounds, t), t);
            local_children[i] = std::make_shared<KdNode>(m);
        }
    }
    
    KdNode m(0, 0, axis);
    
    // Attach leaves to parent node.
    return KdNode::branch(m, local_children);
}

/// Public wrapper method for partition_list. Return the root node and keep the tree in RAM.
///
/// @param v Star list to use for the kd-tree.
/// @param w_n Projection width to use for all stars in BSC5.
/// @return The root node of the kd-tree.
KdNode KdNode::load_tree (const Star::list &v, const double w_n) {
    KdNode r = KdNode::root(w_n);
    KdNode::list projected;
    
    // Find the Mercator projection for all stars in V.
    projected.reserve(v.size());
    for (unsigned int i = 0; i < v.size(); i++) {
        // From full start to finish: (ra, dec) -> <i, j, k> -> (r, lat, lon) -> (x, y).
        KdNode m = KdNode(v[i], w_n, 0);
        
        // We store the index as opposed to just the HR value, for general use of the tree.
        m.origin_index = i;
        projected.push_back(m);
    }
    
    // Place the list on the heap. We will require this for reverse searches.
    r.origin_list = std::make_shared<Star::list>(v);
    
    // Shuffle the Mercator projection. Expected height of tree becomes O(lg n).
    KdNode::shuffle(projected);
    
    // Populate the tree. The root is the center of projection.
    return populate_list(0, {-500, -500, 500, 500}, projected);
}

// TODO: fix this entire function
/// Given a parent node, explore every child for leaf nodes that are within the boundary box.
///
/// @param focus The star we are searching for.
/// @param w_s Current width of the search.
/// @param parent Parent node of the current search.
/// @param t Current stars that are within the search box.
/// @return List of stars that fit inside the given search box.
Star::list KdNode::query_kd_tree (const KdNode &focus, const double w_s, const KdNode &parent, Star::list &t) {
    for (int i = 0; i < 2; i++) {
        // if the focus search intersects the current boundary, continue
        // otherwise stop
        
        
        
        KdNode working = parent.to_child(i);
        
        // Do not attempt if there is no child.
        if (parent.is_dead_child(i)) {
            break;
        }
        
        // Current child is within the boundary box and is not terminal. Traverse down.
        if (working.within_quadrant(focus, w_s) && !working.is_terminal_branch()) {
            query_kd_tree(focus, w_s, working, t);
        }
        else if (working.is_terminal_branch()) {
            // Child is within the boundary box and holds leaf nodes. Append the leaves.
            for (int j = 0; j < 2; j++) {
                // Do not append dead children. Search for stars using HR number.
                if (!working.is_dead_child(j)) {
                    t.push_back((*this->origin_list)[working.to_child(j).origin_index]);
                }
            }
        }
        else {
            // Move to next node.
        }
    }
    
    return t;
}

/// Wrapper method for query_kd_tree. Query the kd-node for nearby stars to focus using the given fov. In theory,
/// this is a O(lgn) operation as opposed to the regular nearby_stars growth of O(n).
///
/// The node this is operating on **MUST** be the root node itself.
///
/// @param q Star to search around.
/// @param fov Limit a star must be separated from the focus by.
/// @param expected Expected number of stars around the focus. Better to overshoot.
/// @return Array with all nearby stars.
Star::list KdNode::nearby_stars (const Star &q, const double fov, const unsigned int expected) {
    double search_width = width_given_angle(fov);
    Star::list nearby;
    
    // Operating node MUST be the root, with coordinates at (0, 0). If not, return list with one (0, 0, 0) star.
    if (this->x != 0 || this->y != 0 || this->axis != 0 || this->origin_list == nullptr) {
        return {Star::zero()};
    }
    
    nearby.reserve(expected);
    return query_kd_tree(KdNode(q, this->w_n, 0), search_width, *this, nearby);
}