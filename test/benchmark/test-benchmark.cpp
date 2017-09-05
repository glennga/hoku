/// @file test-benchmark.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestBenchmark class, as well as the main function to run the tests.

#include "test-benchmark.h"

/// Check that the stars are not in the same order after shuffling.
///
/// @return 0 when finished.
int TestBenchmark::test_star_shuffle () {
    Benchmark input(15, Star::chance(), Rotation::chance());
    Star a = input.stars[0], b(0, 0, 0);
    
    input.shuffle();
    b = input.stars[0];
    input.shuffle();
    
    assert_not_equal(a, b, "ShuffledSetStarShuffle1", a.str() + "," + b.str());
    return 0 * assert_not_equal(b, input.stars[0], "ShuffledSetStarShuffle2", b.str() + "," + input.stars[0].str());
}

/// Check that the file current_plot.dat is formatted correctly.
///
/// @return 0 when finished.
int TestBenchmark::test_current_plot_file () {
    Star a = Star::chance();
    Rotation b = Rotation::chance();
    Benchmark input(15, a, b);
    Star c = Rotation::rotate(a, b);
    std::string d;
    char e[200];
    
    std::remove(input.CURRENT_PLOT.c_str());
    std::remove(input.ERROR_PLOT.c_str());
    input.record_current_plot();
    
    std::ifstream current_plot_from_input(input.CURRENT_PLOT.c_str());
    assert_true(current_plot_from_input.good(), "CurrentPlotFileOpen", input.CURRENT_PLOT);
    
    std::getline(current_plot_from_input, d);
    assert_equal(15, std::stoi(d.c_str()), "CurrentPlotFOVEquality");
    std::getline(current_plot_from_input, d);
    assert_equal(1, std::stod(d.c_str()), "CurrentPlotNormEquality");
    
    std::getline(current_plot_from_input, d);
    sprintf(e, "%f %f %f ", c[0], c[1], c[2]);
    assert_equal(d, std::string(e), "CurrentPlotFocusEquality", 2);
    
    std::getline(current_plot_from_input, d);
    sprintf(e, "%f %f %f %d", input.stars[0][0], input.stars[0][1], input.stars[0][2], input.stars[0].get_hr());
    return 0 * assert_equal(d, std::string(e), "CurrentPlotStar0Equality", 2);
}

/// Check that the file error_plot.dat is formatted correctly.
///
/// @return 0 when finished.
int TestBenchmark::test_error_plot_file () {
    Benchmark input(15, Star::chance(), Rotation::chance());
    std::string a;
    char b[200];
    
    std::remove(input.CURRENT_PLOT.c_str());
    std::remove(input.ERROR_PLOT.c_str());
    input.add_extra_light(1);
    input.record_current_plot();
    
    std::ifstream error_plot_from_input(input.ERROR_PLOT.c_str());
    assert_true(error_plot_from_input.good(), "ErrorPlotFileOpen", input.ERROR_PLOT);
    
    // NOTE: Here b truncates a digit, but this is correct otherwise.
    std::getline(error_plot_from_input, a);
    sprintf(b, "%f %f %f %d %s", input.error_models[0].affected[0][0], input.error_models[0].affected[0][1],
            input.error_models[0].affected[0][2], input.error_models[0].affected[0].get_hr(),
            input.error_models[0].plot_color.c_str());
    return 0 * assert_equal(a, std::string(b), "ErrorPlotExtraLightEquality", 2);
}

/// Check that all error models place stars near focus.
///
/// @return 0 when finished.
int TestBenchmark::test_error_near_focus () {
    Benchmark input(15, Star::chance(), Rotation::chance());
    input.add_extra_light(3);
    input.remove_light(3, 4);
    input.shift_light(3, 1);
    
    for (int a = 0; a < 5; a++) {
        std::string test_name = "CandidateNearFocusStar" + std::to_string(a + 1);
        assert_true(Star::within_angle(input.stars[a], input.focus, input.fov / 2), test_name,
                    input.stars[a].str() + "," + input.focus.str() + "," + std::to_string(input.fov / 2.0));
    }
    
    return 0;
}

/// Check that extra stars exist in the light adding method.
///
/// @return 0 when finished.
int TestBenchmark::test_extra_light_added () {
    Benchmark input(15, Star::chance(), Rotation::chance());
    unsigned long long a = input.stars.size();
    input.add_extra_light(3);
    
    return 0 * assert_equal(input.stars.size(), a + 3, "ExtraLightAddedStars");
}

/// Check that stars have been removed in light removal method.
///
/// @return 0 when finished.
int TestBenchmark::test_removed_light_removed () {
    Benchmark input(15, Star::chance(), Rotation::chance());
    unsigned long long a = input.stars.size();
    input.remove_light(3, 4);
    
    return 0 * assert_less_than(input.stars.size(), a, "RemoveLightRemovedStars");
}

/*
 * Check that stars have been shifted in light shift method.
 */
/// Check that the HR numbers of all stars are equal to 0.
///
/// @return 0 when finished.
int TestBenchmark::test_shifted_light_shifted () {
    Benchmark input(15, Star::chance(), Rotation::chance());
    std::vector<Star> a = input.stars;
    input.shift_light(3, 0.1);
    int b = 0;
    
    for (Star original : a) {
        for (Star modified : input.stars) {
            if (!(original == modified)) {
                b++;
            }
        }
    }
    
    // |original|*|modified| = (number of different pairs) + |original| - 3
    return 0 * assert_equal(a.size() * input.stars.size(), b + a.size() - 3, "ShiftLightShiftedStars");
}

/// Check that the HR numbers of all stars are equal to 0.
///
/// @return 0 when finished.
int TestBenchmark::test_hr_number_clear () {
    Benchmark input(15, Star::chance(), Rotation::chance());
    std::vector<Star> a = input.clean_stars();
    
    for (int q = 0; q < 3; q++) {
        std::string test_name = "HRNumberClearStar" + std::to_string(q + 1);
        assert_equal(a[q].get_hr(), 0, test_name);
    }
    
    return 0;
}

/// We are not checking anything here- this is where the user visually checks and ensures that the plot is displayed
/// properly.
///
/// @return 0 when finished.
int TestBenchmark::test_display_plot () {
    Benchmark input(15, Star::chance(), Rotation::chance());
    input.add_extra_light(2);
    input.shift_light(2, 0.1);
    
    input.display_plot();
    return 0;
}

/// Check that the a given benchmark is inserted and can be parsed correctly.
/// 
/// @return 
int TestBenchmark::test_nibble_insertion () {
    std::string schema = "set_n INT, item_n INT, e INT, r INT, s INT, i FLOAT, j FLOAT, k FLOAT, fov FLOAT";
    Benchmark input(15, Star::chance(), Rotation::chance());
    Nibble nb;
    
    // We insert our benchmark with the next available set_n.
    nb.select_table(Benchmark::TABLE_NAME);
    nb.create_table(Benchmark::TABLE_NAME, schema);
    unsigned int a = (unsigned int) nb.search_table("MAX(set_n)", 1, 1)[0] + 1;
    input.insert_into_nibble(nb, a);
    
    Benchmark b = Benchmark::parse_from_nibble(nb, a);
    
    assert_equal(input.focus, b.focus, "ParsedStarIsEqual", input.focus.str() + "," + b.focus.str());
    assert_equal(input.fov, b.fov, "ParsedFovIsEqual");
    
    if (assert_equal(input.stars.size(), b.stars.size(), "ParsedSizeSameAsOriginal")) {
        int d = (int) input.stars.size() - 1;
        assert_equal(input.stars[0], b.stars[0], "FirstStarIsEqual", input.stars[0].str() + "," + b.stars[0].str());
        assert_equal(input.stars[d], b.stars[d], "LastStarIsEqual", input.stars[d].str() + "," + b.stars[d].str());
    }
    
    // We were never here...
    (*nb.db).exec("DELETE FROM " + Benchmark::TABLE_NAME + " WHERE set_n = " + std::to_string(a));
    return 0;
}

/// Check that the correct number of stars are returned from the "compare" function.
///
/// @return 0 when finished.
int TestBenchmark::test_compare_stars() {
    Benchmark a(15, Star::chance(), Rotation::chance());
    Star::list b = a.stars;
    
    // Erase two stars from set B.
    b.erase(b.begin() + 0);
    b.erase(b.begin() + 1);
    b.push_back(Star(5, 5, 5));
    
    assert_not_equal(a.stars.size(), Benchmark::compare_stars(a, b), "ComparePresentsNotCorrectNumber");
    assert_equal(a.stars.size(), Benchmark::compare_stars(a, b) + 2, "ComparePresentsCorrectNumber");
    
    return 0;
}

/// Enumerate all tests in TestBenchmark.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestBenchmark::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_star_shuffle();
        case 1: return test_current_plot_file();
        case 2: return test_error_plot_file();
        case 3: return test_error_near_focus();
        case 4: return test_extra_light_added();
        case 5: return test_removed_light_removed();
        case 6: return test_shifted_light_shifted();
        case 7: return test_hr_number_clear();
        case 8: return test_display_plot();
        case 9: return test_nibble_insertion();
        case 10: return test_compare_stars();
        default: return -1;
    }
}

/// Run the tests in TestBenchmark. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    return TestBenchmark().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
