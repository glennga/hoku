/// @file test-nibble.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestNibble class, as well as the main function to run the tests.


#include "storage/test-nibble.h"

/// Check that components are correctly parsed from the given line.
///
/// @return 0 when finished.
int TestNibble::test_components_from_line () {
    std::array<double, 6> a = {6.55083333333333, -43.68, 0.718486460056107, 0.0825069799257624, -0.690630005849423,
        3.94};
    std::string b = " 100   Kap PheCD-44  101   2262215092                       002117"
        ".1-441405002612.2-434048318.42-72.68 3.94  +0.17 +0.11 +0.08   A7V+0.109+0.029 +.072+011      219";
    std::array<double, 6> c = Nibble().components_from_line(b);
    
    assert_equal(a[0], c[0], "ComponentFromLineAlpha", 0.000001);
    assert_equal(a[1], c[1], "ComponentFromLineDelta", 0.000001);
    assert_equal(a[2], c[2], "ComponentFromLineI");
    assert_equal(a[3], c[3], "ComponentFromLineJ");
    assert_equal(a[4], c[4], "ComponentFromLineK");
    return 0 * assert_equal(a[5], c[5], "ComponentM", 0.01);
}

/// Check that Nibble database and ASCII catalog is present after running Nibble::generate_bsc5_table.
///
/// @return 0 when finished.
int TestNibble::test_file_existence () {
    Nibble().generate_bsc5_table();
    std::ifstream catalog(Nibble().CATALOG_LOCATION), nibble(Nibble().DATABASE_LOCATION);
    
    assert_true(catalog.good(), "CatalogExistence", Nibble().CATALOG_LOCATION);
    return 0 * assert_true(nibble.good(), "DatabaseExistence", Nibble().DATABASE_LOCATION);
}

/// Check that the BSC5 table is present after running Nibble::generate_bsc5_table.
///
/// @return 0 when finished.
int TestNibble::test_bsc5_table_existence () {
    return 0 * assert_equal(Nibble().generate_bsc5_table(), -1, "BSC5TableExistence");
}

/// Check that the BSC5 query method returns the expected values.
///
/// @return 0 when finished.
int TestNibble::test_bsc5_query_result () {
    Star a = Nibble().query_bsc5(3);
    
    assert_equal(a[0], 0.994772975556659, "BSC5QueryComponentI");
    assert_equal(a[1], 0.0231608361523004, "BSC5QueryComponentJ");
    return 0 * assert_equal(a[2], -0.0994500013618795, "BSC5QueryComponentK");
}

/// Check that the BSC5 table can be queried using the general search method.
///
/// @return 0 when finished.
int TestNibble::test_table_search_result () {
    Nibble nb;
    std::vector<double> a = nb.search_table("hr = 3", "i, j, k", 3);
    std::vector<double> b = nb.search_table("hr = 3 or hr = 4", "i, j, k", 6, 2);
    
    assert_equal(a[0], 0.994772975556659, "GeneralBSC5QueryComponentI");
    assert_equal(a[1], 0.0231608361523004, "GeneralBSC5QueryComponentJ");
    assert_equal(a[2], -0.0994500013618795, "GeneralBSC5QueryComponentK");
    assert_equal(b[0], 0.994772975556659, "GeneralBSC5QueryLimit2ComponentI");
    assert_equal(b[1], 0.0231608361523004, "GeneralBSC5QueryLimit2ComponentJ");
    return 0 * assert_equal(b[2], -0.0994500013618795, "GeneralBSC5QueryLimit2ComponentK");
}

/// Check that the correct result is found by indexing the return of 'search_table'.
///
/// @return 0 when finished.
int TestNibble::test_table_search_result_index () {
    Nibble nb;
    std::vector<double> a = nb.search_table("hr = 3 or hr = 4", "i, j, k", 6);
    std::vector<double> b = nb.table_results_at(a, 3, 0);
    std::vector<double> c = nb.table_results_at(a, 3, 1);
    
    assert_equal(b[0], 0.994772975556659, "ResultReturnIndex0ComponentI");
    assert_equal(b[1], 0.0231608361523004, "ResultReturnIndex0ComponentJ");
    assert_equal(b[2], -0.0994500013618795, "ResultReturnIndex0ComponentK");
    assert_equal(c[0], 0.97249075430388, "ResultReturnIndex1ComponentI");
    assert_equal(c[1], 0.0241917492431918, "ResultReturnIndex1ComponentJ");
    return 0 * assert_equal(c[2], 0.231681876852775, "ResultReturnIndex1ComponentK");
}

/// Check that the BSC5 table has an index created.
///
/// @return 0 when finished.
int TestNibble::test_table_polish_index () {
    Nibble nb;
    nb.polish_table("alpha");
    bool assertion = false;
    
    try {
        SQLite::Statement(*nb.db, "CREATE INDEX BSC5_alpha on BSC5(alpha)").exec();
    }
    catch (std::exception &e) {
        // Exception thrown while creating index means that the index exists.
        std::cout << "Exception: " << e.what() << std::endl;
        assertion = true;
    }
    
    // Delete new table and index. Rerun original bsc5 table generation.
    SQLite::Transaction transaction(*nb.db);
    SQLite::Statement(*nb.db, "DROP INDEX BSC5_alpha").exec();
    SQLite::Statement(*nb.db, "DROP TABLE BSC5").exec();
    transaction.commit();
    nb.generate_bsc5_table();
    
    return 0 * assert_true(assertion, "IndexBSC5AlphaExistence");
}

/// Check that the BSC5 table has an index created.
///
/// @return 0 when finished.
int TestNibble::test_table_polish_sort () {
    Nibble nb;
    nb.polish_table("alpha");
    std::vector<double> a = nb.search_table("ROWID = 1", "hr", 1);
    assert_equal(a[0], 9081, "IndexBSC5AlphaSort");
    
    // Delete new table and index. Rerun original BSC5 table generation.
    try {
        SQLite::Transaction transaction(*nb.db);
        SQLite::Statement(*nb.db, "DROP INDEX BSC5_alpha").exec();
        SQLite::Statement(*nb.db, "DROP TABLE BSC5").exec();
        transaction.commit();
        nb.generate_bsc5_table();
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
    std::vector<double> a{0, 0, 0, 0, 0, 0, 10000000}, b;
    SQLite::Transaction transaction(*nb.db);
    
    nb.insert_into_table("alpha, delta, i, j, k, m, hr", a);
    SQLite::Statement query(*nb.db, "SELECT alpha, delta FROM BSC5 WHERE hr = 10000000");
    while (query.executeStep()) {
        b.push_back(query.getColumn(0).getDouble());
        b.push_back(query.getColumn(1).getDouble());
    }
    
    assert_equal(b[0], 0, "TableInsertionAlpha");
    assert_equal(b[1], 0, "TableInsertionDelta");
    
    try {
        SQLite::Statement(*nb.db, "DELETE FROM BSC5 WHERE hr = 10000000").exec();
        transaction.commit();
    }
    catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}

/// Check that the results returned from all_bsc5_stars are correct.
///
/// @return 0 when finished.
int TestNibble::test_bsc5_all_stars_grab () {
    Nibble nb;
    Star::list a = nb.all_bsc5_stars();
    Star b = nb.query_bsc5(3), c = nb.query_bsc5(4), d = nb.query_bsc5(5);
    Star e = nb.query_bsc5(12), f = nb.query_bsc5(9110);
    
    assert_equal(a[0], b, "BSCStarGrab3", a[0].str() + "," + b.str());
    assert_equal(a[1], c, "BSCStarGrab4", a[1].str() + "," + c.str());
    assert_equal(a[2], d, "BSCStarGrab5", a[2].str() + "," + d.str());
    assert_equal(a[5], e, "BSCStarGrab12", a[5].str() + "," + e.str());
    return 0 * assert_equal(a[5028], f, "BSCStarGrab9110", a[5028].str() + "," + f.str());
}

/// Check that the first 10 stars returned are all nearby the focus.
///
/// @return 0 when finished.
int TestNibble::test_nearby_star_grab () {
    Nibble nb;
    Star focus = Star::chance();
    std::vector<Star> nearby = nb.nearby_stars(focus, 7.5, 30);
    
    for (int q = 0; q < 10; ++q) {
        assert_true(Star::within_angle(nearby[q], focus, 7.5), "CandidateNearFocus" + std::to_string(q),
                    nearby[q].str() + "," + focus.str() + ",7.5");
    }
    
    return 0;
}

/// Check that the in-memory connection of Nibble works identically to the disk connection.
///
/// @return 0 when finished.
int TestNibble::test_in_memory_instance () {
    Nibble nb ("BSC5", "hr");
    Star a = nb.query_bsc5(3);
    
    assert_equal(a[0], 0.994772975556659, "BSC5QueryComponentIInMemory");
    assert_equal(a[1], 0.0231608361523004, "BSC5QueryComponentJInMemory");
    return 0 * assert_equal(a[2], -0.0994500013618795, "BSC5QueryComponentKInMemory");
}

/// Enumerate all tests in TestNibble.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestNibble::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_components_from_line();
        case 1: return test_file_existence();
        case 2: return test_bsc5_table_existence();
        case 3: return test_bsc5_query_result();
        case 4: return test_table_search_result();
        case 5: return test_table_search_result_index();
        case 6: return test_table_polish_index();
        case 7: return test_table_polish_sort();
        case 8: return test_table_insertion();
        case 9: return test_bsc5_all_stars_grab();
        case 10: return test_nearby_star_grab();
        case 11: return test_in_memory_instance();
        default: return -1;
    }
}

/// Run the tests in TestNibble. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    return TestNibble().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
