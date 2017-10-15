/// @file test-chomp.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestChomp class, as well as the main function to run the tests.

#include "storage/test-chomp.h"

/// Check that the results returned from bright_as_list are correct.
///
/// @return 0 when finished.
int TestChomp::test_bright_star_grab () {
    Chomp ch(true);
    Star::list a = ch.bright_as_list();
    
    Star b = ch.query_hip(88), c = ch.query_hip(107), d = ch.query_hip(122);
    Star e = ch.query_hip(124), f = ch.query_hip(117930);
    
    assert_equal(a[0], b, "BrightStarGrab88", a[0].str() + "," + b.str());
    assert_equal(a[1], c, "BrightStarGrab107", a[1].str() + "," + c.str());
    assert_equal(a[2], d, "BrightStarGrab122", a[2].str() + "," + d.str());
    assert_equal(a[3], e, "BrightStarGrab124", a[3].str() + "," + e.str());
    return 0 * assert_equal(a[Chomp::BRIGHT_TABLE_LENGTH - 1], f, "BrightStarGrab117930",
                            a[Chomp::BRIGHT_TABLE_LENGTH - 1].str() + "," + f.str());
}

/// Check that the first 10 bright stars returned are all nearby the focus.
///
/// @return 0 when finished.
int TestChomp::test_nearby_bright_star_grab () {
    std::random_device seed;
    Chomp ch(true);
    Star focus = Star::chance(seed);
    std::vector<Star> nearby = ch.nearby_bright_stars(focus, 7.5, 30);
    
    for (int q = 0; q < 10; q++) {
        assert_true(Star::within_angle(nearby[q], focus, 7.5), "BrightCandidateNearFocus" + std::to_string(q),
                    nearby[q].str() + "," + focus.str() + ",7.5");
    }
    
    return 0;
}

/// Check that the first 10 stars returned are all nearby the focus.
///
/// @return 0 when finished.
int TestChomp::test_nearby_hip_star_grab() {
    std::random_device seed;
    Chomp ch(true);
    Star focus = Star::chance(seed);
    std::vector<Star> nearby = ch.nearby_hip_stars(focus, 5, 100);
    
    for (int q = 0; q < 10; q++) {
        assert_true(Star::within_angle(nearby[q], focus, 5), "HipCandidateNearFocus" + std::to_string(q),
                    nearby[q].str() + "," + focus.str() + ",7.5");
    }
    
    return 0;
}


/// Check that components are correctly parsed from the given line.
///
/// @return 0 when finished.
int TestChomp::test_components_from_line () {
    std::array<double, 6> a = {0.000911850889839031, 1.08901336539477, 0.999819374779962, 1.59119257019658e-05,
        0.0190057244380963, 9.20429992675781};
    std::string b = "     1|  5|0|1| 0.0000159148  0.0190068680|   4.55|   -4.55|   -1.19|  1.29|  0.66|  1.33|  1.25| "
        " 0.75| 90| 0.91| 0|   0.0|   0| 9.2043|0.0020|0.017|0| 0.482|0.025| 0.550|   1.19  -0.71   1.00  -0.02   0.02 "
        "1.00   0.45  -0.05   0.03   1.09  -0.41   0.09   0.08  -0.60   1.00";
    std::array<double, 6> c = Chomp().components_from_line(b);
    
    assert_equal(a[0], c[0], "ComponentFromLineAlpha", 0.000001);
    assert_equal(a[1], c[1], "ComponentFromLineDelta", 0.000001);
    assert_equal(a[2], c[2], "ComponentFromLineI");
    assert_equal(a[3], c[3], "ComponentFromLineJ");
    assert_equal(a[4], c[4], "ComponentFromLineK");
    return 0 * assert_equal(a[5], c[5], "ComponentM", 0.01);
}

/// Check that the both the bright stars table and the hip table are present after running the generators.
///
/// @return 0 when finished.
int TestChomp::test_star_table_existence () {
    Chomp ch;
    ch.generate_hip_table();
    ch.generate_bright_table();
    
    assert_equal(ch.generate_hip_table(), -1, "HipTableExistence");
    return 0 * assert_equal(ch.generate_bright_table(), -1, "BrightTableExistence");
}

/// Check that the hip query method returns the expected values.
///
/// @return 0 when finished.
int TestChomp::test_hip_query_result () {
    Star a = Chomp().query_hip(3);
    
    assert_equal(a[0], 0.778689180572338, "HipQueryComponentI");
    assert_equal(a[1], 6.80614031952957e-05, "HipQueryComponentJ");
    return 0 * assert_equal(a[2], 0.627409878330925, "HipQueryComponentK");
}

/// Check that the regular query returns correct results. This test is just used to compare against the k-vector
/// query time.
///
/// @return 0 when finished.
int TestChomp::test_regular_query () {
    Nibble nb;
    
    nb.select_table("PYRA_20");
    Nibble::tuples_d a = nb.search_table("theta", "theta BETWEEN 5.004 and 5.005", 90, 30);
    
    for (unsigned int q = 0; q < a.size(); q++) {
        std::string test_name = "RegularQueryResultWithinBoundsSet" + std::to_string(q + 1);
        assert_within(a[q][0], 5.003, 5.006, test_name);
    }
    
    return 0;
}

/// Check that the k-vector query returns the correct results.
///
/// @return 0 when finished.
int TestChomp::test_k_vector_query () {
    Chomp ch;
    
    ch.select_table("PYRA_20");
    Nibble::tuples_d a = ch.k_vector_query("theta", "theta", 5.004, 5.005, 90);
    
    for (unsigned int q = 0; q < a.size(); q++) {
        std::string test_name = "KVectorQueryResultWithinBoundsSet" + std::to_string(q + 1);
        assert_within(a[q][0], 5.003, 5.006, test_name);
    }
    
    return 0;
}

/// Enumerate all tests in TestChomp.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestChomp::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_bright_star_grab();
        case 1: return test_nearby_bright_star_grab();
        case 2: return test_components_from_line();
        case 3: return test_star_table_existence();
        case 4: return test_hip_query_result();
        case 5: return test_regular_query();
        case 6: return test_k_vector_query();
        default: return -1;
    }
}

/// Run the tests in TestChomp. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    return TestChomp().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
