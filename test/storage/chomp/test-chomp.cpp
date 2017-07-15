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
    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    std::vector<double> kaph;
    kaph = Nibble::search_table(db, "SEP20", "theta BETWEEN 5.004 and 5.005", "theta", 90, 30);

    for (int a = 0; a < kaph.size(); a++) {
        std::string test_name = "RegularQueryResultWithinBoundsSet" + std::to_string(a + 1);
        assert_true(kaph[a] > 5.003 && kaph[a] < 5.006, test_name);
    }
}

/*
 * Check that the k-vector query returns the correct results.
 */
void TestChomp::test_k_vector_query() {
    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    std::vector<double> kaph;
    kaph = Chomp::k_vector_query(db, "SEP20", "theta", "theta", 5.004, 5.005, 90);

    for (int a = 0; a < kaph.size(); a++) {
        std::string test_name = "KVectorQueryResultWithinBoundsSet" + std::to_string(a + 1);
        assert_true(kaph[a] > 5.003 && kaph[a] < 5.006, test_name);
    }
}

/*
 * Check that
 */

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
//    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
//    SQLite::Transaction t(db);
//    db.exec("DROP TABLE SEP20_KVEC");
//    t.commit();
//    Chomp::create_k_vector("SEP20", "theta");
//    auto a = Nibble::search_table(db, "SEP20", "theta BETWEEN ")
//    return 0;
    return TestChomp().execute_tests();
}
