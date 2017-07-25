/*
 * @file: test-chomp.cpp
 *
 * @brief: Source file for the TestChomp namespace, as well as the main function to run the tests.
 */

#include "test-chomp.h"

/*
 * Check that the regular query returns correct results. This test is just used to compare
 * against the k-vector query time.
 */
void TestChomp::test_regular_query() {
    Nibble nb;
    std::vector<double> kaph;

    nb.select_table("SEP20");
    kaph = nb.search_table("theta BETWEEN 5.004 and 5.005", "theta", 90, 30);

    for (unsigned int a = 0; a < kaph.size(); a++) {
        std::string test_name = "RegularQueryResultWithinBoundsSet" + std::to_string(a + 1);
        assert_true(kaph[a] > 5.003 && kaph[a] < 5.006, test_name);
    }
}

/*
 * Check that the k-vector query returns the correct results.
 */
void TestChomp::test_k_vector_query() {
    Chomp ch;
    std::vector<double> kaph;

    ch.select_table("SEP20");
    kaph = ch.k_vector_query("theta", "theta", 5.004, 5.004, 90);

    for (unsigned int a = 0; a < kaph.size(); a++) {
        std::string test_name = "KVectorQueryResultWithinBoundsSet" + std::to_string(a + 1);
        assert_true(kaph[a] > 5.003 && kaph[a] < 5.006, test_name);
    }
}

/*
 * Enumerate all tests in TestChomp.
 *
 * @return -1 if the test case does not exist. 0 otherwise.
 */
int TestChomp::enumerate_tests(int test_case) {
    switch (test_case) {
        case 0: test_regular_query();
            break;
        case 1: test_k_vector_query();
            break;
        default: return -1;
    }

    return 0;
}

/*
 * Run the tests in TestChomp.
 */
int main() {
    return TestChomp().execute_tests();
}
