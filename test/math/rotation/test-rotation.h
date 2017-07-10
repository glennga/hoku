/*
 * @file: test_rotation.h
 *
 * @brief: Header file for the TestRotation class, which tests the Rotation class.
 */

#ifndef TEST_ROTATION_H
#define TEST_ROTATION_H

#include "base-test.h"
#include "rotation.h"
#include <cstdio>

class TestRotation : public BaseTest {
    private:
        // test the private and public constructor methods
        void test_public_constructor();
        void test_private_constructor_row_set();
        void test_private_constructor_component_set();

        // test matrix operations
        void test_matrix_multiplication_transpose();
        void test_matrix_to_quaternion();

        // test quaternion properties
        void test_quaternion_double_cover_property();
        void test_quaternion_unit_property();

        // test rotation methods
        void test_rotate_logic();
        void test_rotation_identity();
        void test_triad_property_simple();
        void test_triad_property_random();
        void test_triad_multiple_stars();

    public:
        int enumerate_tests(int);
};

#endif /* TEST_ROTATION_H */