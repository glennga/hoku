/*
 * @file: test-chomp.cpp
 *
 * @brief: Source file for the TestChomp class, as well as the main function to run the tests.
 */

#include "test-chomp.h"

/*
 * Check that the regular query returns correct results. This test is just used to compare
 * against the k-vector query time.
 */
int TestChomp::test_regular_query () {
    Nibble nb;
    std::vector<double> a;
    
    nb.select_table("SEP20");
    a = nb.search_table("theta BETWEEN 5.004 and 5.005", "theta", 90, 30);
    
    for (unsigned int q = 0; q < a.size(); q++) {
        std::string test_name = "RegularQueryResultWithinBoundsSet" + std::to_string(q + 1);
        assert_within(a[q], 5.003, 5.006, test_name);
    }
    
    return 0;
}

/*
 * Check that the k-vector query returns the correct results.
 */
int TestChomp::test_k_vector_query () {
    Chomp ch;
    std::vector<double> a;
    
    ch.select_table("SEP20");
    a = ch.k_vector_query("theta", "theta", 5.004, 5.004, 90);
    
    for (unsigned int q = 0; q < a.size(); q++) {
        std::string test_name = "KVectorQueryResultWithinBoundsSet" + std::to_string(q + 1);
        assert_within(a[q], 5.003, 5.006, test_name);
    }
    
    return 0;
}

/*
 * Check that the QuadNode star constructor has the correct components.
 */
int TestChomp::test_quadnode_star_constructor () {
    QuadNode b(Star::chance(), 1000);
    
    assert_equal(b.w_i, 1, "QuadNodeLocalWidthDefault");
    assert_equal(b.w_n, 1000, "QuadNodeProjectedWidth");
    return 0 * assert_equal(b.hr, 0, "QuadNodeHRValueDefault");
}

/*
 * Check that the QuadNode root has the expected properties.
 */
int TestChomp::test_quadnode_root_property () {
    QuadNode a = QuadNode::root(1000);
    
    assert_equal(a.x, 0, "QuadNodeRootExpectedX");
    assert_equal(a.y, 0, "QuadNodeRootExpectedY");
    assert_equal(a.w_n, 1000, "QuadNodeRootExpectedW_N");
    return 0 * assert_equal(a.w_i, 1000, "QuadNodeRootExpectedW_I");
}

/*
 * Check that the branch method for QuadNode operates as intended.
 */
int TestChomp::test_quadnode_branch () {
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

/*
 * Check that the quadrant centers form a square.
 */
int TestChomp::test_quadnode_quadrant_centers () {
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

/*
 * Check that nodes are distinguished from being inside and outside quadrants correctly.
 */
int TestChomp::test_quadnode_within_quad () {
    QuadNode a(0, 0, 500), b(2000, 2000, 500), c(1, 1, 500);
    
    assert_true(c.within_quadrant(a), "NodeInsideQuadrant", c.str() + "," + a.str());
    return 0 * assert_false(b.within_quadrant(a), "NodeNotInsideQuadrant", b.str() + "," + a.str());
}

int TestChomp::test_quadnode_reduce () {
    QuadNode::list a = {QuadNode(0, 0, 1000), QuadNode(2000, 2000, 1000), QuadNode(1, 1, 1000)};
    QuadNode::list b = QuadNode(0, 0, 1000).reduce_to_quadrant(a, 100);
    
    assert_equal(a.size(), 3, "QuadNodeASizeIs3");
    return 0 * assert_equal(b.size(), 2, "QuadNodeReduction");
}

/*
 * Check that the find_quad_leaves builds the tree in preorder.
 */
int TestChomp::test_quadnode_expected_leaf_order () {
    QuadNode::list a =
        {QuadNode(-251, 251, 1000), QuadNode(251, 249, 1000), QuadNode(-249, -249, 1000), QuadNode(249, -249, 1000)};
    QuadNode::list
        b = {QuadNode(-250, 250, 500), QuadNode(250, 250, 500), QuadNode(-250, -250, 500), QuadNode(250, -250, 500)};
    QuadNode c(0, 0, 1000), d = Chomp().find_quad_leaves(c, 1000, a);
    
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
 * Enumerate all tests in TestChomp.
 *
 * @return -1 if the test case does not exist. 0 otherwise.
 */
int TestChomp::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_regular_query();
        case 1: return test_k_vector_query();
        case 2: return test_quadnode_star_constructor();
        case 3: return test_quadnode_root_property();
        case 4: return test_quadnode_branch();
        case 5: return test_quadnode_quadrant_centers();
        case 6: return test_quadnode_within_quad();
        case 7: return test_quadnode_reduce();
        case 8: return test_quadnode_expected_leaf_order();
        default: return -1;
    }
}

/*
 * Run the tests in TestChomp. Currently set to log and print all data.
 */
int main () {
    //    Chomp c;
    //    Star some_star = Star::chance();
    //    c.load_quad_tree(2000);
    //    auto a = c.nearby_stars_quad_tree(some_star, 10, 10);
    //    std::cout << some_star[0] << " " << some_star[1] << " " << some_star[2] << std::endl;
    //    for (auto b : a) {
    //        std::cout << b[0] << " " << b[1] << " " << b[2] << std::endl;
    //    }
    //    std::cout << a.size() << std::endl;
    //    auto b = 1;
    return TestChomp().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
