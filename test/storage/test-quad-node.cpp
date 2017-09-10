/// @file test-quad-node.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestQuadNode class, as well as the main function to run the tests.

#include "storage/test-quad-node.h"

/// Check that the QuadNode star constructor has the correct components.
///
/// @return 0 when finished.
int TestQuadNode::test_star_constructor () {
    QuadNode b(Star::chance(), 1000, 1);
    
    assert_equal(b.w_i, 1, "LocalWidthDefault");
    assert_equal(b.w_n, 1000, "ProjectedWidth");
    return 0 * assert_equal(b.hr, 0, "HRValueDefault");
}

/// Check that the QuadNode root has the expected properties.
/// 
/// @return 0 when finished.
int TestQuadNode::test_root_property () {
    QuadNode a = QuadNode::root(1000);
    
    assert_equal(a.x, 0, "RootExpectedX");
    assert_equal(a.y, 0, "RootExpectedY");
    assert_equal(a.w_n, 1000, "RootExpectedW_N");
    return 0 * assert_equal(a.w_i, 1000, "RootExpectedW_I");
}

/// Check that the branch method for QuadNode operates as intended.
///
/// @return 0 when finished.
int TestQuadNode::test_branch () {
    QuadNode a(Star::chance(), 1000, 1);
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
    QuadNode a(-250, 250, 500), b(1, 1, 500), c(-251, 251, 500), d(-251, 251, 0);
    
    assert_true(c.within_quadrant(a), "NodeInsideQuadrant", c.str() + "," + a.str());
    assert_true(d.within_quadrant(a), "NodeInsideQuadrantWithoutW_I", d.str() + "," + a.str());
    return 0 * assert_false(b.within_quadrant(a), "NodeNotInsideQuadrant", b.str() + "," + a.str());
}

/// Check that the reduction method removes the correct stars, and keeps the correct stars.
///
/// @return 0 when finished.
int TestQuadNode::test_reduce () {
    QuadNode::list a = {QuadNode(0, 0, 1000), QuadNode(2000, 2000, 1000), QuadNode(1, 1, 1000)};
    QuadNode::list b = QuadNode(0, 0, 1000).reduce_to_quadrant(a, 100);
    
    assert_equal(a.size(), 3, "SizeIs3");
    return 0 * assert_equal(b.size(), 2, "Reduction");
}

/// Check that the quadrant_intersects_quadrant works as intended.
///
/// @return 0 when finished.
int TestQuadNode::test_quadrant_intersection () {
    QuadNode a(-250, 250, 500), b(250, 250, 500), c(0, 250, 488);
    
    assert_false(a.quadrant_intersects_quadrant(b), "LeftDoesNotIntersectRight", a.str() + "," + b.str());
    assert_true(a.quadrant_intersects_quadrant(c), "LeftIntersectsMiddle", a.str() + "," + c.str());
    assert_true(b.quadrant_intersects_quadrant(c), "RightIntersectsMiddle", b.str() + "," + c.str());
    return 0 * assert_true(c.quadrant_intersects_quadrant(a), "MiddleIntersectsLeft", c.str() + "," + a.str());
}

/// Check that the find_quad_leaves builds the tree in preorder.
///
/// @return 0 when finished.
int TestQuadNode::test_expected_leaf_order () {
    QuadNode::list a = {QuadNode(-251, 251, 1000), QuadNode(251, 249, 1000), QuadNode(-249, -249, 1000),
        QuadNode(249, -249, 1000)};
    QuadNode::list b = {QuadNode(-250, 250, 500), QuadNode(250, 250, 500), QuadNode(-250, -250, 500),
        QuadNode(250, -250, 500)};
    QuadNode c(0, 0, 1000), d = QuadNode::root(1000).find_quad_leaves(c, 1000, a);
    
    assert_equal(c, d, "ExpectedRoot", c.str() + "," + d.str());
    
    for (int q = 0; q < 4; q++) {
        std::string test_name = "ExpectedBranch" + std::to_string(q);
        assert_equal(b[q], d.to_child(q), test_name, b[q].str() + "," + d.to_child(q).str());
    }
    
    for (int q = 0; q < 4; q++) {
        std::string test_name = "ExpectedChild" + std::to_string(q) + "0";
        assert_equal(a[q], d.to_child(q).to_child(0), test_name, a[q].str() + "," + d.to_child(q).to_child(0).str());
        for (int p = 1; p < 4; p++) {
            std::string test_name_2 = "ExpectedChild" + std::to_string(q) + std::to_string(p);
            assert_true(d.to_child(q).children[p] == nullptr, test_name_2, d.to_child(q).str());
        }
    }
    
    return 0;
}

/// Check that an unbalanced tree is built correctly.
///
/// @return 0 when finished.
int TestQuadNode::test_unbalanced_tree () {
    QuadNode::list a = {QuadNode(-251, 251, 1000), QuadNode(-252, 252, 1000), QuadNode(-253, 253, 1000),
        QuadNode(-254, 254, 1000)};
    QuadNode b(0, 0, 1000), c = QuadNode::root(1000).find_quad_leaves(b, 1000, a);
    
    for (int q = 0; q < 4; q++) {
        std::string test_name = "ExpectedChildForUnbalanced" + std::to_string(q);
        assert_equal(a[q], c.to_child(0).to_child(q), test_name, a[q].str() + "," + c.to_child(0).to_child(q).str());
        assert_true(c.to_child(0).to_child(q).is_green, "ChildIsColoredGreen");
    }
    
    assert_true(c.to_child(0).is_terminal_branch(), "UpLeftIsTerminalBranch");
    assert_false(c.to_child(1).is_terminal_branch(), "UpRightIsNotATerminalBranch");
    
    assert_false(c.to_child(0).children == QuadNode::no_children(), "HasChildrenFor0");
    assert_true(c.to_child(1).children == QuadNode::no_children(), "NoChildrenFor1");
    assert_true(c.to_child(2).children == QuadNode::no_children(), "NoChildrenFor2");
    return 0 * assert_true(c.to_child(3).children == QuadNode::no_children(), "NoChildrenFor3");
}

/// Check that an unbalanced tree with 5 nodes for the first level case is built correctly.
///
/// @return 0 when finished.
int TestQuadNode::test_partition_for_leaves () {
    QuadNode::list a = {QuadNode(-251, 251, 1000), QuadNode(-252, 252, 1000), QuadNode(-253, 253, 1000),
        QuadNode(-254, 254, 1000), QuadNode(-126, 126, 1000)};
    QuadNode b(0, 0, 1000), c = QuadNode::root(1000).find_quad_leaves(b, 1000, a);
    QuadNode d = c.to_child(0).to_child(0), e = c.to_child(0).to_child(3);
    QuadNode::list f = {QuadNode(-375, 375, 250), QuadNode(-125, 375, 250), QuadNode(-375, 125, 250),
        QuadNode(-125, 125, 250)};
    
    for (int q = 0; q != 6; q += 3) {
        std::string test_name = "ExpectedBranchForChild0" + std::to_string(q);
        assert_equal(f[q], c.to_child(0).to_child(q), test_name, f[q].str() + "," + c.to_child(0).to_child(q).str());
    }
    
    for (int q = 0; q < 4; q++) {
        std::string test_name = "ExpectedChildNumber0ForManyLeaves" + std::to_string(q);
        assert_equal(a[q], d.to_child(q), test_name, a[q].str() + "," + d.to_child(q).str());
    }
    assert_equal(a[4], e.to_child(0), "ExpectedChildNumber3ForManyLeaves5", a[4].str() + "," + e.to_child(0).str());
    
    assert_false(c.to_child(0).children == QuadNode::no_children(), "HasChildrenFor0ForManyLeaves");
    assert_true(c.to_child(1).children == QuadNode::no_children(), "NoChildrenFor1ForManyLeaves");
    assert_true(c.to_child(2).children == QuadNode::no_children(), "NoChildrenFor2ForManyLeaves");
    return 0 * assert_true(c.to_child(3).children == QuadNode::no_children(), "NoChildrenFor3ForManyLeaves");
}

/// Check that the nearby stars method operates as intended.
///
/// @return 0 when finished.
int TestQuadNode::test_nearby_stars () {
    QuadNode q = QuadNode::load_tree(1000);
    Star a = Star::chance();
    Star::list b = Nibble().nearby_stars(a, 10, 90), c = q.nearby_stars(a, 10, 90);
    std::vector<double> d, e;
    
    assert_not_equal(Nibble().nearby_stars(a, 10, 90).size(), 0, "NearbyStarsNoQuadTree");
    assert_not_equal(q.nearby_stars(a, 10, 90).size(), 0, "NearbyStarsUsingQuadTree");
    
    for (const Star &s : c) {
        std::string test_name = "NearbyStarIsActuallyNearFocus" + std::to_string(s.get_hr());
        // Adding 3 degrees to fov... B and C are both defined by different definitions of "nearby".
        assert_less_than(Star::angle_between(s, a), 10 + 3, test_name);
    }
    
    return 0;
}

/// Enumerate all tests in TestQuadNode.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestQuadNode::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_star_constructor();
        case 1: return test_root_property();
        case 2: return test_branch();
        case 3: return test_quadrant_centers();
        case 4: return test_within_quad();
        case 5: return test_reduce();
        case 6: return test_quadrant_intersection();
        case 7: return test_expected_leaf_order();
        case 8: return test_unbalanced_tree();
        case 9: return test_partition_for_leaves();
        case 10: return test_nearby_stars();
        default: return -1;
    }
}

/// Run the tests in TestQuadNode. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    return TestQuadNode().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
