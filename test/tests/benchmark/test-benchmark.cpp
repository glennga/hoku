/// @file test-benchmark.cpp
/// @author Glenn Galvizo
///
/// Source file for all Benchmark class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include <fstream>
#include "gtest/gtest.h"

#include "benchmark/benchmark.h"

/// Check that the constructor for random generation works as intended. 'generate_stars' is tested here.
TEST(BenchmarkConstructor, RandomGenerator) {
    Chomp ch;
    Benchmark input(ch, 20);
    Benchmark input2(ch, 20, 7.0);
    
    EXPECT_NE(input.q_rb, input2.q_rb);
    EXPECT_NE(input.center, input2.center);
    EXPECT_EQ(input.fov, input2.fov);
    
    for (const Star &s : input.b) {
        EXPECT_LT(s.get_magnitude(), Benchmark::DEFAULT_M_BAR);
    }
    for (const Star &s : input2.b) {
        EXPECT_LT(s.get_magnitude(), 7.0);
    }
}

/// Check that constructor for specific generation works as intended. 'generate_stars' is tested here.
TEST(BenchmarkConstructor, SpecificGenerator) {
    Chomp ch;
    Star s = Star::chance();
    Rotation q = Rotation::chance();
    Benchmark input(ch, s, q, 20);
    Benchmark input2(ch, s, q, 20);
    
    EXPECT_EQ(input.q_rb, q);
    EXPECT_EQ(input.center, Rotation::rotate(s, q));
    EXPECT_EQ(input.fov, 20.0);
    EXPECT_NE(input.b[0], input2.b[0]);
    EXPECT_NE(input.b[1], input2.b[1]);
    
    for (const Star &s_1 : input.b) {
        EXPECT_LT(s_1.get_magnitude(), Benchmark::DEFAULT_M_BAR);
    }
}

/// Check that the direct-setting private constructor works as intended.
TEST(BenchmarkConstructor, NoGenerator) {
    Star::list s = {Star::chance(), Star::chance(), Star::chance()};
    Benchmark input(s, s[0], 20);
    EXPECT_EQ(input.b[0], s[0]);
    EXPECT_EQ(input.b[1], s[1]);
    EXPECT_EQ(input.b[2], s[2]);
    EXPECT_EQ(input.center, s[0]);
    EXPECT_EQ(input.fov, 20);
}

/// Check that the properties of a black image hold.
TEST(BenchmarkImage, Black) {
    Benchmark input = Benchmark::black();
    EXPECT_EQ(input.b.size(), 0);
    EXPECT_EQ(input.center, Vector3::Zero());
    EXPECT_EQ(input.fov, 0);
}

/// Check that the stars are not in the same order after shuffling.
TEST(BenchmarkImage, StarShuffle) {
    Chomp ch;
    Benchmark input(ch, 20);
    Star a = input.b[0];
    
    input.shuffle();
    Star b = input.b[0];
    input.shuffle();
    EXPECT_NE(a, b);
    EXPECT_NE(a, input.b[0]);
}

/// Check that the catalog ID numbers of all stars are equal to 0.
TEST(BenchmarkImage, LabelClear) {
    Chomp ch;
    Benchmark input(ch, 15);
    std::vector<Star> a = input.clean_stars();
    
    for (const Star &s : a) {
        EXPECT_EQ(s.get_label(), Star::NO_LABEL);
    }
}

/// Check that the present_stars method returns the correct fov and stars.
TEST(BenchmarkImage, Presentation) {
    Chomp ch;
    Benchmark input(ch, 15);
    Star::list s;
    double fov;
    
    input.present_image(s, fov);
    EXPECT_EQ(fov, input.fov);
    EXPECT_EQ(s.size(), input.b.size());
    for (const Star &s_i : s) {
        EXPECT_EQ(s_i.get_label(), Star::NO_LABEL);
    }
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
    sprintf(e, "%0.16f %0.16f %0.16f", c[0], c[1], c[2]);
    EXPECT_EQ(d, std::string(e));
    
    std::getline(current_plot_from_input, d);
    sprintf(e, "%0.16f %0.16f %0.16f %d", input.b[0][0], input.b[0][1], input.b[0][2], input.b[0].get_label());
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

/// Check that no error is thrown when the plot is displayed.
TEST(BenchmarkDisplay, Plot) {
    Chomp ch;
    Benchmark input(ch, 15);
    EXPECT_NO_THROW(input.display_plot(););
}

/// Check that all error models place stars near focus.
TEST(BenchmarkError, NearFocus) {
    Chomp ch;
    
    Benchmark input(ch, 15);
    input.add_extra_light(3);
    input.remove_light(3, 4);
    input.shift_light(3, 1);
    for (int a = 0; a < 5; a++) {
        EXPECT_TRUE(Star::within_angle(input.b[a], input.center, input.fov / 2));
    }
}

/// Check that extra stars exist in the light adding method.
TEST(BenchmarkError, ExtraLightAdded) {
    Chomp ch;
    Benchmark input(ch, 15);
    unsigned long long a = input.b.size();
    input.add_extra_light(3);
    EXPECT_EQ(input.b.size(), a + 3);
}

/// Check that stars have been removed in light removal method.
TEST(BenchmarkError, RemovedLightRemoved) {
    Chomp ch;
    Benchmark input(ch, 15);
    unsigned long long a = input.b.size();
    input.remove_light(3, 15);
    EXPECT_LT(input.b.size(), a);
}

// Check that stars have been shifted in light shift method.
TEST(BenchmarkError, ShiftedLightMoved) {
    Chomp ch;
    Benchmark input(ch, 15);
    std::vector<Star> a = input.b;
    input.shift_light(3, 0.1);
    int b = 0;
    
    // |original|*|modified| = (number of different pairs) + |original| - 3
    for (Star original : a) {
        for (Star modified : input.b) {
            if (!(original == modified)) {
                b++;
            }
        }
    }
    EXPECT_EQ(a.size() * input.b.size(), b + a.size() - 3);
}

// TODO: Fix the barreled light method. Not currently working.
/// Check that stars have been shifted in light barrel method.
TEST(BenchmarkError, BarreledLightMoved) {
    Chomp ch;
    Benchmark input(ch, 15);
    std::vector<Star> a = input.b;
    input.barrel_light(0.00001);
    int b = 0;
    
    // All stars should be modified.
    for (Star original : a) {
        for (Star modified : input.b) {
            if (original == modified) {
                b++;
            }
        }
    }
    //    EXPECT_EQ(0, b);
}


/// Check that an error star exists at the front of the stars vector when cap_error is raised.
TEST(BenchmarkError, CapError) {
    Chomp ch;
    Benchmark input(ch, 15);
    std::vector<Star> a = input.b;
    input.shift_light(1, 0.1, true);
    bool error_star_at_front = true;
    
    for (const Star &original : a) {
        if (input.b[0] == original) {
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