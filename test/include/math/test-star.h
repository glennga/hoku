/// @file test-star.h
/// @author Glenn Galvizo
///
/// Header file for the TestStar class, which tests the Star class.

#ifndef TEST_STAR_H
#define TEST_STAR_H

#include "base-test/base-test.h"
#include "math/star.h"
#include <cstdio>

class TestStar : public BaseTest {
  private:
    int test_constructor_no_unit ();
    int test_constructor_unit ();
    int test_get_operators ();
    int test_plus_operator ();
    int test_minus_operator ();
    int test_scale_operator ();
    int test_norm_computation ();
    int test_unit_norm ();
    int test_unit_zero_star ();
    int test_equality_same ();
    int test_equality_precision ();
    int test_chance_unit ();
    int test_chance_hr ();
    int test_chance_duplicate ();
    int test_dot_computation_1 ();
    int test_dot_computation_2 ();
    int test_cross_computation_1 ();
    int test_cross_computation_2 ();
    int test_angle_computation_1 ();
    int test_angle_computation_2 ();
    int test_angle_within_check ();
    int test_angle_out_check ();
    int test_angle_same ();
    int test_hr_clear ();
    int test_angle_within_multiple_stars ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_STAR_H */