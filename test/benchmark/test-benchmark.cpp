/*
 * @file: test-benchmark.cpp
 *
 * @brief: Source file for the TestBenchmark class, as well as the main function to run the tests.
 */

#include "test-benchmark.h"

/*
 * Check that the stars are not in the same order after shuffling.
 */
void TestBenchmark::test_star_shuffle() {
    Benchmark input(15, Star::chance(), Rotation::chance());
    Star a = input.stars[0], b(0, 0, 0);

    // shuffle set twice
    input.shuffle();
    b = input.stars[0];
    input.shuffle();

    assert_not_equal(a, b, "ShuffledSetStarShuffle1", a.str() + "," + b.str());
    assert_not_equal(b, input.stars[0], "ShuffledSetStarShuffle2",
                     b.str() + "," + input.stars[0].str());
}

/*
 * Check that the file current_plot.dat is formatted correctly.
 */
void TestBenchmark::test_current_plot_file() {
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
    sprintf(e, "%f %f %f %d", input.stars[0][0], input.stars[0][1], input.stars[0][2],
            input.stars[0].get_hr());
    assert_equal(d, std::string(e), "CurrentPlotStar0Equality", 2);
}

/*
 * Check that the file error_plot.dat is formatted correctly.
 */
void TestBenchmark::test_error_plot_file() {
    Benchmark input(15, Star::chance(), Rotation::chance());
    std::string a;
    char b[200];

    std::remove(input.CURRENT_PLOT.c_str());
    std::remove(input.ERROR_PLOT.c_str());
    input.add_extra_light(1);
    input.record_current_plot();

    std::ifstream error_plot_from_input(input.ERROR_PLOT.c_str());
    assert_true(error_plot_from_input.good(), "ErrorPlotFileOpen", input.ERROR_PLOT);

    // NOTE: here b truncates a digit, but this is correct otherwise
    std::getline(error_plot_from_input, a);
    sprintf(b, "%f %f %f %d %s", input.error_models[0].affected[0][0],
            input.error_models[0].affected[0][1], input.error_models[0].affected[0][2],
            input.error_models[0].affected[0].get_hr(), input.error_models[0].plot_color.c_str());
    assert_equal(a, std::string(b), "ErrorPlotExtraLightEquality", 2);
}

/*
 * Check that all error models place stars near focus.
 */
void TestBenchmark::test_error_near_focus() {
    Benchmark input(15, Star::chance(), Rotation::chance());
    input.add_extra_light(3);
    input.remove_light(3, 4);
    input.shift_light(3, 1);

    for (int a = 0; a < 5; a++) {
        std::string test_name = "CandidateNearFocusStar" + std::to_string(a + 1);
        assert_true(Star::within_angle(input.stars[a], input.focus, input.fov / 2), test_name,
                    input.stars[a].str() + "," + input.focus.str() + "," +
                    std::to_string(input.fov / 2.0));
    }
}

/*
 * Check that extra stars exist in the light adding method.
 */
void TestBenchmark::test_extra_light_added() {
    Benchmark input(15, Star::chance(), Rotation::chance());
    int a = input.stars.size();
    input.add_extra_light(3);

    assert_equal(input.stars.size(), a + 3, "ExtraLightAddedStars");
}

/*
 * Check that stars have been removed in light removal method.
 */
void TestBenchmark::test_removed_light_removed() {
    Benchmark input(15, Star::chance(), Rotation::chance());
    unsigned int a = input.stars.size();
    input.remove_light(3, 4);
    
    assert_less_than(input.stars.size(), a, "RemoveLightRemovedStars");
}

/*
 * Check that stars have been shifted in light shift method.
 */
void TestBenchmark::test_shifted_light_shifted() {
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
    assert_equal(a.size() * input.stars.size(), b + a.size() - 3, "ShiftLightShiftedStars");
}

/*
 * Check that the HR numbers of all stars are equal to 0.
 */
void TestBenchmark::test_hr_number_clear() {
    Benchmark input(15, Star::chance(), Rotation::chance());
    std::vector<Star> a = input.clean_stars();

    for (int q = 0; q < 3; q++) {
        std::string test_name = "HRNumberClearStar" + std::to_string(q + 1);
        assert_equal(a[q].get_hr(), 0, test_name);
    }
}

/*
 * Enumerate all tests in TestBenchmark.
 *
 * @return -1 if the test case does not exist. 0 otherwise.
 */
int TestBenchmark::enumerate_tests(int test_case) {
    switch (test_case) {
        case 0: test_star_shuffle();
            break;
        case 1: test_current_plot_file();
            break;
        case 2: test_error_plot_file();
            break;
        case 3: test_error_near_focus();
            break;
        case 4: test_extra_light_added();
            break;
        case 5: test_removed_light_removed();
            break;
        case 6: test_shifted_light_shifted();
            break;
        case 7: test_hr_number_clear();
            break;
        default: return -1;
    }

    return 0;
}

/*
 * Run the tests in TestBenchmark. Currently set to print and log all data.
 */
int main() {
    return TestBenchmark().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
