/// @file test-angle.h
/// @author Glenn Galvizo
///
/// Header file for the TestAngle class, which tests the Angle class.

#ifndef TEST_ANGLE_H
#define TEST_ANGLE_H

#include "base-test.h"
#include "angle.h"

class TestAngle : public BaseTest {
  private:
    int test_pair_query ();
    int test_pair_multiple_choice_query ();
    int test_candidate_fov_query ();
    int test_candidate_none_query ();
    int test_candidate_results_query ();
    int test_rotating_match_correct_input ();
    int test_rotating_match_error_input ();
    int test_rotating_match_duplicate_input ();
    int test_identify_clean_input ();
    int test_identify_error_input ();
    int test_saturation_match ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_ANGLE_H */