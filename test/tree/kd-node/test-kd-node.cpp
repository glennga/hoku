/// @file test-kd-node.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestKdNode class, as well as the main function to run the tests.

#include "test-kd-node.h"

///// Check that the KdNode star constructor has the correct components.
/////
///// @return 0 when finished.
//int TestKdNode::test_star_constructor () {
//    KdNode b(Star::chance(), 1000, 1);
//
//    assert_equal(b.axis, 1, "DimensionAxis");
//    assert_equal(b.w_n, 1000, "ProjectedWidth");
//    return 0 * assert_equal(b.hr, 0, "HRValueDefault");
//}
//
///// Check that the KdNode root has the expected properties.
/////
///// @return 0 when finished.
//int TestKdNode::test_root_property () {
//    KdNode a = KdNode::root(1000);
//
//    assert_equal(a.x, 0, "RootExpectedX");
//    assert_equal(a.y, 0, "RootExpectedY");
//    assert_equal(a.w_n, 1000, "RootExpectedW_N");
//    assert_equal(a.bounds[0][0], -500, "RootExpectedBounds00");
//    assert_equal(a.bounds[0][1], -500, "RootExpectedBounds01");
//    assert_equal(a.bounds[0][2], 0, "RootExpectedBounds02");
//    assert_equal(a.bounds[0][3], 500, "RootExpectedBounds03");
//
//    assert_equal(a.bounds[1][0], 0, "RootExpectedBounds10");
//    assert_equal(a.bounds[1][1], -500, "RootExpectedBounds11");
//    assert_equal(a.bounds[1][2], 500, "RootExpectedBounds12");
//    assert_equal(a.bounds[1][3], 500, "RootExpectedBounds13");
//    return 0 * assert_equal(a.axis, 0, "RootExpectedAxis");
//}
//
///// Check that the branch method for KdNode operates as intended.
/////
///// @return 0 when finished.
//int TestKdNode::test_branch () {
//    KdNode a(Star::chance(), 1000, 0);
//    KdNode::child_edges b = {std::make_shared<KdNode>(KdNode(-5, 5)), nullptr};
//    KdNode c = KdNode::branch(a, b);
//
//    assert_equal(c.x, a.x, "BranchSelfX");
//    assert_equal(c.y, a.y, "BranchSelfY");
//    assert_equal(c.w_n, a.w_n, "BranchSelfW_N");
//    assert_equal(c.axis, a.axis, "BranchSelfAxis");
//    assert_equal(c.hr, a.hr, "BranchSelfHR");
//    assert_equal(c.to_child(0).x, -5, "BranchChild1X");
//    assert_equal(c.to_child(0).y, 5, "BranchChild1Y");
//    return 0 * assert_equal(c.to_child(1).w_n, -1, "BranchChild2IsNull");
//}
//
///// Check that the "==" operator works as intended.
/////
///// @return 0 when finished.
//int TestKdNode::test_equal_operator () {
//    KdNode a = KdNode::root(1000);
//
//    return 0 * assert_true(a == KdNode::root(1000), "EqualOperator", a.str() + "," + KdNode::root(1000).str());
//}
//
///// Check that the reduction method removes the correct stars.
/////
///// @return 0 when finished.
//int TestKdNode::test_reduction () {
//    KdNode a = KdNode::root(1000);
//    KdNode::list b = {KdNode(250, 250), KdNode(-250, -250)};
//    KdNode::list c = KdNode::reduce_using_bounds(b, a, 0), d = KdNode::reduce_using_bounds(b, a, 1);
//
//    assert_equal(c.size(), 1, "CorrectNumberOfStarsReduction0");
//    assert_equal(d.size(), 1, "CorrectNumberOfStarsReduction1");
//    assert_equal(c[0], b[1], "Star0ExistsInSet", c[0].str() + "," + b[0].str(true));
//    return 0 * assert_equal(d[0], b[0], "Star1ExistsInSet", d[0].str() + "," + b[1].str());
//}
//
///// Check that find_median for KdNode correctly finds the median in a given list.
/////
///// @return 0 when finished.
//int TestKdNode::test_find_median () {
//    KdNode::list a = {KdNode(-1, 123), KdNode(-2, 2), KdNode(-3, 3), KdNode(-4, 4)};
//    KdNode::list b = {KdNode(-1, 123), KdNode(-2, 2), KdNode(-3, 3), KdNode(-4, 4), KdNode(-0.5, 0.5)};
//    KdNode c = KdNode::root(1000), d = KdNode(0, 0, 1), e = KdNode::find_bounds(c, 0, a), f = KdNode(0, 0, 0);
//    d.bounds[0] = {-500, -500, 0, 3}, d.bounds[1] = {-500, 3, 0, 500};
//
//    assert_equal(e, d, "UnsortedEvenListFindMedian", e.str() + "," + d.str());
//
//    // Now sorting by axis 1 instead of 0.
//    KdNode g = KdNode::find_bounds(e, 1, b);
//    f.bounds[0] = {-500, 3, -4, 500}, f.bounds[1] = {-4, 3, 0, 500};
//    return 0 * assert_equal(g, f, "UnsortedListFindMedianDifferentAxis", g.str() + "," + f.str());
//}
//
///// Check that a tree with unbalanced data is built correctly.
/////
///// @return 0 when finished.
//int TestKdNode::test_unbalanced_data () {
//    KdNode::list a = {KdNode(-1, -1), KdNode(-2, -1), KdNode(-3, -1), KdNode(-4, -1)};
//    KdNode::list b = {KdNode(0, 0, 1), KdNode(0, 0, 0)};
//    b[0].bounds[0] = {-500, -500, 0, -1}, b[0].bounds[1] = {-500, -1, 0, 500};
//    KdNode c = KdNode::root(1000), d = KdNode::populate_list(c, a);
//
//    KdNode e = d.to_child(0).to_child(1).to_child(0).to_child(0);
//    KdNode f = d.to_child(0).to_child(1).to_child(0).to_child(1);
//    KdNode g = d.to_child(0).to_child(1).to_child(1).to_child(0);
//    KdNode h = d.to_child(0).to_child(1).to_child(1).to_child(1);
//
//    assert_equal(d.to_child(0), b[0], "RootChild0", d.to_child(0).str() + "," + b[0].str());
//    assert_equal(d.to_child(1), KdNode::root(-1), "RootChild1IsNull", d.to_child(1).str());
//
//    KdNode i = d.to_child(0).to_child(0), j = d.to_child(0).to_child(1);
//    assert_equal(i, KdNode::root(-1), "Child0Child0IsNull", i.str());
//
//    assert_equal(e, a[3], "FirstChildPlacedCorrectly", e.str() + "," + a[3].str());
//    assert_equal(f, a[2], "SecondChildPlacedCorrectly", f.str() + "," + a[2].str());
//    assert_equal(g, a[1], "ThirdChildPlacedCorrectly", g.str() + "," + a[1].str());
//    return 0 * assert_equal(h, a[0], "FourthChildPlacedCorrectly", h.str() + "," + a[0].str());
//}

/// Check that the nearby stars method operates as intended.
///
/// @return 0 when finished.
int TestKdNode::test_nearby_stars () {
    Star::list asd = Nibble().all_bsc5_stars(), basd = Nibble().all_bsc5_stars();
    for (int i = 0; i < 100; i++) {
        asd.insert(asd.begin(), basd.begin(), basd.end());
    
    }
    
    KdNode q = KdNode::load_tree(asd, 1000);
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

/// Enumerate all tests in TestKdNode.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestKdNode::enumerate_tests (int test_case) {
    switch (test_case) {
//        case 0: return test_star_constructor();
//        case 1: return test_root_property();
//        case 2: return test_branch();
//        case 3: return test_equal_operator();
//        case 4: return test_reduction();
//        case 5: return test_find_median();
//        case 6: return test_unbalanced_data();
        case 7: return test_nearby_stars();
        default: return -1;
    }
}

/// Run the tests in TestKdNode. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    return TestKdNode().execute_tests(BaseTest::FULL_PRINT_LOG_ON, 7);
}
