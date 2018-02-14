/// @file test-benchmark.cpp
/// @author Glenn Galvizo
///
/// Source file for all Benchmark class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include <fstream>
#include "gtest/gtest.h"

#include "benchmark/benchmark.h"

/// Check that the stars are not in the same order after shuffling.
TEST(BenchmarkImage, StarShuffle) {
    Chomp ch;
    Benchmark input(ch, 15);
    Star a = input.stars[0], b(0, 0, 0);
    
    input.shuffle();
    b = input.stars[0];
    input.shuffle();
    EXPECT_FALSE(a == b);
    EXPECT_FALSE(b == input.stars[0]);
}

/// Check that the file current_plot log file is formatted correctly.
TEST(BenchmarkLog, CurrentPlotFile) {
    Chomp ch;
    Star a = Star::chance();
    Rotation b = Rotation::chance();
    Benchmark input(ch, a, b, 15);
    Star c = Rotation::rotate(a, b);
    std::string d;
    char e[200];
    
    std::remove(input.CURRENT_TMP.c_str());
    std::remove(input.ERROR_TMP.c_str());
    input.record_current_plot();
    std::ifstream current_plot_from_input(input.CURRENT_TMP);
    ASSERT_TRUE(current_plot_from_input.good());
    
    std::getline(current_plot_from_input, d);
    EXPECT_EQ(15, std::stoi(d));
    std::getline(current_plot_from_input, d);
    EXPECT_DOUBLE_EQ(1, std::stod(d));
    
    std::getline(current_plot_from_input, d);
    sprintf(e, "%0.16f %0.16f %0.16f ", c[0], c[1], c[2]);
    EXPECT_EQ(d, std::string(e));
    
    std::getline(current_plot_from_input, d);
    sprintf(e, "%0.16f %0.16f %0.16f %d", input.stars[0][0], input.stars[0][1], input.stars[0][2],
            input.stars[0].get_label());
    EXPECT_EQ(d, std::string(e));
}

/// Check that the file error_plot log file is formatted correctly.
TEST(BenchmarkLog, ErrorPlotFile) {
    Chomp ch;
    Benchmark input(ch, 15);
    std::string a;
    char b[200];
    
    std::remove(input.CURRENT_TMP.c_str());
    std::remove(input.ERROR_TMP.c_str());
    input.add_extra_light(1);
    input.record_current_plot();
    
    std::ifstream error_plot_from_input(input.ERROR_TMP);
    ASSERT_TRUE(error_plot_from_input.good());
    
    // NOTE: Here, b truncates a digit, but this is correct otherwise.
    std::getline(error_plot_from_input, a);
    sprintf(b, "%0.16f %0.16f %0.16f %d %s", input.error_models[0].affected[0][0], input.error_models[0].affected[0][1],
            input.error_models[0].affected[0][2], input.error_models[0].affected[0].get_label(),
            input.error_models[0].plot_color.c_str());
    EXPECT_EQ(a, std::string(b));
}

/// Check that all error models place stars near focus.
TEST(BenchmarkError, NearFocus) {
    Chomp ch;
    
    Benchmark input(ch, 15);
    input.add_extra_light(3);
    input.remove_light(3, 4);
    input.shift_light(3, 1);
    for (int a = 0; a < 5; a++) {
        EXPECT_TRUE(Star::within_angle(input.stars[a], input.focus, input.fov / 2));
    }
}

/// Check that extra stars exist in the light adding method.
TEST(BenchmarkError, ExtraLightAdded) {
    Chomp ch;
    Benchmark input(ch, 15);
    unsigned long long a = input.stars.size();
    input.add_extra_light(3);
    EXPECT_EQ(input.stars.size(), a + 3);
}

/// Check that stars have been removed in light removal method.
TEST(BenchmarkError, RemovedLightRemoved) {
    Chomp ch;
    Benchmark input(ch, 15);
    unsigned long long a = input.stars.size();
    input.remove_light(3, 15);
    EXPECT_LT(input.stars.size(), a);
}

// Check that stars have been shifted in light shift method.
TEST(BenchmarkError, ShiftedLightMoved) {
    Chomp ch;
    Benchmark input(ch, 15);
    std::vector<Star> a = input.stars;
    input.shift_light(3, 0.1);
    int b = 0;
    
    // |original|*|modified| = (number of different pairs) + |original| - 3
    for (Star original : a) {
        for (Star modified : input.stars) {
            if (!(original == modified)) {
                b++;
            }
        }
    }
    EXPECT_EQ(a.size() * input.stars.size(), b + a.size() - 3);
}

// Check that stars have been shifted in light barrel method.
TEST(BenchmarkError, BarreledLightMoved) {
    Chomp ch;
    Benchmark input(ch, 15);
    std::vector<Star> a = input.stars;
    input.barrel_light(0.00001);
    int b = 0;
    
    // All stars should be modified.
    for (Star original : a) {
        for (Star modified : input.stars) {
            if (original == modified) {
                b++;
            }
        }
    }
    EXPECT_EQ(0, b);
}

/// Check that the catalog ID numbers of all stars are equal to 0.
TEST(BenchmarkImage, LabelClear) {
    Chomp ch;
    Benchmark input(ch, 15);
    std::vector<Star> a = input.clean_stars();
    
    for (int q = 0; q < 3; q++) {
        EXPECT_EQ(a[q].get_label(), Star::NO_LABEL);
    }
}

/// We are not checking anything here- this is where the user visually checks and ensures that the plot is displayed
/// properly.
TEST(BenchmarkImage, DisplayExtraShifted) {
    Chomp ch;
    Benchmark input(ch, 15);
    input.add_extra_light(2);
    input.shift_light(2, 10);
    
    input.display_plot();
}

/// We are not checking anything here- this is where the user visually checks and ensures that the barrel distortion
/// is working as intended.
TEST(BenchmarkImage, DisplayBarreled) {
    Chomp ch;
    Benchmark input(ch, 15);
    
    input.display_plot();
    input.barrel_light(0.000000000000001);
    input.display_plot();
}

/// Check that the correct number of stars are returned from the "compare" function.
TEST(BenchmarkImage, Compare) {
    Chomp ch;
    Benchmark a(ch, 15);
    Star::list b = a.stars;
    
    // Erase two stars from set B.
    b.erase(b.begin() + 0);
    b.erase(b.begin() + 1);
    b.emplace_back(Star(5, 5, 5));
    EXPECT_NE(a.stars.size(), Benchmark::compare_stars(a, b));
    EXPECT_EQ(a.stars.size(), Benchmark::compare_stars(a, b) + 2);
}

/// Check that an error star exists at the front of the stars vector when cap_error is raised.
TEST(BenchmarkError, CapError) {
    Chomp ch;
    Benchmark input(ch, 15);
    std::vector<Star> a = input.stars;
    input.shift_light(1, 0.1, true);
    bool error_star_at_front = true;
    
    for (const Star &original : a) {
        if (input.stars[0] == original) {
            error_star_at_front = false;
            break;
        }
    }
    EXPECT_TRUE(error_star_at_front);
}

/// Runs all tests defined in this file.
///
/// @param argc Argument count. Used in Google Test initialization.
/// @param argv Argument vector. Used in Google Test initialization.
/// @return The result of running all tests.
int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}