/// @file base-test.h
/// @author Glenn Galvizo
///
/// Header file for BaseTest class, which is a base class for all testing classes.

#ifndef BASE_TEST_H
#define BASE_TEST_H

#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <memory>
#include <cmath>
#include <vector>

/// Base class (abstract virtual) for all testing classes. Includes logging and printing functionality.
///
/// @example
/// @code{.cpp}
/// /// Class TestX tests the X class.
/// class TestX : private BaseTest {
///     int test_x_is_not_zero() {
///         X a;
///         assert_not_equal(a.x_value, 0, "XIsNotZero");
///         // All tests MUST return an integer not equal to -1.
///         return 0 * assert_true(a.x_is_not_zero(), "XIsNotZeroMethod", a.str());
///     }
///
///     int test_x_not_within_bounds() {
///         assert_not_within(X().x_value, -4, -5, "XIsNotWithinNegativeFourAndNegativeFive");
///         return 0 * assert_not_within(X().x_value, -1, 10, "XIsNotWithinNegativeOneAndTen");
///     }
///
///     int enumerate_tests(const int test) {
///         switch(test) {
///             case 0: return test_x_is_not_zero();
///             case 1: return test_x_not_within_bounds();
///             default: return -1;
///         }
///     }
///
/// // Run all the tests in TestX. Print all data to console. Log all data.
/// int main() return TestX.execute_tests(Flavor::FULL_PRINT_LOG_ON);
/// @endcode
class BaseTest {
  public:
    /// Defines the type of output for the tests - logging and printing.
    enum Flavor {
        NO_PRINT_LOG_OFF, ///< Do not print to console. Do not log to file.
        MINIMAL_PRINT_LOG_OFF, ///< Print minimal data to console. Do not log to file.
        FULL_PRINT_LOG_OFF, ///< Print all data to console. Do not log to file.
        NO_PRINT_LOG_ON, ///< Do not print to console. Log to file.
        MINIMAL_PRINT_LOG_ON, ///< Print minimal data to console. Log to file.
        FULL_PRINT_LOG_ON ///< Print all data to console. Log to file.
    };
  
  public:
    int execute_tests (const Flavor = Flavor::FULL_PRINT_LOG_ON, const int = -1);
  
  protected:
    bool assert_true (bool, const std::string &, const std::string & = "");
    bool assert_false (bool, const std::string &, const std::string & = "");
    
    bool assert_equal (double, double, const std::string &, double= BaseTest::PRECISION_DEFAULT);
    bool assert_not_equal (double, double, const std::string &, double= BaseTest::PRECISION_DEFAULT);
    
    bool assert_less_than (double, double, const std::string &);
    bool assert_greater_than (double, double, const std::string &);
    
    bool assert_equal (const std::string &, const std::string &, const std::string &, int = 0);
    bool assert_not_equal (const std::string &, const std::string &, const std::string &, int = 0);
    
    bool assert_within (double, double, double, const std::string &);
    bool assert_not_within (double, double, double, const std::string &);
    
    virtual int enumerate_tests (int) = 0;
  
  protected:
    /// Determine if the first value is equal to the second. Push this assertion to our test stack. If desired, print
    /// the results.
    ///
    /// @tparam T Type of parameters 'a' and 'b' must be (they must be of the same type).
    /// @param a Datum to compare with 'b'. 'a' must equal 'b'.
    /// @param b Datum to compare with 'a'. 'a' must equal 'b'.
    /// @param test_name Name of the current test being performed.
    /// @param log_data Comma separated data to log onto a file.
    /// @return True if 'a == b' holds. False otherwise.
    template <typename T>
    bool assert_equal (const T &a, const T &b, const std::string &test_name, const std::string &log_data) {
        log_current(a == b, test_name + ",GenericEqualAssertion", log_data);
        return push_results(a == b, test_name, "A == B.", "\'A == B\' is not true.");
    }
    
    /// Determine if the first value is not equal to the second. Push this assertion to our test stack. If desired,
    /// print the results.
    ///
    /// @tparam T Type of parameters 'a' and 'b' must be (they must be of the same type).
    /// @param a Datum to compare with 'b'. 'a' must not equal 'b'.
    /// @param b Datum to compare with 'a'. 'a' must not equal 'b'.
    /// @param test_name Name of the current test being performed.
    /// @param log_data Comma separated data to log onto a file.
    /// @return True if 'a == b' does not hold. False otherwise.
    template <typename T>
    bool assert_not_equal (const T &a, const T &b, const std::string &test_name, const std::string &log_data) {
        log_current(!(a == b), test_name + ",GenericNotEqualAssertion", log_data);
        return push_results(!(a == b), test_name, "\'A == B\' is not true.", "A == B.");
    }
    
    /// Determine if the first datum exists in the vector of datum. Push this assertion to our test stack. If desired,
    /// print the results.
    ///
    /// @tparam T 'e' must of type T. 's' must be a vector of type T.
    /// @param e Datum that must exist in s.
    /// @param s Vector of datum which must hold e.
    /// @param test_name Name of the current test being performed.
    /// @param log_data Comma separated data to log onto a file.
    /// @return True if e exists in s. False otherwise.
    template <typename T>
    bool assert_inside (const T &e, const std::vector<T> &s, const std::string &test_name,
                        const std::string &log_data) {
        std::string test_and_assertion = test_name + ",GenericElementWithinContainer";
        log_current(std::find(s.begin(), s.end(), e) != s.end(), test_and_assertion, log_data);
        
        return push_results(std::find(s.begin(), s.end(), e) != s.end(), test_name, "E exists in S.",
                            "E does not exist in S.");
    }
    
    /// Determine if the first datum does not exist in the vector of datum. Push this assertion to our test stack. If
    /// desired, print the results.
    ///
    /// @tparam T 'e' must of type T. 's' must be a vector of type T.
    /// @param e Datum that must not exist in s.
    /// @param s Vector of datum which must not hold e.
    /// @param test_name Name of the current test being performed.
    /// @param log_data Comma separated data to log onto a file.
    /// @return True if e does not exist in s. False otherwise.
    template <typename T>
    bool assert_outside (const T &e, const std::vector<T> &s, const std::string &test_name,
                         const std::string &log_data) {
        std::string test_and_assertion = test_name + ",GenericElementNotWithinContainer";
        log_current(std::find(s.begin(), s.end(), e) == s.end(), test_and_assertion, log_data);
        
        return push_results(std::find(s.begin(), s.end(), e) == s.end(), test_name, "E does not exist in S.",
                            "E exists in S.");
    }
    
    /// Default precision for all float comparisons.
    constexpr static double PRECISION_DEFAULT = 0.00000000001;
  
  private:
    int print_summary ();
    int print_current (const std::string &);
    
    int log_current (const bool, const std::string &, const std::string &);
    bool push_results (const bool, const std::string &, const std::string &, const std::string &);
  
  private:
    /// String for content headers (assertions vs summaries).
    const char *CONTENT_HEADER = "***********************************************************";
    
    // String for section headers (different assertion calls).
    const char *SECTION_HEADER = "-----------------------------------------------------------";
    
    /// Output stream to the log file. Non-copyable, so we need a pointer to one on the heap.
    std::shared_ptr<std::ofstream> log;
    
    /// Current flavor. Defines how to print data to console, and how to log data to a file.
    Flavor f;
    
    /// Alias for the clock in the Chrono library.
    using clock = std::chrono::high_resolution_clock;
    
    /// Time point before a test call. Clock is reset before a test call with enumerate_tests.
    std::chrono::time_point<clock> time_before_call;
    
    /// Collection of passed test names. Does not hold any information about the assertions.
    std::vector<std::string> tests_passed;
    
    /// Collection of all test names. Does not hold any information about the assertions.
    std::vector<std::string> all_tests;
};

#endif /* BASE_TEST_H */
