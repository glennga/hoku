/// @file test-quad-node.cpp
/// @author Glenn Galvizo
///
/// Source file for all QuadNode class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "storage/quad-node.h"
#include "gtest/gtest.h"

/// Check that the QuadNode star constructor has the correct components.
TEST(QuadNodeConstructor, StarConstructor) {
    std::random_device seed;
    QuadNode b(Star::chance(seed), 1000, 1);
    EXPECT_DOUBLE_EQ(b.w_i, QuadNode::DEFAULT_LOCAL_WIDTH);
    EXPECT_DOUBLE_EQ(b.w_n, 1000);
    EXPECT_EQ(b.label, Mercator::NO_LABEL);
}

/// Check that the QuadNode root has the expected properties.
TEST(QuadNodeProperty, Root) {
    QuadNode a = QuadNode::root(1000);
    EXPECT_DOUBLE_EQ(a.x, 0);
    EXPECT_DOUBLE_EQ(a.y, 0);
    EXPECT_DOUBLE_EQ(a.w_n, 1000);
    EXPECT_DOUBLE_EQ(a.w_i, 1000);
}

/// Check that the branch method for QuadNode operates as intended.
TEST(QuadNodeBranch, Branch) {
    std::random_device seed;
    QuadNode a(Star::chance(seed), 1000, 1);
    QuadNode::child_edges b = {std::make_shared<QuadNode>(QuadNode(-5, 5, 1000)), nullptr, nullptr, nullptr};
    QuadNode c = QuadNode::branch(a, b);
    
    EXPECT_DOUBLE_EQ(c.x, a.x);
    EXPECT_DOUBLE_EQ(c.y, a.y);
    EXPECT_DOUBLE_EQ(c.w_n, a.w_n);
    EXPECT_DOUBLE_EQ(c.w_i, a.w_i);
    EXPECT_EQ(c.label, a.label);
    EXPECT_DOUBLE_EQ(c.to_child(0).x, -5);
    EXPECT_DOUBLE_EQ(c.to_child(0).y, 5);
    EXPECT_EQ(c.to_child(1).w_n, QuadNode::ROOT_GLOBAL_WIDTH);
}

/// Check that the quadrant centers form a square.
TEST(QuadNodeQuadrant, Centers) {
    QuadNode::child_edges a = QuadNode(0, 0, 1000).find_quadrant_centers();
    QuadNode b = QuadNode::branch(QuadNode(0, 0, 1000), a);
    
    EXPECT_DOUBLE_EQ(b.to_child(0).y, b.to_child(1).y);
    EXPECT_DOUBLE_EQ(b.to_child(2).y, b.to_child(3).y);
    EXPECT_DOUBLE_EQ(b.to_child(0).x, b.to_child(2).x);
    EXPECT_DOUBLE_EQ(b.to_child(1).x, b.to_child(3).x);
    EXPECT_DOUBLE_EQ(b.to_child(0).w_i, 500);
    EXPECT_DOUBLE_EQ(b.to_child(1).w_i, 500);
    EXPECT_DOUBLE_EQ(b.to_child(2).w_i, 500);
    EXPECT_DOUBLE_EQ(b.to_child(3).w_i, 500);
    EXPECT_DOUBLE_EQ(b.w_i, 1000);
}

/// Check that nodes are distinguished from being inside and outside quadrants correctly.
TEST(QuadNodeQuadrant, Within) {
    QuadNode a(-250, 250, 500), b(1, 1, 500), c(-251, 251, 500), d(-251, 251, 0);
    EXPECT_TRUE(c.within_quadrant(a));
    EXPECT_TRUE(d.within_quadrant(a));
    EXPECT_FALSE(b.within_quadrant(a));
}

/// Check that the reduction method removes the correct stars, and keeps the correct stars.
TEST(QuadNodeQuadrant, Reduce) {
    QuadNode::list a = {QuadNode(0, 0, 1000), QuadNode(2000, 2000, 1000), QuadNode(1, 1, 1000)};
    QuadNode::list b = QuadNode(0, 0, 1000).reduce_to_quadrant(a, 100);
    EXPECT_EQ(a.size(), 3);
    EXPECT_EQ(b.size(), 2);
}

/// Check that the quadrant_intersects_quadrant works as intended.
TEST(QuadNodeQuadrant, Intersection) {
    QuadNode a(-250, 250, 500), b(250, 250, 500), c(0, 250, 488);
    EXPECT_FALSE(a.quadrant_intersects_quadrant(b));
    EXPECT_TRUE(a.quadrant_intersects_quadrant(c));
    EXPECT_TRUE(b.quadrant_intersects_quadrant(c));
    EXPECT_TRUE(c.quadrant_intersects_quadrant(a));
}

/// Check that the find_quad_leaves builds the tree in preorder.
TEST(QuadNodeProperty, ExpectedLeafOrder) {
    QuadNode::list a = {QuadNode(-251, 251, 1000), QuadNode(251, 249, 1000), QuadNode(-249, -249, 1000),
        QuadNode(249, -249, 1000)};
    QuadNode::list b = {QuadNode(-250, 250, 500), QuadNode(250, 250, 500), QuadNode(-250, -250, 500),
        QuadNode(250, -250, 500)};
    QuadNode c(0, 0, 1000), d = QuadNode::root(1000).find_quad_leaves(c, 1000, a);
    EXPECT_EQ(c, d);
    
    for (int q = 0; q < 4; q++) {
        EXPECT_EQ(b[q], d.to_child(q));
    }
    for (int q = 0; q < 4; q++) {
        EXPECT_EQ(a[q], d.to_child(q).to_child(0));
        for (int p = 1; p < 4; p++) {
            EXPECT_TRUE(d.to_child(q).children[p] == nullptr);
        }
    }
}

/// Check that an unbalanced tree is built correctly.
TEST(QuadNodeTree, Unbalanced) {
    QuadNode::list a = {QuadNode(-251, 251, 1000), QuadNode(-252, 252, 1000), QuadNode(-253, 253, 1000),
        QuadNode(-254, 254, 1000)};
    QuadNode b(0, 0, 1000), c = QuadNode::root(1000).find_quad_leaves(b, 1000, a);
    
    for (int q = 0; q < 4; q++) {
        EXPECT_EQ(a[q], c.to_child(0).to_child(q));
        EXPECT_TRUE(c.to_child(0).to_child(q).is_green);
    }
    
    EXPECT_TRUE(c.to_child(0).is_terminal_branch());
    EXPECT_FALSE(c.to_child(1).is_terminal_branch());
    EXPECT_FALSE(c.to_child(0).children == QuadNode::no_children());
    EXPECT_EQ(c.to_child(1).children, QuadNode::no_children());
    EXPECT_EQ(c.to_child(2).children, QuadNode::no_children());
    EXPECT_EQ(c.to_child(3).children, QuadNode::no_children());
}

/// Check that an unbalanced tree with 5 nodes for the first level case is built correctly.
TEST(QuadNodeTree, PartitionForLeaves) {
    QuadNode::list a = {QuadNode(-251, 251, 1000), QuadNode(-252, 252, 1000), QuadNode(-253, 253, 1000),
        QuadNode(-254, 254, 1000), QuadNode(-126, 126, 1000)};
    QuadNode b(0, 0, 1000), c = QuadNode::root(1000).find_quad_leaves(b, 1000, a);
    QuadNode d = c.to_child(0).to_child(0), e = c.to_child(0).to_child(3);
    QuadNode::list f = {QuadNode(-375, 375, 250), QuadNode(-125, 375, 250), QuadNode(-375, 125, 250),
        QuadNode(-125, 125, 250)};
    
    for (int q = 0; q != 6; q += 3) {
        EXPECT_EQ(f[q], c.to_child(0).to_child(q));
    }
    for (int q = 0; q < 4; q++) {
        EXPECT_EQ(a[q], d.to_child(q));
    }
    EXPECT_EQ(a[4], e.to_child(0));
    EXPECT_FALSE(c.to_child(0).children == QuadNode::no_children());
    EXPECT_EQ(c.to_child(1).children, QuadNode::no_children());
    EXPECT_EQ(c.to_child(2).children, QuadNode::no_children());
    EXPECT_EQ(c.to_child(3).children, QuadNode::no_children());
}

/// Check that the nearby stars method operates as intended.
TEST(QuadNodeNearby, NearbyStars) {
    QuadNode q = QuadNode::load_tree(10000, 6.0);
    std::random_device seed;
    Star a = Star::chance(seed);
    Star::list b = Chomp().nearby_hip_stars(a, 10, 90), c = q.nearby_stars(a, 10, 90);
    std::vector<double> d, e;
    EXPECT_NE(b.size(), 0);
    EXPECT_NE(c.size(), 0);
    
    // Adding 3 degrees to fov... B and C are both defined by different definitions of "nearby".
    for (const Star &s : c) {
        EXPECT_LT(Star::angle_between(s, a), 10 + 3);
    }
}

/// Runs all tests defined in this file.
///
/// @param argc Argument count. Used in Google Test initialization.
/// @param argv Argument vector. Used in Google Test initialization.
/// @return The result of running all tests.
int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}