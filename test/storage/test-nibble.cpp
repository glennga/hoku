/// @file test-nibble.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestNibble class, as well as the main function to run the tests. This assumes that the bright
/// star table generator in Chomp works.

#include "storage/test-nibble.h"

/// Check that Nibble database is present after creating a Nibble instance.
///
/// @return 0 when finished.
int TestNibble::test_file_existence () {
    std::ifstream nibble(Nibble().DATABASE_LOCATION);
    
    return 0 * assert_true(nibble.good(), "DatabaseExistence", Nibble().DATABASE_LOCATION);
}

/// Check that the bright stars table can be queried using the general search method.
///
/// @return 0 when finished.
int TestNibble::test_table_search_result () {
    Chomp ch;
    (*ch.db).exec("DROP TABLE IF EXISTS " + ch.BRIGHT_TABLE);
    (*ch.db).exec("DROP TABLE IF EXISTS " + ch.HIP_TABLE);
    ch.generate_bright_table(), ch.generate_hip_table();
    
    Nibble nb;
    nb.select_table(ch.BRIGHT_TABLE);
    Nibble::tuples_d a = nb.search_table("i, j, k", "label = 88", 3);
    Nibble::tuples_d b = nb.search_table("i, j, k", "label = 88 or label = 107", 6, 2);
    
    assert_equal(a[0][0], 0.658552697278613, "GeneralBrightQueryComponentI");
    assert_equal(a[0][1], 0.00309371805098928, "GeneralBrightQueryComponentJ");
    assert_equal(a[0][2], -0.75252825449659, "GeneralBrightQueryComponentK");
    assert_equal(b[1][0], 0.638254649361992, "GeneralBrightQueryLimit2ComponentI");
    assert_equal(b[1][1], 0.00371847539671449, "GeneralBrightQueryLimit2ComponentJ");
    return 0 * assert_equal(b[1][2], -0.769816325826183, "GeneralBrightQueryLimit2ComponentK");
}

/// Check that the bright stars table has an index created.
///
/// @return 0 when finished.
int TestNibble::test_table_polish_index () {
    Chomp ch;
    Nibble nb;
    
    ch.generate_bright_table();
    nb.select_table(ch.BRIGHT_TABLE);
    nb.polish_table("alpha");
    bool assertion = false;
    
    try {
        SQLite::Statement(*nb.db, "CREATE INDEX HIP_BRIGHT_alpha on " + ch.BRIGHT_TABLE + "(alpha)").exec();
    }
    catch (std::exception &e) {
        // Exception thrown while creating index means that the index exists.
        std::cout << "Exception: " << e.what() << std::endl;
        assertion = true;
    }
    
    // Delete new table and index. Rerun original bright table generation.
    SQLite::Transaction transaction(*nb.db);
    SQLite::Statement(*nb.db, "DROP INDEX HIP_BRIGHT_alpha").exec();
    SQLite::Statement(*nb.db, "DROP TABLE " + ch.BRIGHT_TABLE).exec();
    transaction.commit();
    Chomp().generate_bright_table();
    
    return 0 * assert_true(assertion, "IndexBrightAlphaExistence");
}

/// Check that the bright stars table has an index created.
///
/// @return 0 when finished.
int TestNibble::test_table_polish_sort () {
    Chomp ch;
    Nibble nb;
    
    ch.generate_bright_table();
    nb.select_table(ch.BRIGHT_TABLE);
    nb.polish_table("delta");
    
    double a = nb.search_single("label", "rowid = 1");
    assert_equal(a, 104023, "IndexBRIGHTDeltaSort");
    
    // Delete new table and index. Rerun original BSC5 table generation.
    try {
        SQLite::Transaction transaction(*nb.db);
        SQLite::Statement(*nb.db, "DROP INDEX HIP_BRIGHT_delta").exec();
        SQLite::Statement(*nb.db, "DROP TABLE " + ch.BRIGHT_TABLE).exec();
        transaction.commit();
        ch.generate_bright_table();
    }
    catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}

/// Test if the insertion of an entry was made.
///
/// @return 0 when finished.
int TestNibble::test_table_insertion () {
    Nibble nb;
    Chomp ch;
    
    ch.generate_bright_table();
    std::vector<double> a{0, 0, 0, 0, 0, 0, 10000000}, b;
    SQLite::Transaction transaction(*nb.db);
    
    nb.select_table(ch.BRIGHT_TABLE);
    nb.insert_into_table("alpha, delta, i, j, k, m, label", a);
    SQLite::Statement query(*nb.db, "SELECT alpha, delta FROM " + ch.BRIGHT_TABLE + " WHERE label = 10000000");
    while (query.executeStep()) {
        b.push_back(query.getColumn(0).getDouble());
        b.push_back(query.getColumn(1).getDouble());
    }
    
    assert_equal(b[0], 0, "TableInsertionAlpha");
    assert_equal(b[1], 0, "TableInsertionDelta");
    
    try {
        SQLite::Statement(*nb.db, "DELETE FROM " + ch.BRIGHT_TABLE + " WHERE label = 10000000").exec();
        transaction.commit();
    }
    catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}

/// Check that the in-memory connection of Nibble works identically to the disk connection.
///
/// @return 0 when finished.
int TestNibble::test_in_memory_instance () {
    Chomp ch;
    ch.generate_bright_table();
    
    Nibble nb(ch.BRIGHT_TABLE, "label");
    Nibble::tuples_d a = nb.search_table("i, j, k", "label = 88", 3);
    
    assert_equal(a[0][0], 0.658552697278613, "BrightStarsQueryComponentIInMemory");
    assert_equal(a[0][1], 0.00309371805098928, "BrightStarsQueryComponentJInMemory");
    return 0 * assert_equal(a[0][2], -0.75252825449659, "BrightStarsQueryComponentKInMemory");
}

/// Enumerate all tests in TestNibble.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestNibble::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_file_existence();
        case 1: return test_table_search_result();
        case 2: return test_table_polish_index();
        case 3: return test_table_polish_sort();
        case 4: return test_table_insertion();
        case 5: return test_in_memory_instance();
        default: return -1;
    }
}

/// Run the tests in TestNibble. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    return TestNibble().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
