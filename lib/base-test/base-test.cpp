/// @file base-test.cpp
/// @author Glenn Galvizo
///
/// Source file for BaseTest, which serves as a base class for all testing classes.

#include "base-test.h"

/// Push the results of the given assertion onto the current assertion stack. If desired, print the current test output.
///
/// @param assertion If true, this pushes the current test on the 'passed' stack.
/// @param test_name Name of the current test being performed.
/// @param explain_pass Output string if the assertion is true.
/// @param explain_fail Output string if the assertion is false.
/// @return True if assertion is true. False otherwise.
bool BaseTest::push_results (const bool assertion, const std::string &test_name, const std::string &explain_pass,
                             const std::string &explain_fail) {
    if (assertion) {
        tests_passed.push_back(test_name);
        print_current(test_name + " has passed. " + explain_pass);
    }
    else {
        print_current(test_name + " has failed. " + explain_fail);
    }
    
    all_tests.push_back(test_name);
    return assertion;
}

/// Push the assertion to our test stack. If desired, print the results. Expected to be true.
///
/// @param assertion Boolean assertion, expected to be true.
/// @param test_name Name of test being performed.
/// @param parameters Data to be logged if desired.
/// @return True if assertion is true. False otherwise.
bool BaseTest::assert_true (bool assertion, const std::string &test_name, const std::string &parameters) {
    log_current(assertion, test_name + ",TrueAssertion", (assertion ? "true," : "false,") + parameters);
    return push_results(assertion, test_name, "Assertion is true.", "Assertion is false.");
}

/// Push the assertion to our test stack. If desired, print the results. Expected to be false.
///
/// @param assertion Boolean assertion, expected to be false.
/// @param test_name Name of test being performed.
/// @param parameters Data to be logged if desired.
/// @return True if assertion is false. False otherwise.
bool BaseTest::assert_false (bool assertion, const std::string &test_name, const std::string &parameters) {
    log_current(!assertion, test_name + ",FalseAssertion", (!assertion ? "true," : "false,") + parameters);
    return push_results(!assertion, test_name, "Assertion is false.", "Assertion is true.");
}

/// Determine if the the first value is less than the second. Push this assertion to our test stack. If desired, print
/// the results.
///
/// @param x This number must be less than a.
/// @param a This number must be greater than x.
/// @param test_name Name of test being performed.
/// @return True if x < a. False otherwise.
bool BaseTest::assert_less_than (const double x, const double a, const std::string &test_name) {
    log_current(x < a, test_name + ",FloatLessThanAssertion", std::to_string(x) + "," + std::to_string(a));
    return push_results(x < a, test_name, std::to_string(x) + " < " + std::to_string(a) + ".",
                        std::to_string(x) + " >= " + std::to_string(a) + ".");
}

/// Determine if the the first value is greater than the second. Push this assertion to our test stack. If desired,
/// print the results.
///
/// @param x This number must be greater than a.
/// @param a This number must be less than x.
/// @param test_name Name of test being performed.
/// @return True if x > a. False otherwise.
bool BaseTest::assert_greater_than (const double x, const double a, const std::string &test_name) {
    log_current(x > a, test_name + ",FloatGreaterThanAssertion", std::to_string(x) + "," + std::to_string(a));
    return push_results(x > a, test_name, std::to_string(x) + " > " + std::to_string(a) + ".",
                        std::to_string(x) + " <= " + std::to_string(a) + ".");
}

/// Determine if the two values are within delta units of each other. Push this assertion to our test stack. If desired,
/// print the results.
///
/// @param a This value must be close to b.
/// @param b This value must be close to a.
/// @param test_name Name of test being performed.
/// @param delta The difference of a and b must be less than this.
/// @return True if a and b are within delta units of each other. False otherwise.
bool BaseTest::assert_equal (double a, double b, const std::string &test_name, double delta) {
    std::ostringstream minimum;
    minimum << std::setprecision(16) << std::fixed << delta;
    std::string finding_delta = "|" + std::to_string(a) + " - " + std::to_string(b) + "|";
    
    log_current(fabs(a - b) < delta, test_name + ",FloatEqualAssertion",
                minimum.str() + "," + std::to_string(a) + "," + std::to_string(b));
    
    return push_results(fabs(a - b) < delta, test_name, finding_delta + " < " + minimum.str() + ".",
                        finding_delta + " >= " + minimum.str() + ".");
}

/// Determine if the two values are not within delta units of each other. Push this assertion to our test stack. If
/// desired, print the results.
///
/// @param a This value must not be close to b.
/// @param b This value must not be close to a.
/// @param test_name Name of test being performed.
/// @param delta The difference of a and b must not be less than this.
/// @return True if a and b are not within delta units of each other. False otherwise.
bool BaseTest::assert_not_equal (double a, double b, const std::string &test_name, double delta) {
    std::ostringstream minimum;
    minimum << std::setprecision(16) << std::fixed << delta;
    std::string finding_delta = "|" + std::to_string(a) + " - " + std::to_string(b) + "|";
    
    log_current(fabs(a - b) >= delta, test_name + ",FloatNotEqualAssertion",
                minimum.str() + "," + std::to_string(a) + "," + std::to_string(b));
    
    return push_results(fabs(a - b) < delta, test_name, finding_delta + " >= " + minimum.str() + ".",
                        finding_delta + " < " + minimum.str() + ".");
}

/// Determine if the two strings have delta characters different from each other. Push this assertion to our test
/// stack. If desired, print the results.
///
/// @param a This string must have the same contents as b.
/// @param b This string must have the same contents as a.
/// @param test_name Name of test being performed.
/// @param delta Number of characters allowed to be mismatched.
/// @return True if both strings are equal. False otherwise.
bool BaseTest::assert_equal (const std::string &a, const std::string &b, const std::string &test_name,
                             const int delta) {
    std::string a_prime = a, b_prime = b;
    
    // Remove all occurrences of commas for logging, as the log file is CSV.
    a_prime.erase(std::remove(a_prime.begin(), a_prime.end(), ','), a_prime.end());
    b_prime.erase(std::remove(b_prime.begin(), b_prime.end(), ','), b_prime.end());
    log_current(abs(a.compare(b)) <= delta, test_name + ",StringEqualAssertion",
                std::to_string(delta) + "," + a_prime + "," + b_prime);
    
    return push_results(abs(a.compare(b)) <= delta, test_name, "\'" + a + "\' is equivalent to \'" + b + "\'.",
                        "\'" + a + "\' is not equivalent to \'" + b + "\'.");
}

/// Determine if the two strings don't have delta characters different from each other. Push this assertion to our test
/// stack. If desired, print the results.
///
/// @param a This string must not have the same contents as b.
/// @param b This string must not have the same contents as a.
/// @param test_name Name of test being performed.
/// @param delta Number of characters not allowed to be mismatched.
/// @return True if both strings are not equal. False otherwise.
bool BaseTest::assert_not_equal (const std::string &a, const std::string &b, const std::string &test_name,
                                 const int delta) {
    std::string a_prime = a, b_prime = b;
    
    // Remove all occurrences of commas for logging, as the log file is CSV.
    a_prime.erase(std::remove(a_prime.begin(), a_prime.end(), ','), a_prime.end());
    b_prime.erase(std::remove(b_prime.begin(), b_prime.end(), ','), b_prime.end());
    log_current(abs(a.compare(b)) > delta, test_name + ",StringNotEqualAssertion",
                std::to_string(delta) + "," + a_prime + "," + b_prime);
    
    return push_results(abs(a.compare(b)) > delta, test_name, "\'" + a + "\' is not equivalent to \'" + b + ".",
                        "\'" + a + "\' is equivalent to \'" + b + ".");
}

/// Determine if the first value is within the bounds defined by the second and third. Push this assertion to our test
/// stack. If desired, print the results.
///
/// @param x This number must be between a and b.
/// @param a The lower bound for x.
/// @param b The upper bound for x.
/// @param test_name Name of the test being performed.
/// @return True if x in (a, b). False otherwise.
bool BaseTest::assert_within (const double x, const double a, const double b, const std::string &test_name) {
    std::string finding_within = std::to_string(a) + " < " + std::to_string(x) + " < " + std::to_string(b);
    log_current(a < x && x < b, test_name + ",FloatElementWithinBounds",
                std::to_string(x) + "," + std::to_string(a) + "," + std::to_string(b));
    
    return push_results(a < x && x < b, test_name, finding_within + " is true.", finding_within + " is false");
}

/// Determine if the first value is not within the bounds defined by the second and third. Push this assertion to our
/// test stack. If desired, print the results.
///
/// @param x This number must not be between a and b.
/// @param a The lower bound for x false zone.
/// @param b The upper bound for x false zone.
/// @param test_name Name of the test being performed.
/// @return True if x not in (a, b). False otherwise.
bool BaseTest::assert_not_within (const double x, const double a, const double b, const std::string &test_name) {
    std::string finding_within = std::to_string(a) + " < " + std::to_string(x) + " < " + std::to_string(b);
    log_current(a < x && x < b, test_name + ",FloatElementNotWithinBounds",
                std::to_string(x) + "," + std::to_string(a) + "," + std::to_string(b));
    
    return push_results(a < x && x < b, test_name, finding_within + " is true.", finding_within + " is false");
}

/// Log the current assertion if flavor is set to NO_PRINT_LOG_ON, MINIMAL_PRINT_LOG_ON, or FULL_PRINT_LOG_ON.
///
/// @param assertion The result of the assertion, logs true/false value.
/// @param name_type The name of the test, and the type of assertion separated by a comma.
/// @param compared Parameters of the assertion that should be logged, (comma separated).
/// @return 0 when finished.
int BaseTest::log_current (const bool assertion, const std::string &name_type, const std::string &compared) {
    auto t = clock::now() - time_before_call;
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t).count();
    
    // Do not log anything if desired.
    if (f == NO_PRINT_LOG_OFF || f == MINIMAL_PRINT_LOG_OFF || f == FULL_PRINT_LOG_OFF) {
        return 0;
    }
    
    // Otherwise, log the columns in order: Name-Type-Time-Pass/Fail-Parameters
    (*this->log) << name_type << "," << elapsed;
    (*this->log) << (assertion ? ",1," : ",0,");
    (*this->log) << compared << std::endl;
    
    return 0;
}

/// Print the string specified. Output dependent on set flavor.
///
/// @param minimal Minimal message to print.
/// @return 0 when finished.
int BaseTest::print_current (const std::string &minimal) {
    auto t = clock::now() - time_before_call;
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t).count();
    
    // Do not print anything if specified.
    if (f == NO_PRINT_LOG_OFF || f == NO_PRINT_LOG_ON) {
        return 0;
    }
    
    // Print the minimal data passed. End here if appropriate flavor.
    std::cout << SECTION_HEADER << std::endl << minimal << std::endl;
    if (f == MINIMAL_PRINT_LOG_OFF || f == MINIMAL_PRINT_LOG_ON) {
        return 0;
    }
    
    // Otherwise, print the time elapsed as well.
    std::cout << "Time Elapsed: " << elapsed << " uS" << std::endl;
    return 0;
}

/// Print a summary of the test results. Dependent on set flavor.
///
/// @return 0 when finished.
int BaseTest::print_summary () {
    double n_passed, n_ran;
    
    // Do not print anything if specified.
    if (f == NO_PRINT_LOG_OFF || f == NO_PRINT_LOG_ON) {
        return 0;
    }
    
    // Display the total number of successes and failures. This is included with the 'minimal' option.
    n_passed = this->tests_passed.size(), n_ran = this->all_tests.size();
    std::cout << CONTENT_HEADER << std::endl << "Summary:" << std::endl << n_passed << " / " << n_ran;
    std::cout << " have passed." << std::endl << (n_ran - n_passed) << " / " << n_ran << " have failed." << std::endl;
    std::cout << CONTENT_HEADER << std::endl << CONTENT_HEADER << std::endl;
    
    return 0;
}

/// Run through all tests defined in 'enumerate_tests'. After testing, print the results obtained from test_passed
/// and all_tests vectors.
///
/// @param f Print and logging option, wrapped in Flavor enum.
/// @param specific_test Only execute certain test if desired.
/// @return -1 if the log file cannot be opened. 0 otherwise.
int BaseTest::execute_tests (const Flavor f, const int specific_test) {
    using clock = std::chrono::system_clock;
    int test_case = 0;
    this->f = f;
    
    // If logging is enabled, open the file. Note that HOKU_PROJECT_PATH must be set.
    if (f == NO_PRINT_LOG_ON || f == MINIMAL_PRINT_LOG_ON || f == FULL_PRINT_LOG_ON) {
        std::string log_file;
        std::ostringstream l;
        
        // Construct the log file based on the HOKU_PROJECT_PATH environment variable.
        l << "/data/test/" << clock::to_time_t(clock::now() - std::chrono::hours(24)) << ".csv";
        log_file = std::string(std::getenv("HOKU_PROJECT_PATH")) + l.str();
        
        // Open this log file for writing. Non-copyable so we must share this data.
        this->log = std::make_shared<std::ofstream>(std::ofstream(log_file));
        if (!(*this->log).is_open()) {
            return -1;
        }
        
        // Append the columns to the CSV file.
        (*this->log) << "Name,Type,Time(uS),Pass/Fail,Parameters" << std::endl;
    }
    
    this->time_before_call = clock::now();
    
    // If specified, only run one test.
    if (specific_test != -1) {
        enumerate_tests(specific_test);
    }
    else {
        // Loop through all tests. Reset the clock before every test.
        while (enumerate_tests(test_case++) != -1) {
            this->time_before_call = clock::now();
        }
    }
    
    (*this->log).close();
    return print_summary();
}
