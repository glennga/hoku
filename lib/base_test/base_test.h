/*
 * @file: base_test.h
 *
 * @brief: Header file for BaseTest class, which is a base class for all testing classes.
 */

#ifndef BASE_TEST_H
#define BASE_TEST_H

#include <iostream>
#include <chrono>
#include <algorithm>

// global constant to enable all private fields to be viewed
#define DEBUGGING_MODE_IS_ON

/*
 * @class BaseTest
 * @brief Base class for all testing classes.
 */
class BaseTest {
    private:
        // clock, set before running call
        std::chrono::time_point<std::chrono::high_resolution_clock> time_before_call;

        // string vector of passed tests, and of total tests
        std::vector<std::string> tests_passed;
        std::vector<std::string> all_tests;

        // start the clock, display time elapsed
        void reset_clock();
        int display_time_elapsed();

    protected:
        // basic assertions for true, false, float equality and string equality, print result
        bool assert_true(bool, std::string);
        bool assert_false(bool, std::string);
        bool assert_equal(double, double, std::string, double=BaseTest::PRECISION_DEFAULT);
        bool assert_equal(std::string, std::string, std::string);

        // default precision of all float comparisons
        constexpr static double PRECISION_DEFAULT = 0.00000000001;

        // ideally, user must enumerate all tests defined in class
        virtual int enumerate_tests(int) = 0;

    public:
        // execute the tests and display the results
        int execute_tests();

};

#endif /* BASE_TEST_H */