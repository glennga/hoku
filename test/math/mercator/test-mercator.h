/*
 * @file: test-mercator.h
 *
 * @brief: Header file for the TestMercator class, which tests the Mercator class.
 */

#ifndef TEST_MERCATOR_H
#define TEST_MERCATOR_H

#include "base-test.h"
#include "mercator.h"
#include <cstdio>

class TestMercator : public BaseTest {
    private:
        // test the projection method
        void test_projection_within_bounds();

        // test the reduction method
        void test_reduction_within_bounds();
        
        // test the projection presentation method
        void test_present_projection();
        
        // test the formation of corners in find_corners
        void test_corners_form_box();
        
        // test the within_bounds method
        void test_is_within_bounds();

    public:
        int enumerate_tests(int);
};

#endif /* TEST_MERCATOR_H */