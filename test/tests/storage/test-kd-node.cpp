/// @file test-kd-node.cpp
/// @author Glenn Galvizo
///
/// Source file for all KdNode class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "gtest/gtest.h"

#include "storage/chomp.h"
#include "storage/kd-node.h"

/// Check that the KdNode star constructor has the correct components.
TEST(KdNodeConstructor, StarConstructor) {
    KdNode b(Star::chance(), 1000);
    EXPECT_EQ(b.origin_index, KdNode::NO_ORIGIN);
    EXPECT_DOUBLE_EQ(b.w_n, 1000);
    EXPECT_EQ(b.label, Mercator::NO_LABEL);
}

/// Check that a list is sorted by the correct dimension.
TEST(KdNodeProperty, DimensionSort) {
    KdNode::list a;
    for (int i = 0; i < 20; i++) {
        a.emplace_back(Star::chance(), 1000);
    }
    KdNode::list b = a;
    KdNode::sort_by_dimension(0, (unsigned) a.size() - 3, 0, a);
    
    EXPECT_LT(a[0][0], a[1][0]);
    EXPECT_EQ(*(a.end() - 1), *(b.end() - 1));
    EXPECT_EQ(*(a.end() - 2), *(b.end() - 2));
    KdNode::sort_by_dimension(0, (unsigned) a.size() - 1, 1, a);
    EXPECT_LT(a[0][1], a[1][1]);
    EXPECT_LT(a[1][1], a[2][1]);
}

/// Check that the "==" operator works as intended.
TEST(KdNodeOperator, Equality) {
    Star a = Star::chance();
    KdNode b = KdNode(a, 1000);
    EXPECT_TRUE(b == KdNode(a, 1000));
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
TEST(KdNodeTree, Simple) {
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
    
    EXPECT_EQ(b[0], c);
    EXPECT_EQ(b[1], d);
    EXPECT_EQ(b[2], e);
    EXPECT_EQ(b[3], f);
    EXPECT_EQ(b[4], g);
}

/// Check that the nearby stars method operates as intended.
TEST(KdNodeNearby, NearbyStars) {
    Star::list a = Chomp().bright_as_list();
    KdNode q = KdNode::load_tree(a, 1000);
    Star b = Star::chance();
    Star::list c = Chomp().nearby_bright_stars(b, 10, 90), d = q.nearby_stars(b, 10, 90, a);
    std::vector<double> e, f;
    
    ASSERT_NE(Chomp().nearby_bright_stars(b, 10, 90).size(), 0);
    ASSERT_NE(q.nearby_stars(b, 10, 90, a).size(), 0);
    
    for (const Star &s : d) {
        EXPECT_LT(Star::angle_between(s, b), 10 + 2);
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