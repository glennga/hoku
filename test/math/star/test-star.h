/*
 * @file: test_star.h
 *
 * @brief: Header file for the TestStar class, which tests the Star class.
 */

#ifndef TEST_STAR_H
#define TEST_STAR_H

#include "base-test.h"
#include "star.h"
#include <cstdio>

class TestStar : public BaseTest {
    private:
        // test constructor method unit flag
        void test_constructor_no_unit();
        void test_constructor_unit();

        // test the get methods ([] overload)
        void test_get_operators();
        
        // test the add/subtract methods (+/- operator overload)
        void test_plus_operator();
        void test_minus_operator();

        // test the scale method (* operator overload)
        void test_scale_operator();

        // test the norm method
        void test_norm_computation();

        // test the unit method
        void test_unit_norm();
        void test_unit_zero_star();

        // test the equality checking method
        void test_equality_same();
        void test_equality_precision();

        // test the random star method
        void test_chance_unit();
        void test_chance_hr();
        void test_chance_duplicate();

        // test the dot and cross methods
        void test_dot_computation_1();
        void test_dot_computation_2();
        void test_cross_computation_1();
        void test_cross_computation_2();

        // test the angle calculation and check methods
        void test_angle_computation_1();
        void test_angle_computation_2();
        void test_angle_within_check();
        void test_angle_out_check();
        
        // test the HR number clearing method
        void test_hr_clear();

    public:
        int enumerate_tests(int);
};

#endif /* TEST_STAR_H */