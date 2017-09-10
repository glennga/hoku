/// @file test-spherical-triangle.h
/// @author Glenn Galvizo
///
/// Header file for the TestSphericalTriangle class, which tests the SphericalTriangle class.


#ifndef TEST_SPHERICAL_TRIANGLE_H
#define TEST_SPHERICAL_TRIANGLE_H

#include "base-test/base-test.h"
#include "../../../include/identification/spherical-triangle.h"
#include <cstdio>

class TestSphericalTriangle : public BaseTest {
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
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_SPHERICAL_TRIANGLE_H */