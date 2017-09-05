/// @file kd-node.cpp
/// @author Glenn Galvizo
///
/// Source file for KdNode class, which represents a node and associated functions for the Mercator kd-tree.

#include "kd-node.h"

/// Recursively populate a kd-tree given the current endpoints of a list "t", the current depth of the tree, and the
/// bounds associated with the working node.
///
/// @param i Beginning index of t for the current sub-problem.
/// @param j End index of t for the current sub-problem.
/// @param depth Current depth of tree. Root starts at d = 0.
/// @param bounds Current bounding box the node represents.
/// @param t Reference to list t. The list is sorted in place.
KdNode::KdNode (const unsigned int i, const unsigned int j, const int depth, const bounds_set &b, list &t) {
    this->left_child = nullptr, this->right_child = nullptr;
    bounds_set b_l = b, b_r = b;
    unsigned int median_index;
    
    // Base case: We have one child in our list t. Our current node is this remainder.
    if (i == j) {
        this->x = t[i][0], this->y = t[i][1], this->w_n = t[i].w_n, this->origin_index = t[i].origin_index;
        return;
    }
    
    // Cycle based on depth. depth % 2 != 1 (even) sort by dimension 0, odd numbers sort by dimension 1.
    sort_by_dimension(i, j, (depth & 1), t);
    
    // Find the median. To create balanced trees, we split by this point. With even indices, select the lower.
    median_index = (j - i) / 2;
    this->x = t[i + median_index][0], this->y = t[i + median_index][1], this->origin_index = -1;
    this->b_min = b[0], this->b_max = b[1];
    
    // Determine the new bounds. For left children, the maxes are redefined. Vice-versa for right children.
    if (!(depth & 1)) {
        b_l[1][0] = this->x, b_r[0][0] = this->x;
    }
    else {
        b_l[1][1] = this->y, b_r[0][1] = this->y;
    }
    
    // Find the children of this node. The median is attached to the left sub-problem.
    this->left_child = std::make_shared<KdNode>(KdNode(i, i + median_index, depth + 1, b_l, t));
    this->right_child = std::make_shared<KdNode>(KdNode(i + median_index + 1, j, depth + 1, b_r, t));
}

/// Determine if the two KdNode's **components** are within KdNode_EQUALITY_PRECISION_DEFAULT units of each other.
/// This does not check if the bounds are equal.
///
/// @param q KdNode to check against the current.
/// @return True if all components are the same. False otherwise.
bool KdNode::operator== (const KdNode &q) const {
    const double E = KDNODE_EQUALITY_PRECISION_DEFAULT;
    return fabs(x - q.x) < E && fabs(y - q.y) < E && fabs(origin_index - q.origin_index) < E && fabs(w_n - q.w_n) < E;
}

/// Return all components in the current point as a string object, except for the bounds components.
///
/// @return String of components in form of (x:y:w_n:origin_index:hr).
std::string KdNode::str () const {
    std::stringstream components;
    
    // Need to use stream here to set precision.
    components << std::setprecision(16) << std::fixed << "(" << x << ":" << y << ":" << w_n << ":";
    components << origin_index << ":" << hr << ")";
    
    return components.str();
}

/// **Roughly** determine the width of a box given an angle in degrees. This MUST be called on the root node.
///
/// @param theta Angle to determine width of box from.
/// @return The width the given angle roughly translates to if this is the root node.
double KdNode::width_given_angle (const double theta) {
    if (!(this->origin_index == -1 && this->hr == -1)) {
        throw "\"width_given_angle\" not operating on the root node.";
    }
    
    return (theta / 360.0) * this->w_n;
}

/// Determine if the current node's bounds intersect the given search box and the working axis of the node itself.
///
/// @param q Search box to determine if the current node's bounds crosses.
/// @return True if the current node intersects q. False otherwise.
bool KdNode::does_intersect_quad (const Mercator::quad &q) const {
    return (b_min[0] < q[3][0] && b_max[0] > q[0][0] && b_max[1] > q[3][1] && b_min[1] < q[0][1]);
    
}

/// Kd-tree range search method. Recursively query a node and determine if overlap exists between the search box and
/// the node's working box.
///
/// @param search Search box centered around the focus point.
/// @param m Current working node.
/// @param r Reference to a return list. This is where all the results are stored.
void KdNode::box_query (const Mercator::quad &search, const KdNode &m, KdNode::list &r) {
    // Base case: m is leaf and within our search, append to our return list.
    if (m.origin_index != -1 && m.is_within_bounds(search)) {
        r.push_back(m);
        return;
    }
    
    // Otherwise, check if our current node's bounds intersect our search box.
    bool intersects_quad = m.does_intersect_quad(search);
    
    // If this is true, recurse with the left and right children.
    if (m.left_child != nullptr && intersects_quad) {
        box_query(search, *(m.left_child), r);
    }
    if (m.right_child != nullptr && intersects_quad) {
        box_query(search, *(m.right_child), r);
    }
}

/// Sort the list "t" given it's indices t_i and t_j. Sort by dimension "axis".
///
/// @param i Beginning index of list t to sort.
/// @param j End index of list t to sort.
/// @param axis Dimension to sort the list by.
/// @param t Main list to sort section from.
void KdNode::sort_by_dimension (const unsigned int i, const unsigned int j, const int axis, list &t) {
    std::sort(t.begin() + i, t.begin() + j + 1, [axis] (const KdNode &m_1, const KdNode &m_2) -> bool {
        return m_1[axis] < m_2[axis];
    });
}

/// Wrapper for the KdNode recursive constructor. Project all stars in the given list to KdNodes and store the
/// indices to the original list here. We populate the tree and return the root with the special property that hr = -1.
///
/// @param v Star list to construct the tree from.
/// @param w_n Width of square to project all stars in tree to.
/// @return Root of the kd-tree formed from v.
KdNode KdNode::load_tree (const Star::list &v, const double w_n) {
    bounds_set b = {bounds {-w_n / 2.0, -w_n / 2.0}, bounds {w_n / 2.0, w_n / 2.0}};
    KdNode::list projected;
    
    // Find the Mercator projection for all stars in V. Insert as KdNodes.
    projected.reserve(v.size());
    for (unsigned int i = 0; i < v.size(); i++) {
        // From full start to finish: (ra, dec) -> <i, j, k> -> (r, lat, lon) -> (x, y).
        KdNode m = KdNode(v[i], w_n);
        
        // We store the index as opposed to the HR value, for general use of the tree.
        m.origin_index = i;
        projected.push_back(m);
    }
    
    // Populate the tree. The root is the center of projection.
    KdNode root = KdNode(0, (unsigned) projected.size() - 1, 0, b, projected);
    
    // The root has the following properties: w_n = w_n, origin_index = -1, hr = -1.
    root.origin_index = -1, root.w_n = w_n, root.hr = -1;
    
    return root;
}

/// Wrapper method for box_query. Query the kd-node for nearby stars to focus using the given fov. In theory,
/// this is a O(lgn) operation as opposed to the regular nearby_stars growth of O(n).
///
/// The node this is operating on **MUST** be the root node itself.
///
/// @param q Star to search around.
/// @param fov Limit a star must be separated from the focus by.
/// @param expected Expected number of stars around the focus. Better to overshoot.
/// @param origin Origin star list the kd-tree was constructed out of.
/// @return Array with all nearby stars.
Star::list KdNode::nearby_stars (const Star &q, const double fov, const unsigned int expected,
                                 const Star::list &origin) {
    Star::list nearby_as_stars;
    KdNode::list nearby;
    
    // Operating node MUST be the root, with the properties below. If not, stop here.
    if (!(this->origin_index == -1 && this->hr == -1)) {
        throw "\"nearby_stars\" not operating on root node.";
    }
    
    // Search for nearby nodes in the tree. The fov represents the half width of the search box, so we double it here.
    nearby.reserve(expected);
    box_query(Mercator(q, this->w_n).find_corners(2 * width_given_angle(fov)), *this, nearby);
    
    // Use the indices in the tree to return the stars in the origin list.
    nearby_as_stars.reserve(nearby.size());
    for (const KdNode &m : nearby) {
        nearby_as_stars.push_back(origin[m.origin_index]);
    }
    
    return nearby_as_stars;
}