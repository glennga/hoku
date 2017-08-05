/*
 * @file: base_test.h
 *
 * @brief: Header file for BaseTest class, which is a base class for all testing classes.
 */

#ifndef BASE_TEST_H
#define BASE_TEST_H

#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <memory>

/*
 * @class BaseTest
 * @brief Base class for all testing classes.
 */
class BaseTest {
    public:
        // define the the type of output for the tests
        enum Flavor {
            NO_PRINT_LOG_OFF, MINIMAL_PRINT_LOG_OFF, FULL_PRINT_LOG_OFF,
            NO_PRINT_LOG_ON, MINIMAL_PRINT_LOG_ON, FULL_PRINT_LOG_ON
        };

        // execute the tests based on the given
        int execute_tests(const Flavor = Flavor::FULL_PRINT_LOG_ON, const int = -1);

    protected:
        // basic true/false assertions
        bool assert_true(bool, const std::string &, const std::string & = "");
        bool assert_false(bool, const std::string &, const std::string & = "");

        // float equal assertion, can adjust precision
        bool assert_equal(const double, const double, const std::string &,
                          const double= BaseTest::PRECISION_DEFAULT);

        // float not-equal assertion, can adjust precision
        bool assert_not_equal(const double, const double, const std::string &,
                              const double= BaseTest::PRECISION_DEFAULT);

        // float less than assertion
        bool assert_less_than(const double, const double, const std::string &);

        // float greater than assertion
        bool assert_greater_than(const double, const double, const std::string &);

        // string equal assertion, can adjust precision
        bool assert_equal(const std::string &, const std::string &, const std::string &,
                          const int = 0);

        // string not-equal assertion, can adjust precision
        bool assert_not_equal(const std::string &, const std::string &, const std::string &,
                              const int = 0);

        // float within assertion, first number must be within the next two
        bool assert_within(const double, const double, const double, const std::string &);

        // float not-within assertion, first number must not be within the next two
        bool assert_not_within(const double, const double, const double, const std::string &);

        // generic equal assertion using == operator
        template<typename T>
        bool assert_equal(const T &a, const T &b, const std::string &test_name,
                          const std::string &log_data) {
            log_current(a == b, test_name + ",GenericEqualAssertion", log_data);
            return push_results(a == b, test_name, "A == B.", "\'A == B\' is not true.");
        }

        // generic not-equal assertion using == operator
        template<typename T>
        bool assert_not_equal(const T &a, const T &b, const std::string &test_name,
                              const std::string &log_data) {
            log_current(!(a == b), test_name + ",GenericNotEqualAssertion", log_data);
            return push_results(!(a == b), test_name, "\'A == B\' is not true.", "A == B.");
        }

        // generic element in vector assertion
        template<typename T>
        bool assert_in_container(const T &e, const std::vector<T> &s, const std::string &test_name,
                                 const std::string &log_data) {
            log_current(std::find(s.begin(), s.end(), e) != s.end(),
                        test_name + ",GenericElementWithinContainer", log_data);
            return push_results(std::find(s.begin(), s.end(), e) != s.end(), test_name,
                                "E exists in S.", "E does not exist in S.");
        }

        // generic element not in vector assertion
        template<typename T>
        bool assert_not_in_container(const T &e, const std::vector<T> &s,
                                     const std::string &test_name, const std::string &log_data) {
            log_current(std::find(s.begin(), s.end(), e) == s.end(),
                        test_name + ",GenericElementNotWithinContainer", log_data);
            return push_results(std::find(s.begin(), s.end(), e) == s.end(), test_name,
                                "E does not exist in S.", "E exists in S.");
        }

        // default precision of all float comparisons
        constexpr static double PRECISION_DEFAULT = 0.00000000001;

        // ideally, user must enumerate all tests defined in class
        virtual int enumerate_tests(const int) = 0;

    private:
        // print result summary, define headers
        const char *CONTENT_HEADER = "***********************************************************";
        const char *SECTION_HEADER = "-----------------------------------------------------------";
        int print_summary();

        // log the current test results into a csv file
        int log_current(const bool, const std::string &, const std::string &);

        // push the results, and print the current test results
        int print_current(const std::string &);
        bool push_results(const bool, const std::string &, const std::string &,
                          const std::string &);

        // log stream (if desired), and current testing option
        std::shared_ptr<std::ofstream> log;
        Flavor f;

        // clock, set before running call
        using clock = std::chrono::high_resolution_clock;
        std::chrono::time_point<clock> time_before_call;

        // container of passed tests and total tests
        std::vector<std::string> tests_passed;
        std::vector<std::string> all_tests;
};

#endif /* BASE_TEST_H */