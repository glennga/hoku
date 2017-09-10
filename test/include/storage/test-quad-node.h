/// @file test-quad-node.h
/// @author Glenn Galvizo
///
/// Header file for the TestQuadNode class, which tests the QuadNode class.

#ifndef TEST_QUAD_NODE_H
#define TEST_QUAD_NODE_H

#include "base-test/base-test.h"
#include "../../../include/storage/quad-node.h"
#include <cstdio>

class TestQuadNode : public BaseTest {
  private:
    int test_star_constructor ();
    int test_root_property ();
    int test_branch ();
    int test_quadrant_centers ();
    int test_within_quad ();
    int test_reduce ();
    int test_quadrant_intersection ();
    int test_expected_leaf_order ();
    int test_unbalanced_tree ();
    int test_partition_for_leaves ();
    int test_nearby_stars ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_QUAD_NODE_H */