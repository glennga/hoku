/*
 * @file: test-triangle-planar.h
 *
 * @brief: Header file for the TestTrianglePlanar class, which tests the TrianglePlanar class.
 */

#ifndef TEST_PLANAR_TRIANGLE_H
#define TEST_PLANAR_TRIANGLE_H

#include "base-test.h"
#include "planar-triangle.h"
#include <cstdio>

class TestPlanarTriangle : public BaseTest {
    private:
        // test the trio query method
        void test_trio_query();
        void test_trio_multiple_choice_query();

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

#endif /* TEST_PLANAR_TRIANGLE_H */