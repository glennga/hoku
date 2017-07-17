/*
 * @file: test-chomp.h
 *
 * @brief: Header file for the TestChomp class, which tests the TestChomp namespace.
 */

#ifndef TEST_CHOMP_H
#define TEST_CHOMP_H

#include "base-test.h"
#include "chomp.h"
#include <cstdio>

class TestChomp : public BaseTest {
    private:
        // test regular queries vs k-vector queries
        void test_regular_query();
        void test_k_vector_query();

    public:
        int enumerate_tests(int);
};

#endif /* TEST_CHOMP_H */