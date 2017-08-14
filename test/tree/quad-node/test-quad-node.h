/*
 * @file: test-chomp.h
 *
 * @brief: Header file for the TestChomp class, which tests the TestChomp namespace.
 */

#ifndef TEST_CHOMP_H
#define TEST_CHOMP_H

#include "base-test.h"
#include "chomp.h"
#include <cstdio>

class TestChomp : public BaseTest {
  private:
    int test_regular_query ();
    int test_k_vector_query ();
    int test_quadnode_star_constructor ();
    int test_quadnode_root_property ();
    int test_quadnode_branch ();
    int test_quadnode_quadrant_centers ();
    int test_quadnode_within_quad ();
    int test_quadnode_reduce ();
    int test_quadnode_expected_leaf_order ();
    int test_build_simple_quadtree ();
    int test_nearby_stars_quadtree ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_CHOMP_H */