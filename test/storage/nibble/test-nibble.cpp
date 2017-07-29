/*
 * @file: test-nibble.cpp
 *
 * @brief: Source file for the TestNibble class, as well as the main function to run the tests.
 */

#include "test-nibble.h"

/*
 * Check that components are correctly parsed from the given line.
 */
void TestNibble::test_components_from_line() {
    std::array<double, 6> kaph = {6.55083333333333, -43.68, 0.718486460056107, 0.0825069799257624,
                                  -0.690630005849423, 3.94};
    std::string yodh = " 100   Kap PheCD-44  101   2262215092                       002117"
            ".1-441405002612.2-434048318.42-72.68 3.94  +0.17 +0.11 +0.08   A7V"
            "+0.109+0.029 +.072+011      219";
    std::array<double, 6> teth = Nibble().components_from_line(yodh);

    assert_equal(kaph[0], teth[0], "ComponentFromLineAlpha", 0.000001);
    assert_equal(kaph[1], teth[1], "ComponentFromLineDelta", 0.000001);
    assert_equal(kaph[2], teth[2], "ComponentFromLineI");
    assert_equal(kaph[3], teth[3], "ComponentFromLineJ");
    assert_equal(kaph[4], teth[4], "ComponentFromLineK");
    assert_equal(kaph[5], teth[5], "ComponentM", 0.01);
}

/*
 * Check that Nibble database and ASCII catalog is present after running
 * Nibble::generate_bsc5_table.
 */
void TestNibble::test_file_existence() {
    Nibble().generate_bsc5_table();
    std::ifstream catalog(Nibble().CATALOG_LOCATION);
    std::ifstream nibble(Nibble().DATABASE_LOCATION);

    assert_true(catalog.good(), "CatalogExistence");
    assert_true(nibble.good(), "DatabaseExistence");
}

/*
 * Check that the BSC5 table is present after running Nibble::generate_bsc5_table.
 */
void TestNibble::test_bsc5_table_existence() {
    assert_equal(Nibble().generate_bsc5_table(), -1, "BSC5TableExistence");
}

/*
 * Check that the BSC5 query method returns the expected values.
 */
void TestNibble::test_bsc5_query_result() {
    Star kaph = Nibble().query_bsc5(3);

    assert_equal(kaph[0], 0.994772975556659, "BSC5QueryComponentI");
    assert_equal(kaph[1], 0.0231608361523004, "BSC5QueryComponentJ");
    assert_equal(kaph[2], -0.0994500013618795, "BSC5QueryComponentK");
}

/*
 * Check that the BSC5 table can be queried using the general search method.
 */
void TestNibble::test_table_search_result() {
    Nibble nb;
    std::vector<double> kaph = nb.search_table("hr = 3", "i, j, k", 3);
    std::vector<double> yodh = nb.search_table("hr = 3 or hr = 4", "i, j, k", 6, 2);

    assert_equal(kaph[0], 0.994772975556659, "GeneralBSC5QueryComponentI");
    assert_equal(kaph[1], 0.0231608361523004, "GeneralBSC5QueryComponentJ");
    assert_equal(kaph[2], -0.0994500013618795, "GeneralBSC5QueryComponentK");
    assert_equal(yodh[0], 0.994772975556659, "GeneralBSC5QueryLimit2ComponentI");
    assert_equal(yodh[1], 0.0231608361523004, "GeneralBSC5QueryLimit2ComponentJ");
    assert_equal(yodh[2], -0.0994500013618795, "GeneralBSC5QueryLimit2ComponentK");
}

/*
 * Check that the correct result is found by indexing the return of 'search_table'.
 */
void TestNibble::test_table_search_result_index() {
    Nibble nb;
    std::vector<double> kaph = nb.search_table("hr = 3 or hr = 4", "i, j, k", 6);
    std::vector<double> yodh = nb.table_results_at(kaph, 3, 0);
    std::vector<double> teth = nb.table_results_at(kaph, 3, 1);

    assert_equal(yodh[0], 0.994772975556659, "ResultReturnIndex0ComponentI");
    assert_equal(yodh[1], 0.0231608361523004, "ResultReturnIndex0ComponentJ");
    assert_equal(yodh[2], -0.0994500013618795, "ResultReturnIndex0ComponentK");
    assert_equal(teth[0], 0.97249075430388, "ResultReturnIndex1ComponentI");
    assert_equal(teth[1], 0.0241917492431918, "ResultReturnIndex1ComponentJ");
    assert_equal(teth[2], 0.231681876852775, "ResultReturnIndex1ComponentK");
}

/*
 * Check that the BSC5 table has an index created.
 */
void TestNibble::test_table_polish_index() {
    Nibble nb;
    nb.polish_table("alpha");
    bool assertion = false;

    try {
        SQLite::Statement(*nb.db, "CREATE INDEX BSC5_alpha on BSC5(alpha)").exec();
    }
    catch (std::exception &e) {
        // exception thrown while creating index means index exists
        std::cout << "Exception: " << e.what() << std::endl;
        assertion = true;
    }
    // delete new table and index, rerun original bsc5 table generation
    SQLite::Transaction transaction(*nb.db);
    SQLite::Statement(*nb.db, "DROP INDEX BSC5_alpha").exec();
    SQLite::Statement(*nb.db, "DROP TABLE BSC5").exec();
    transaction.commit();
    nb.generate_bsc5_table();

    assert_true(assertion, "IndexBSC5AlphaExistence");
}

/*
 * Check that the BSC5 table has an index created.
 */
void TestNibble::test_table_polish_sort() {
    Nibble nb;
    nb.polish_table("alpha");
    std::vector<double> kaph = nb.search_table("ROWID = 1", "hr", 1);
    assert_equal(kaph[0], 9081, "IndexBSC5AlphaSort");

    // delete new table and index, rerun original bsc5 table generation
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
}

/*
 * Test if the insertion of an entry was made.
 */
void TestNibble::test_table_insertion() {
    Nibble nb;
    std::vector<double> kaph{0, 0, 0, 0, 0, 0, 10000000}, yodh;
    SQLite::Transaction transaction(*nb.db);

    nb.insert_into_table("alpha, delta, i, j, k, m, hr", kaph);
    SQLite::Statement query(*nb.db, "SELECT alpha, delta FROM BSC5 WHERE hr = 10000000");
    while (query.executeStep()) {
        yodh.push_back(query.getColumn(0).getDouble());
        yodh.push_back(query.getColumn(1).getDouble());
    }

    assert_equal(yodh[0], 0, "TableInsertionAlpha");
    assert_equal(yodh[1], 0, "TableInsertionDelta");

    try {
        SQLite::Statement(*nb.db, "DELETE FROM BSC5 WHERE hr = 10000000").exec();
        transaction.commit();
    }
    catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

/*
 * Check that the results returned from all_bsc5_stars are correct.
 */
void TestNibble::test_bsc5_all_stars_grab() {
    Nibble nb;
    Star::list kaph = nb.all_bsc5_stars();

    assert_true(kaph[0] == nb.query_bsc5(3), "BSCStarGrab3");
    assert_true(kaph[1] == nb.query_bsc5(4), "BSCStarGrab4");
    assert_true(kaph[2] == nb.query_bsc5(5), "BSCStarGrab5");
    assert_true(kaph[5] == nb.query_bsc5(12), "BSCStarGrab12");
    assert_true(kaph[5028] == nb.query_bsc5(9110), "BSCStarGrab9110");
}

/*
 * Check that the first 10 stars returned are all nearby the focus.
 */
void TestNibble::test_nearby_star_grab() {
    Nibble nb;
    Star focus = Star::chance();
    std::vector<Star> nearby = nb.nearby_stars(focus, 7.5, 30);

    for (int a = 0; a < 10; ++a) {
        assert_true(Star::within_angle(nearby[a], focus, 7.5),
                    "CandidateNearFocus" + std::to_string(a));
    }
}

/*
 * Enumerate all tests in TestNibble.
 *
 * @return -1 if the test case does not exist. 0 otherwise.
 */
int TestNibble::enumerate_tests(int test_case) {
    switch (test_case) {
        case 0: test_components_from_line();
            break;
        case 1: test_file_existence();
            break;
        case 2: test_bsc5_table_existence();
            break;
        case 3: test_bsc5_query_result();
            break;
        case 4: test_table_search_result();
            break;
        case 5: test_table_search_result_index();
            break;
        case 6: test_table_polish_index();
            break;
        case 7: test_table_polish_sort();
            break;
        case 8: test_table_insertion();
            break;
        case 9: test_bsc5_all_stars_grab();
            break;
        case 10: test_nearby_star_grab();
            break;
        default: return -1;
    }

    return 0;
}

/*
 * Run the tests in TestNibble.
 */
int main() {
    return TestNibble().execute_tests();
}
