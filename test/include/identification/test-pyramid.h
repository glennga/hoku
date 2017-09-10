/// @file test-pyramid.h
/// @author Glenn Galvizo
///
/// Header file for the TestPyramid class, which tests the Pyramid class.

#ifndef TEST_PYRAMID_H
#define TEST_PYRAMID_H

#include "base-test/base-test.h"
#include "identification/pyramid.h"

class TestPyramid : public BaseTest {
  private:
    int test_pairs_query ();
    int test_reference_find ();
    int test_candidate_quad_find();
    int test_identify_clean_input ();
    int test_identify_error_input ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_PYRAMID_H */