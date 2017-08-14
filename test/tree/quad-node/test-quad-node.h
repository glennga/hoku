/// @file test-quad-node.h
/// @author Glenn Galvizo
///
/// Header file for the TestQuadNode class, which tests the QuadNode class.

#ifndef TEST_QUAD_NODE_H
#define TEST_QUAD_NODE_H

#include "base-test.h"
#include "quad-node.h"
#include <cstdio>

class TestQuadNode : public BaseTest {
  private:
    int test_star_constructor ();
    int test_root_property ();
    int test_branch ();
    int test_quadrant_centers ();
    int test_within_quad ();
    int test_reduce ();
    int test_expected_leaf_order ();
    int test_build_simple_quadtree ();
    int test_nearby_stars_quadtree ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_QUAD_NODE_H */