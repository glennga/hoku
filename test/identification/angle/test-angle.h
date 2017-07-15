/*
 * @file: test-angle.h
 *
 * @brief: Header file for the TestAngle class, which tests the Angle class.
 */

#ifndef TEST_ANGLE_H
#define TEST_ANGLE_H

#include "base-test.h"
#include "angle.h"
#include <cstdio>

class TestAngle : public BaseTest {
    private:
        // test the pair query method
        void test_pair_query();
        void test_pair_multiple_choice_query();

        // test the candidate query method
        void test_candidate_fov_query();
        void test_candidate_none_query();
        void test_candidate_results_query();

        // test the rotating match finding method
        void test_rotating_match_correct_input();
        void test_rotating_match_error_input();
        void test_rotating_match_duplicate_input();

        // test the identification method
        void test_identify_clean_input();
        void test_identify_error_input();

    public:
        int enumerate_tests(int);
};

#endif /* TEST_ANGLE_H */