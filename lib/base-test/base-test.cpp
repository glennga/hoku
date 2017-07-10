/*
 * @file: base_test.cpp
 *
 * @brief: Serves as a base class for all testing classes.
 */

#include "base-test.h"

/*
 * Store the current time in seconds. Like resetting a timer to 0. Waiting one second before
 * proceeding to avoid truncating to zero.
 */
void BaseTest::reset_clock() {
    this->time_before_call = std::chrono::high_resolution_clock::now();
}

/*
 * Print the time elapsed from the last reset_clock call.
 *
 * @return 0 when finished.
 */
int BaseTest::display_time_elapsed() {
    auto time_after_call = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(time_after_call -
                                                                         time_before_call).count();
    std::cout << "Time Elapsed: " << elapsed << " uS" << std::endl;

    return 0;
}

/*
 * Print the results of the assertion. Expected to be true.
 *
 * @param assertion Boolean assertion, expected to be true.
 * @param test_name Name of test being performed.
 * @return True if assertion is true. False otherwise.
 */
bool BaseTest::assert_true(bool assertion, std::string test_name) {
    std::cout << "----------------------------------------------" << std::endl;
    if (assertion) {
        tests_passed.push_back(test_name);
        std::cout << test_name << " has passed." << std::endl;
    } else {
        std::cout << test_name << " has failed. Expected to be true." << std::endl;
    }
    display_time_elapsed();

    all_tests.push_back(test_name);
    return assertion;
}

/*
 * Print the results of the assertion. Expected to be false.
 *
 * @param assertion Boolean assertion, expected to be false.
 * @param test_name Name of test being performed.
 * @return True if assertion is false. False otherwise.
 */
bool BaseTest::assert_false(bool assertion, std::string test_name) {
    std::cout << "----------------------------------------------" << std::endl;
    if (!assertion) {
        tests_passed.push_back(test_name);
        std::cout << test_name << " has passed." << std::endl;
    } else {
        std::cout << test_name << " has failed. Expected to be false." << std::endl;
    }
    display_time_elapsed();

    all_tests.push_back(test_name);
    return !assertion;
}

/*
 * Determine if the two values are within delta units of each other.
 *
 * @param rho This value must be close to beta.
 * @param beta This value must be close to rho.
 * @param test_name Name of test being performed.
 * @param delta The difference of rho and beta must be less than this.
 * @return True if beta and rho are within delta units of each other. False otherwise.
 */
bool BaseTest::assert_equal(double rho, double beta, std::string test_name, double delta) {
    bool assertion = fabs(rho - beta) < delta;

    std::cout << "----------------------------------------------" << std::endl;
    if (assertion) {
        tests_passed.push_back(test_name);
        std::cout << test_name << " has passed." << std::endl;
    } else {
        std::cout << test_name << " has failed. Expected |rho - beta| < " << delta;
        std::cout << "." << std::endl;
    }
    display_time_elapsed();

    all_tests.push_back(test_name);
    return assertion;
}

/*
 * Determine if the two strings posses the same content.
 *
 * @param string_a This string must have the same contents as string_b.
 * @param string_b This string must have the same contents as string_a.
 * @param test_name Name of test being performed.
 * @return True if both strings are equal. False otherwize.
 */
bool BaseTest::assert_equal(std::string string_a, std::string string_b, std::string test_name) {
    bool assertion = string_a.compare(string_b) == 0;

    std::cout << "----------------------------------------------" << std::endl;
    if (assertion) {
        tests_passed.push_back(test_name);
        std::cout << test_name << " has passed." << std::endl;
    } else {
        std::cout << test_name << " has failed. Expected " << string_a << " == " << string_b;
        std::cout << "." << std::endl;
    }
    display_time_elapsed();

    all_tests.push_back(test_name);
    return assertion;
}

/*
 * Run through all tests defined in 'enumerate_tests'. After testing, print the results obtained
 * from test_passed and all_tests vectors.
 *
 * @return 0 when finished.
 */
int BaseTest::execute_tests() {
    double n_passed, n_ran;
    int test_case = 0;

    // loop through all tests, reset clock before every test
    do {
        reset_clock();
    } while (enumerate_tests(test_case++) != -1);

    // display total number of successes and failures
    n_passed = this->tests_passed.size();
    n_ran = this->all_tests.size();
    std::cout << "**********************************************" << std::endl;
    std::cout << "Results:" << std::endl;
    std::cout << n_passed << " / " << n_ran << " have passed." << std::endl;
    std::cout << (n_ran - n_passed) << " / " << n_ran << " have failed." << std::endl;
    std::cout << "**********************************************" << std::endl;
    std::cout << "**********************************************" << std::endl;

    return 0;
}
