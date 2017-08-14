/// @file test-quad-node.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestQuadNode class, as well as the main function to run the tests.

#include "test-quad-node.h"

/// Check that the QuadNode star constructor has the correct components.
///
/// @return 0 when finished.
int TestQuadNode::test_star_constructor () {
    QuadNode b(Star::chance(), 1000);
    
    assert_equal(b.w_i, 1, "QuadNodeLocalWidthDefault");
    assert_equal(b.w_n, 1000, "QuadNodeProjectedWidth");
    return 0 * assert_equal(b.hr, 0, "QuadNodeHRValueDefault");
}

/// Check that the QuadNode root has the expected properties.
/// 
/// @return 0 when finished.
int TestQuadNode::test_root_property () {
    QuadNode a = QuadNode::root(1000);
    
    assert_equal(a.x, 0, "QuadNodeRootExpectedX");
    assert_equal(a.y, 0, "QuadNodeRootExpectedY");
    assert_equal(a.w_n, 1000, "QuadNodeRootExpectedW_N");
    return 0 * assert_equal(a.w_i, 1000, "QuadNodeRootExpectedW_I");
}

/// Check that the branch method for QuadNode operates as intended.
///
/// @return 0 when finished.
int TestQuadNode::test_branch () {
    QuadNode a(Star::chance(), 1000);
    QuadNode::child_edges b = {std::make_shared<QuadNode>(QuadNode(-5, 5, 1000)), nullptr, nullptr, nullptr};
    QuadNode c = QuadNode::branch(a, b);
    
    assert_equal(c.x, a.x, "BranchSelfX");
    assert_equal(c.y, a.y, "BranchSelfY");
    assert_equal(c.w_n, a.w_n, "BranchSelfW_N");
    assert_equal(c.w_i, a.w_i, "BranchSelfW_I");
    assert_equal(c.hr, a.hr, "BranchSelfHR");
    assert_equal(c.to_child(0).x, -5, "BranchChild1X");
    assert_equal(c.to_child(0).y, 5, "BranchChild1Y");
    return 0 * assert_equal(c.to_child(1).w_n, -1, "BranchChild2IsNull");
}

/// Check that the quadrant centers form a square.
///
/// @return 0 when finished.
int TestQuadNode::test_quadrant_centers () {
    QuadNode::child_edges a = QuadNode(0, 0, 1000).find_quadrant_centers();
    QuadNode b = QuadNode::branch(QuadNode(0, 0, 1000), a);
    
    assert_equal(b.to_child(0).y, b.to_child(1).y, "QuadrantCenterFindSameTopY");
    assert_equal(b.to_child(2).y, b.to_child(3).y, "QuadrantCenterFindSameBottomY");
    assert_equal(b.to_child(0).x, b.to_child(2).x, "QuadrantCenterFindSameLeftX");
    assert_equal(b.to_child(1).x, b.to_child(3).x, "QuadrantCenterFindSameRightX");
    
    assert_equal(b.to_child(0).w_i, 500, "QuadrantCenterExpectedW_IChild0");
    assert_equal(b.to_child(1).w_i, 500, "QuadrantCenterExpectedW_IChild1");
    assert_equal(b.to_child(2).w_i, 500, "QuadrantCenterExpectedW_IChild2");
    assert_equal(b.to_child(3).w_i, 500, "QuadrantCenterExpectedW_IChild3");
    return 0 * assert_equal(b.w_i, 1000, "QuadrantCenterExpectedW_IParent");
}

/// Check that nodes are distinguished from being inside and outside quadrants correctly.
///
/// @return 0 when finished.
int TestQuadNode::test_within_quad () {
    QuadNode a(0, 0, 500), b(2000, 2000, 500), c(1, 1, 500);
    
    assert_true(c.within_quadrant(a), "NodeInsideQuadrant", c.str() + "," + a.str());
    return 0 * assert_false(b.within_quadrant(a), "NodeNotInsideQuadrant", b.str() + "," + a.str());
}

/// Check that the reduction method removes the correct stars, and keeps the correct stars.
///
/// @return 0 when finished.
int TestQuadNode::test_reduce () {
    QuadNode::list a = {QuadNode(0, 0, 1000), QuadNode(2000, 2000, 1000), QuadNode(1, 1, 1000)};
    QuadNode::list b = QuadNode(0, 0, 1000).reduce_to_quadrant(a, 100);
    
    assert_equal(a.size(), 3, "QuadNodeASizeIs3");
    return 0 * assert_equal(b.size(), 2, "QuadNodeReduction");
}

/// Check that the find_quad_leaves builds the tree in preorder.
///
/// @return 0 when finished.
int TestQuadNode::test_expected_leaf_order () {
    QuadNode::list a =
        {QuadNode(-251, 251, 1000), QuadNode(251, 249, 1000), QuadNode(-249, -249, 1000), QuadNode(249, -249, 1000)};
    QuadNode::list
        b = {QuadNode(-250, 250, 500), QuadNode(250, 250, 500), QuadNode(-250, -250, 500), QuadNode(250, -250, 500)};
    QuadNode c(0, 0, 1000), d = QuadNode::root(1000).find_quad_leaves(c, 1000, a);
    
    assert_equal(c, d, "QuadNodeExpectedRoot", c.str() + "," + d.str());
    
    for (int q = 0; q < 4; q++) {
        std::string test_name = "QuadNodeExpectedBranch" + std::to_string(q);
        assert_equal(b[q], d.to_child(q), test_name, b[q].str() + "," + d.to_child(q).str());
    }
    
    for (int q = 0; q < 4; q++) {
        std::string test_name = "QuadNodeExpectedChild" + std::to_string(q) + "0";
        assert_equal(a[q], d.to_child(q).to_child(0), test_name, a[q].str() + "," + d.to_child(q).to_child(0).str());
        for (int p = 1; p < 4; p++) {
            std::string test_name_2 = "QuadNodeExpectedChild" + std::to_string(q) + std::to_string(p);
            assert_true(d.to_child(q).children[p] == nullptr, test_name_2, d.to_child(q).str());
        }
    }
    
    return 0;
}

/*
 * Enumerate all tests in TestQuadNode.
 *
 * @return -1 if the test case does not exist. 0 otherwise.
 */
int TestQuadNode::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_star_constructor();
        case 1: return test_root_property();
        case 2: return test_branch();
        case 3: return test_quadrant_centers();
        case 4: return test_within_quad();
        case 5: return test_reduce();
        case 6: return test_expected_leaf_order();
        default: return -1;
    }
}

/// Run the tests in TestQuadNode. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    return TestQuadNode().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
