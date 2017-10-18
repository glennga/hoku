/// @file test-kd-node.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestKdNode class, as well as the main function to run the tests.

#include "storage/test-kd-node.h"

/// Check that the KdNode star constructor has the correct components.
///
/// @return 0 when finished.
int TestKdNode::test_star_constructor () {
    std::random_device seed;
    KdNode b(Star::chance(seed), 1000);
    
    assert_equal(b.origin_index, -1, "OriginIndexDefault");
    assert_equal(b.w_n, 1000, "ProjectedWidth");
    return 0 * assert_equal(b.label, 0, "HRValueDefault");
}

/// Check that a list is sorted by the correct dimension.
///
/// @return 0 when finished.
int TestKdNode::test_dimension_sort () {
    std::random_device seed;
    KdNode::list a;
    
    for (int i = 0; i < 20; i++) {
        a.push_back(KdNode(Star::chance(seed), 1000));
    }
    KdNode::list b = a;
    
    KdNode::sort_by_dimension(0, (unsigned) a.size() - 3, 0, a);
    assert_less_than(a[0][0], a[1][0], "ListSortedBy0");
    assert_equal(*(a.end() - 1), *(b.end() - 1), "PartialListSortedOnlyLastElement",
                 (*(a.end() - 1)).str() + "," + (*(b.end() - 1)).str());
    assert_equal(*(a.end() - 2), *(b.end() - 2), "PartialListSortedOnlySecondToLastElement",
                 (*(a.end() - 2)).str() + "," + (*(b.end() - 2)).str());
    
    KdNode::sort_by_dimension(0, (unsigned) a.size() - 1, 1, a);
    assert_less_than(a[0][1], a[1][1], "ListSortedBy1Elements0And1");
    return 0 * assert_less_than(a[1][1], a[2][1], "ListSortedBy1Elements1And2");
}

/// Check that the "==" operator works as intended.
///
/// @return 0 when finished.
int TestKdNode::test_equal_operator () {
    std::random_device seed;
    Star a = Star::chance(seed);
    KdNode b = KdNode(a, 1000);
    
    return 0 * assert_true(b == KdNode(a, 1000), "EqualOperator", a.str());
}

/// Check that a simple tree with 5 elements is built correctly.
///
/// @code{.cpp}
/// t = 0: a = {(176, -175, 0), (156, -152, 1), (147, -140, 2), (142, -133, 3), (139, -128, 4)}
///        We sort based on dim 0 -> {(139, -128, 4), (142, -133, 3), (147, -140, 2), (156, -152, 1), (176, -175, 0)}
///        Median = (147, -140, 2)
/// t = 1: a' = {(139, -128, 4), (142, -133, 3), (147, -140, 2)}
///        We sort based on dim 1 -> {(147, -140, 2), (142, -133, 3), (139, -128, 4)}
///        Median = (142, -133, 3)
/// t = 1: a' = {(156, -152, 1), (176, -175, 0)}
///        We sort based on dim 1 -> {(176, -175, 0), (156, -152, 1)}
///        Median = (176, -175, 0)
/// t = 2 from top t = 1: a'' = {(147, -140, 2), (142, -133, 3)}
///                       We sort based on dim 0 -> {(142, -133, 3), (147, -140, 2)}
///                       Median = (142, -133, 3)
/// t = 2 from top t = 1: a'' = {(139, -128, 4)}
///                       Base case. This is a leaf.
/// t = 2 from bot t = 1: a'' = {(176, -175, 0)}
///                       Base case. This is a leaf.
/// t = 2 from bot t = 1: a'' = {(156, -152, 1)}
///                       Base case. This is a leaf.
/// t = 3 from top t = 2 from top t = 1: a''' = {(142, -133, 3)}
///                                      Base case. This is a leaf.
/// t = 3 from top t = 2 from top t = 1: a''' = {(147, -140, 2)}
///                                      Base case. This is a leaf.
///
/// (147, -140, -1) --> (142, -133, -1) --> (142, -133, -1) --> (142, -133, 3)
///                 |                   |-> (139, -128, 4)  |-> (147, -140, 2)
///                 |-> (176, -175, 0) --> (176, -175, 0)
///                                    |-> (156, -152, 1)
/// @endcode
///
/// @return 0 when finished.
int TestKdNode::test_simple_tree () {
    KdNode::list a, b;
    a.reserve(5), b.reserve(5);
    for (int i = 1; i < 6; i++) {
        KdNode t = KdNode(Star(i, i + 1, i + 2, 0, true), 1000);
        t.origin_index = i - 1;
        a.push_back(t), b.push_back(t);
    }
    KdNode q(0, (unsigned) a.size() - 1, 0, {KdNode::bounds {-500, -500}, KdNode::bounds{500, 500}}, a);
    
    KdNode c = *(*q.right_child).left_child;
    KdNode d = *(*q.right_child).right_child;
    KdNode e = *(*(*q.left_child).left_child).right_child;
    KdNode f = *(*(*q.left_child).left_child).left_child;
    KdNode g = *(*q.left_child).right_child;
    
    assert_equal(b[0], c, "Element0InCorrectPlace", b[0].str() + "," + c.str());
    assert_equal(b[1], d, "Element1InCorrectPlace", b[1].str() + "," + d.str());
    assert_equal(b[2], e, "Element2InCorrectPlace", b[2].str() + "," + e.str());
    assert_equal(b[3], f, "Element3InCorrectPlace", b[3].str() + "," + f.str());
    return 0 * assert_equal(b[4], g, "Element4InCorrectPlace", b[4].str() + "," + g.str());
}

/// Check that the nearby stars method operates as intended.
///
/// @return 0 when finished.
int TestKdNode::test_nearby_stars () {
    Star::list a = Chomp().bright_as_list();
    KdNode q = KdNode::load_tree(a, 1000);
    std::random_device seed;
    Star b = Star::chance(seed);
    Star::list c = Chomp().nearby_bright_stars(b, 10, 90), d = q.nearby_stars(b, 10, 90, a);
    std::vector<double> e, f;
    
    assert_not_equal(Chomp().nearby_bright_stars(b, 10, 90).size(), 0, "NearbyStarsNoKdTree");
    assert_not_equal(q.nearby_stars(b, 10, 90, a).size(), 0, "NearbyStarsUsingKdTree");
    
    for (const Star &s : d) {
        std::string test_name = "NearbyStarIsActuallyNearFocus" + std::to_string(s.get_label());
        // Adding 2 degrees to fov... c and d are both defined by different definitions of "nearby".
        assert_less_than(Star::angle_between(s, b), 10 + 2, test_name);
    }
    
    return 0;
}

/// Enumerate all tests in TestKdNode.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestKdNode::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_star_constructor();
        case 1: return test_dimension_sort();
        case 2: return test_equal_operator();
        case 3: return test_simple_tree();
        case 4: return test_nearby_stars();
        default: return -1;
    }
}

/// Run the tests in TestKdNode. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    return TestKdNode().execute_tests(BaseTest::FULL_PRINT_LOG_ON, 4);
}
