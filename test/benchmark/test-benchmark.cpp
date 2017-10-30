/// @file test-benchmark.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestBenchmark class, as well as the main function to run the tests.

// Give us access to all private members.
#define ENABLE_TESTING_ACCESS

#include "benchmark/test-benchmark.h"

/// Check that the stars are not in the same order after shuffling.
///
/// @return 0 when finished.
int TestBenchmark::test_star_shuffle () {
    std::random_device seed;
    Chomp ch;
    
    Benchmark input(ch, seed, 15);
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
    std::random_device seed;
    Chomp ch;
    
    Star a = Star::chance(seed);
    Rotation b = Rotation::chance(seed);
    Benchmark input(ch, seed, a, b, 15);
    Star c = Rotation::rotate(a, b);
    std::string d;
    char e[200];
    
    std::remove(input.CURRENT_TMP.c_str());
    std::remove(input.ERROR_TMP.c_str());
    input.record_current_plot();
    
    std::ifstream current_plot_from_input(input.CURRENT_TMP);
    assert_true(current_plot_from_input.good(), "CurrentPlotFileOpen", input.CURRENT_TMP);
    
    std::getline(current_plot_from_input, d);
    assert_equal(15, std::stoi(d), "CurrentPlotFOVEquality");
    std::getline(current_plot_from_input, d);
    assert_equal(1, std::stod(d), "CurrentPlotNormEquality");
    
    std::getline(current_plot_from_input, d);
    sprintf(e, "%0.16f %0.16f %0.16f ", c[0], c[1], c[2]);
    assert_equal(d, std::string(e), "CurrentPlotFocusEquality", 2);
    
    std::getline(current_plot_from_input, d);
    sprintf(e, "%0.16f %0.16f %0.16f %d", input.stars[0][0], input.stars[0][1], input.stars[0][2],
            input.stars[0].get_label());
    return 0 * assert_equal(d, std::string(e), "CurrentPlotStar0Equality", 2);
}

/// Check that the file error_plot.dat is formatted correctly.
///
/// @return 0 when finished.
int TestBenchmark::test_error_plot_file () {
    std::random_device seed;
    Chomp ch;
    
    Benchmark input(ch, seed, 15);
    std::string a;
    char b[200];
    
    std::remove(input.CURRENT_TMP.c_str());
    std::remove(input.ERROR_TMP.c_str());
    input.add_extra_light(1);
    input.record_current_plot();
    
    std::ifstream error_plot_from_input(input.ERROR_TMP);
    assert_true(error_plot_from_input.good(), "ErrorPlotFileOpen", input.ERROR_TMP);
    
    // NOTE: Here b truncates a digit, but this is correct otherwise.
    std::getline(error_plot_from_input, a);
    sprintf(b, "%0.16f %0.16f %0.16f %d %s", input.error_models[0].affected[0][0], input.error_models[0].affected[0][1],
            input.error_models[0].affected[0][2], input.error_models[0].affected[0].get_label(),
            input.error_models[0].plot_color.c_str());
    return 0 * assert_equal(a, std::string(b), "ErrorPlotExtraLightEquality", 2);
}

/// Check that all error models place stars near focus.
///
/// @return 0 when finished.
int TestBenchmark::test_error_near_focus () {
    std::random_device seed;
    Chomp ch;
    
    Benchmark input(ch, seed, 15);
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
    std::random_device seed;
    Chomp ch;
    
    Benchmark input(ch, seed, 15);
    unsigned long long a = input.stars.size();
    input.add_extra_light(3);
    
    return 0 * assert_equal(input.stars.size(), a + 3, "ExtraLightAddedStars");
}

/// Check that stars have been removed in light removal method.
///
/// @return 0 when finished.
int TestBenchmark::test_removed_light_removed () {
    std::random_device seed;
    Chomp ch;
    
    Benchmark input(ch, seed, 15);
    unsigned long long a = input.stars.size();
    input.remove_light(3, 15);
    
    return 0 * assert_less_than(input.stars.size(), a, "RemoveLightRemovedStars");
}

/*
 * Check that stars have been shifted in light shift method.
 */
/// Check that the HR numbers of all stars are equal to 0.
///
/// @return 0 when finished.
int TestBenchmark::test_shifted_light_shifted () {
    std::random_device seed;
    Chomp ch;
    
    Benchmark input(ch, seed, 15);
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

/// Check that the catalog ID numbers of all stars are equal to 0.
///
/// @return 0 when finished.
int TestBenchmark::test_label_clear () {
    std::random_device seed;
    Chomp ch;
    Benchmark input(ch, seed, 15);
    std::vector<Star> a = input.clean_stars();
    
    for (int q = 0; q < 3; q++) {
        std::string test_name = "CatalogIDClearStar" + std::to_string(q + 1);
        assert_equal(a[q].get_label(), 0, test_name);
    }
    
    return 0;
}

/// We are not checking anything here- this is where the user visually checks and ensures that the plot is displayed
/// properly.
///
/// @return 0 when finished.
int TestBenchmark::test_display_plot () {
    std::random_device seed;
    Chomp ch;
    
    Benchmark input(ch, seed, 15);
    input.add_extra_light(2);
    input.shift_light(2, 10);
    
    input.display_plot();
    return 0;
}

/// Check that the correct number of stars are returned from the "compare" function.
///
/// @return 0 when finished.
int TestBenchmark::test_compare_stars () {
    std::random_device seed;
    Chomp ch;
    
    Benchmark a(ch, seed, 15);
    Star::list b = a.stars;
    
    // Erase two stars from set B.
    b.erase(b.begin() + 0);
    b.erase(b.begin() + 1);
    b.emplace_back(Star(5, 5, 5));
    
    assert_not_equal(a.stars.size(), Benchmark::compare_stars(a, b), "ComparePresentsNotCorrectNumber");
    assert_equal(a.stars.size(), Benchmark::compare_stars(a, b) + 2, "ComparePresentsCorrectNumber");
    
    return 0;
}

/// Check that an error star exists at the front of the stars vector when cap_error is raised.
///
/// @return 0 when finished.
int TestBenchmark::test_cap_error () {
    std::random_device seed;
    Chomp ch;
    
    Benchmark input(ch, seed, 15);
    std::vector<Star> a = input.stars;
    input.shift_light(1, 0.1, true);
    
    for (const Star &original : a) {
        if (input.stars[0] == original) {
            return 0 * assert_true(false, "ErrorStarAtFront");
        }
    }
    return 0 * assert_true(true, "ErrorStarAtFront");
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
        case 7: return test_label_clear();
        case 8: return test_display_plot();
        case 9: return test_compare_stars();
        case 10: return test_cap_error();
        default: return -1;
    }
}

/// Run the tests in TestBenchmark. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    return TestBenchmark().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
