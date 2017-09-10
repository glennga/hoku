/// @file test-kd-node.h
/// @author Glenn Galvizo
///
/// Header file for the TestKdNode class, which tests the KdNode class.

#ifndef TEST_KD_NODE_H
#define TEST_KD_NODE_H

#include "base-test/base-test.h"
#include "../../../include/storage/kd-node.h"
#include <cstdio>

class TestKdNode : public BaseTest {
  private:
    int test_star_constructor ();
    int test_dimension_sort ();
    int test_equal_operator ();
    int test_simple_tree ();
    int test_nearby_stars ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_KD_NODE_H */