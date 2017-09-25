/// @file test-planar-triangle.h
/// @author Glenn Galvizo
///
/// Header file for the TestPlanarTriangle class, which tests the PlanarTriangle class.


#ifndef TEST_PLANAR_TRIANGLE_H
#define TEST_PLANAR_TRIANGLE_H

#include "base-test/base-test.h"
#include "identification/planar-triangle.h"
#include <cstdio>

class TestPlanarTriangle : public BaseTest {
  private:
    int test_trio_query ();
    int test_match_stars_fov ();
    int test_match_stars_none ();
    int test_match_stars_results ();
    int test_pivot_query_results ();
    int test_rotating_match_correct_input ();
    int test_rotating_match_error_input ();
    int test_rotating_match_duplicate_input ();
    int test_identify_clean_input ();
    int test_identify_error_input ();
    int test_tree_built_outside ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_PLANAR_TRIANGLE_H */