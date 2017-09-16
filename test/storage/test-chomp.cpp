/// @file test-chomp.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestChomp class, as well as the main function to run the tests.

#include "storage/test-chomp.h"

/// Check that the regular query returns correct results. This test is just used to compare against the k-vector
/// query time.
///
/// @return 0 when finished.
int TestChomp::test_regular_query () {
    std::vector<double> a;
    Nibble nb;
    
    nb.select_table("SEP_20");
    a = nb.search_table("theta BETWEEN 5.004 and 5.005", "theta", 90, 30);
    
    for (unsigned int q = 0; q < a.size(); q++) {
        std::string test_name = "RegularQueryResultWithinBoundsSet" + std::to_string(q + 1);
        assert_within(a[q], 5.003, 5.006, test_name);
    }
    
    return 0;
}

/// Check that the k-vector query returns the correct results.
///
/// @return 0 when finished.
int TestChomp::test_k_vector_query () {
    Chomp ch;
    std::vector<double> a;
    
    ch.select_table("SEP_20");
    a = ch.k_vector_query("theta", "theta", 5.004, 5.005, 90);
    
    for (unsigned int q = 0; q < a.size(); q++) {
        std::string test_name = "KVectorQueryResultWithinBoundsSet" + std::to_string(q + 1);
        assert_within(a[q], 5.003, 5.006, test_name);
    }
    
    return 0;
}

/// Enumerate all tests in TestChomp.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestChomp::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_regular_query();
        case 1: return test_k_vector_query();
        default: return -1;
    }
}

/// Run the tests in TestChomp. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    return TestChomp().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
