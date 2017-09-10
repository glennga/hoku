/// @file test-mercator.h
/// @author Glenn Galvizo
///
/// Header file for the TestMercator class, which tests the Mercator class.

#ifndef TEST_MERCATOR_H
#define TEST_MERCATOR_H

#include "base-test/base-test.h"
#include "../../../include/math/mercator.h"

class TestMercator : public BaseTest {
  private:
    int test_projection_within_bounds ();
    int test_corners_form_box ();
    int test_is_within_bounds ();
    int test_distance_between ();
    int test_bracket_operator ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_MERCATOR_H */