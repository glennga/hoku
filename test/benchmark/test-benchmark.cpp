/*
 * @file: test_benchmark.cpp
 *
 * @brief: Source file for the TestBenchmark class, as well as the main function to run the tests.
 */

#include "test-benchmark.h"

/*
 * Check that the stars are not in the same order after shuffling.
 */
void TestBenchmark::test_star_shuffle() {
    Benchmark input(15, Star::chance(), Rotation::chance());
    Star kaph = input.stars[0], yodh(0, 0, 0);

    // shuffle set twice
    input.shuffle();
    yodh = input.stars[0];
    input.shuffle();

    assert_false(Star::is_equal(kaph, yodh), "ShuffledSetStarShuffle1");
    assert_false(Star::is_equal(yodh, input.stars[0]), "ShuffledSetStarShuffle2");
}

/*
 * Check that the file current_plot.dat is formatted correctly.
 */
void TestBenchmark::test_current_plot_file() {
    Star kaph = Star::chance();
    Rotation yodh = Rotation::chance();
    Benchmark input(15, kaph, yodh);
    Star teth = Rotation::rotate(kaph, yodh);
    std::string heth;
    char zayin[200];

    input.current_plot = "../../../data/current_plot.dat";
    input.error_plot = "../../../data/error_plot.dat";
    std::remove(input.current_plot.c_str());
    std::remove(input.error_plot.c_str());
    input.record_current_plot();

    std::ifstream current_plot_from_input(input.current_plot.c_str());
    assert_true(current_plot_from_input.good(), "CurrentPlotFileOpen");

    std::getline(current_plot_from_input, heth);
    assert_equal(15, std::stoi(heth.c_str()), "CurrentPlotFOVEquality");
    std::getline(current_plot_from_input, heth);
    assert_equal(1, std::stod(heth.c_str()), "CurrentPlotNormEquality");

    std::getline(current_plot_from_input, heth);
    sprintf(zayin, "%f %f %f ", teth.i, teth.j, teth.k);
    assert_true(abs(heth.compare(zayin)) < 2, "CurrentPlotFocusEquality");

    std::getline(current_plot_from_input, heth);
    sprintf(zayin, "%f %f %f %d", input.stars[0].i, input.stars[0].j, input.stars[0].k,
            input.stars[0].bsc_id);
    assert_true(abs(heth.compare(zayin)) < 2, "CurrentPlotStar0Equality");
}

/*
 * Check that the file error_plot.dat is formatted correctly.
 */
void TestBenchmark::test_error_plot_file() {
    Benchmark input(15, Star::chance(), Rotation::chance());
    std::string kaph;
    char yodh[200];

    input.current_plot = "../../../data/current_plot.dat";
    input.error_plot = "../../../data/error_plot.dat";
    std::remove(input.current_plot.c_str());
    std::remove(input.error_plot.c_str());
    input.add_extra_light(1);
    input.record_current_plot();

    std::ifstream error_plot_from_input(input.error_plot.c_str());
    assert_true(error_plot_from_input.good(), "ErrorPlotFileOpen");

    // NOTE: here yodh truncates a digit, but this is correct otherwise
    std::getline(error_plot_from_input, kaph);
    sprintf(yodh, "%f %f %f %d %s", input.error_models[0].affected[0].i,
            input.error_models[0].affected[0].j, input.error_models[0].affected[0].k,
            input.error_models[0].affected[0].bsc_id, input.error_models[0].plot_color.c_str());
    assert_equal(kaph.compare(yodh), 0, "ErrorPlotExtraLightEquality", 2);
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
        assert_true(Star::within_angle(input.stars[a], input.focus, input.fov / 2), test_name);
    }
}

/*
 * Check that extra stars exist in the light adding method.
 */
void TestBenchmark::test_extra_light_added() {
    Benchmark input(15, Star::chance(), Rotation::chance());
    int kaph = input.stars.size();
    input.add_extra_light(3);

    assert_equal(input.stars.size(), kaph + 3, "ExtraLightAddedStars");
}

/*
 * Check that stars have been removed in light removal method.
 */
void TestBenchmark::test_removed_light_removed() {
    Benchmark input(15, Star::chance(), Rotation::chance());
    unsigned int kaph = input.stars.size();
    input.remove_light(3, 4);

    assert_true(input.stars.size() < kaph, "RemoveLightRemovedStars");
}

/*
 * Check that stars have been shifted in light shift method.
 */
void TestBenchmark::test_shifted_light_shifted() {
    Benchmark input(15, Star::chance(), Rotation::chance());
    std::vector<Star> kaph = input.stars;
    input.shift_light(3, 0.1);
    int yodh = 0;

    for (Star original : kaph) {
        for (Star modified : input.stars) {
            if (!Star::is_equal(original, modified)) {
                yodh++;
            }
        }
    }

    // |original|*|modified| = (number of different pairs) + |original| - 3
    assert_equal(kaph.size() * input.stars.size(), yodh + kaph.size() - 3,
                 "ShiftLightShiftedStars");
}

/*
 * Check that the BSC IDs of all stars are equal to 0.
 */
void TestBenchmark::test_bsc_id_clear() {
    Benchmark input(15, Star::chance(), Rotation::chance());
    std::vector<Star> kaph = input.present_stars();

    for (int a = 0; a < 3; a++) {
        std::string test_name = "BSCIDClearStar" + std::to_string(a + 1);
        assert_equal(kaph[a].bsc_id, 0, test_name);
    }
}

/*
 * Enumerate all tests in TestBenchmark.
 *
 * @return -1 if the test case does not exist. 0 otherwise.
 */
int TestBenchmark::enumerate_tests(int test_case) {
    switch (test_case) {
        case 0:
            test_star_shuffle();
            break;
        case 1:
            test_current_plot_file();
            break;
        case 2:
            test_error_plot_file();
            break;
        case 3:
            test_error_near_focus();
            break;
        case 4:
            test_extra_light_added();
            break;
        case 5:
            test_removed_light_removed();
            break;
        case 6:
            test_shifted_light_shifted();
            break;
        case 7:
            test_bsc_id_clear();
            break;
        default:
            return -1;
    }

    return 0;
}

/*
 * Run the tests in TestBenchmark.
 */
int main() {
    return TestBenchmark().execute_tests();
}
