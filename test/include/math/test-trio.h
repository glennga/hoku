/// @file test-trio.h
/// @author Glenn Galvizo
///
/// Header file for the TestTrio class, which tests the Trio class.

#ifndef TEST_TRIO_H
#define TEST_TRIO_H

#include "base-test/base-test.h"
#include "math/trio.h"

class TestTrio : public BaseTest {
  private:
    int test_planar_length_computation ();
    int test_spherical_length_computation ();
    int test_semi_perimeter_computation ();
    int test_planar_area_computation ();
    int test_planar_moment_computation ();
    int test_spherical_area_computation ();
    int test_planar_centroid_computation ();
    int test_spherical_moment_computation ();
    int view_planar_triangle_shifts ();
    int view_spherical_triangle_shifts ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_TRIO_H */