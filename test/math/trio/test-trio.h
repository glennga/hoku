/*
 * @file: test_trio.h
 *
 * @brief: Header file for the TestTrio class, which tests the Trio class.
 */

#ifndef TEST_TRIO_H
#define TEST_TRIO_H

#include "base-test.h"
#include "trio.h"
#include <cstdio>

class TestTrio : public BaseTest {
    private:
        // test the length grabbing methods
        void test_planar_length_computation();
        void test_spherical_length_computation();

        // test the semi perimeter method
        void test_semi_perimeter_computation();

        // test the planar area and moment methods
        void test_planar_area_computation();
        void test_planar_moment_computation();

        // test the spherical area method
        void test_spherical_area_computation();

        // test planar centroid method
        void test_planar_centroid_computation();

        // test triangle cut method
        void test_cut_triangle_computation();

        // test spherical moment method
        void test_spherical_moment_computation();

    public:
        int enumerate_tests(int);
};

#endif /* TEST_TRIO_H */