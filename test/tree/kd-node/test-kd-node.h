/// @file test-kd-node.h
/// @author Glenn Galvizo
///
/// Header file for the TestKdNode class, which tests the KdNode class.

#ifndef TEST_KD_NODE_H
#define TEST_KD_NODE_H

#include "base-test.h"
#include "kd-node.h"
#include <cstdio>

class TestKdNode : public BaseTest {
  private:
    int test_star_constructor ();
    int test_root_property ();
    int test_branch ();
    int test_equal_operator ();
    int test_reduction ();
    int test_find_median ();
    int test_unbalanced_data ();
    int test_nearby_stars ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_KD_NODE_H */