/// @file test-identification.cpp
/// @author Glenn Galvizo
///
/// Source file for all Identification class unit tests and the test runner. Uses the Angle class as a "proxy" class.

#define ENABLE_TESTING_ACCESS

#include <numeric>
#include <fstream>
#include "gmock/gmock.h"
#include "third-party/inih/INIReader.h"

#include "identification/angle.h"

/// Check that the parameter collector method transfers the appropriate parameters.
TEST(ParameterCollect, CleanInput) {
    std::ofstream f1(std::string(std::getenv("HOKU_PROJECT_PATH")) + "/data/tmp/TESTCONFIG1.ini");
    ASSERT_TRUE(f1.is_open());
    
    f1 << "[id-parameters]             ; Values used in 'Parameters' struct.\n"
        "sq = 0.000000001            ; Sigma query value (degrees).\n"
        "sl = 500                    ; Tuple count returned restriction.\n"
        "prsc = 1                    ; 'Pass R Set Cardinality' toggle.\n"
        "fbr = 0                     ; 'Favor Bright Stars' toggle.\n"
        "so = 0.00000001             ; Sigma overlay (degrees).\n"
        "nu-m = 50000                ; Maximum number of query star comparisons (nu max).\n"
        "wbs = TRIAD                 ; Function used to solve Wabha (possible TRIAD, QUEST, Q)";
    f1.close();
    
    INIReader cf1(std::string(std::getenv("HOKU_PROJECT_PATH")) + "/data/tmp/TESTCONFIG1.ini");
    Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
    Identification::collect_parameters(p, cf1);
    
    EXPECT_FLOAT_EQ(p.sigma_query, 0.000000001);
    EXPECT_EQ(p.sql_limit, 500);
    EXPECT_EQ(p.pass_r_set_cardinality, true);
    EXPECT_EQ(p.favor_bright_stars, false);
    EXPECT_FLOAT_EQ(p.sigma_overlay, 0.00000001);
    EXPECT_EQ(p.nu_max, 50000);
    EXPECT_EQ(p.f, Rotation::triad);
    std::remove((std::string(std::getenv("HOKU_PROJECT_PATH")) + "/data/tmp/TESTCONFIG1.ini").c_str());
}

/// Check that the parameter collector method uses default parameters under improper conditions.
TEST(ParameterCollect, ErrorInput) {
    std::ofstream f2(std::string(std::getenv("HOKU_PROJECT_PATH")) + "/data/tmp/TESTCONFIG2.ini");
    ASSERT_TRUE(f2.is_open());
    
    f2 << "[id-parameters]             ; Values used in 'Parameters' struct.\n"
        "sq = asd                    ; Sigma query value (degrees).\n"
        "sl = 500                    ; Tuple count returned restriction.\n"
        "prsc =       a              ; 'Pass R Set Cardinality' toggle.\n"
        "fbr =         2             ; 'Favor Bright Stars' toggle.\n"
        "so = 0.001                  ; Sigma overlay (degrees).\n"
        "nu-m = 5                    ; Maximum number of query star comparisons (nu max).\n"
        "wbs = TY                    ; Function used to solve Wabha (possible TRIAD, QUEST, Q)";
    f2.close();
    
    INIReader cf2(std::string(std::getenv("HOKU_PROJECT_PATH")) + "/data/tmp/TESTCONFIG2.ini");
    Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
    Identification::collect_parameters(p, cf2);
    
    EXPECT_FLOAT_EQ(p.sigma_query, Identification::DEFAULT_SIGMA_QUERY);
    EXPECT_EQ(p.sql_limit, 500);
    EXPECT_EQ(p.pass_r_set_cardinality, Identification::DEFAULT_PASS_R_SET_CARDINALITY);
    EXPECT_EQ(p.favor_bright_stars, Identification::DEFAULT_FAVOR_BRIGHT_STARS);
    EXPECT_FLOAT_EQ(p.sigma_overlay, 0.001);
    EXPECT_EQ(p.nu_max, 5);
    EXPECT_EQ(p.f, Rotation::triad);
    std::remove((std::string(std::getenv("HOKU_PROJECT_PATH")) + "/data/tmp/TESTCONFIG2.ini").c_str());
}

/// Check that the rotating match method marks the all stars as matched.
TEST(FindMatches, CorrectInput) {
    Chomp ch;
    Star a = Star::chance(), b = Star::chance();
    Rotation c = Rotation::chance();
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::triad({a, b}, {d, e});
    Benchmark input(ch, Star::chance(), c, 8);
    std::vector<Star> rev_input;
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.sigma_overlay = 0.000001;
    Angle g(input, p);
    
    // Reverse all input by inverse rotation matrix.
    rev_input.reserve(input.b.size());
    for (Star rotated : input.b) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    std::vector<Star> h = g.find_positive_overlay(rev_input, c);
    EXPECT_EQ(h.size(), input.b.size());
    for (unsigned int q = 0; q < h.size(); q++) {
        EXPECT_EQ(h[q].get_label(), input.b[q].get_label());
    }
}

/// Check that the rotating match method marks only the correct stars as matched.
TEST(FindMatches, ErrorInput) {
    Chomp ch;
    Star a = Star::chance(), b = Star::chance();
    Rotation c = Rotation::chance();
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::triad({a, b}, {d, e});
    Benchmark input(ch, Star::chance(), c, 8);
    std::vector<Star> rev_input;
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.sigma_overlay = 0.000001;
    Angle g(input, p);
    
    // Reverse all input by inverse rotation matrix.
    rev_input.reserve(input.b.size());
    for (Star rotated : input.b) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    // Append center as error.
    rev_input.push_back(input.center);
    std::vector<Star> h = g.find_positive_overlay(rev_input, c);
    EXPECT_EQ(h.size(), input.b.size());
    
    for (unsigned int q = 0; q < h.size(); q++) {
        EXPECT_EQ(h[q].get_label(), input.b[q].get_label());
    }
}

/// Check that the rotating match method marks only the correct stars as matched, not the duplicate as well.
TEST(FindMatches, RotatingDuplicateInput) {
    Chomp ch;
    Star a = Star::chance(), b = Star::chance();
    Rotation c = Rotation::chance();
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::triad({a, b}, {d, e});
    Benchmark input(ch, Star::chance(), c, 8);
    std::vector<Star> rev_input;
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.sigma_overlay = 0.000001;
    Angle g(input, p);
    
    // Reverse all input by inverse rotation matrix.
    rev_input.reserve(input.b.size());
    for (Star rotated : input.b) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    // Append first star as error.
    rev_input.push_back(rev_input[0]);
    rev_input.push_back(rev_input[0]);
    rev_input.push_back(rev_input[0]);
    std::vector<Star> h = g.find_positive_overlay(rev_input, c);
    EXPECT_EQ(h.size(), input.b.size());
    
    for (unsigned int q = 0; q < h.size(); q++) {
        EXPECT_EQ(h[q].get_label(), input.b[q].get_label());
    }
}

/// Check that the sort candidates by brightness method correctly sorts the starts.
TEST(SortBrightness, BrightestStart) {
    Chomp ch;
    Benchmark input(ch, 15);
    Angle g(input, Angle::DEFAULT_PARAMETERS);
    std::vector<Identification::labels_list> ell = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}, {13, 14, 15}};
    g.sort_brightness(ell);
    auto grab_b = [&ch] (const int ell) -> double {
        return ch.query_hip(ell).get_magnitude();
    };
    
    // Ensure that the list is sorted.
    double a = (grab_b(ell[0][0]) + grab_b(ell[0][1]) + grab_b(ell[0][2])) / 3;
    double b = (grab_b(ell[1][0]) + grab_b(ell[1][1]) + grab_b(ell[1][2])) / 3;
    double c = (grab_b(ell[2][0]) + grab_b(ell[2][1]) + grab_b(ell[2][2])) / 3;
    double d = (grab_b(ell[3][0]) + grab_b(ell[3][1]) + grab_b(ell[3][2])) / 3;
    double e = (grab_b(ell[4][0]) + grab_b(ell[4][1]) + grab_b(ell[4][2])) / 3;
    EXPECT_LT(a, b);
    EXPECT_LT(b, c);
    EXPECT_LT(c, d);
    EXPECT_LT(d, e);
}

/// Check that the alignment output used by the Angle method returns a similar rotation.
TEST(AngleAlign, CleanInput) {
    // TODO: Finish a test for comparing the ground-truth quaternion and the resultant from the alignment.
}

/// Check that correct result is returned with a clean input.
TEST(AngleIdentifyAll, CleanInput) {
    Chomp ch;
    Benchmark input(ch, 10, 6.5);
    Angle::Parameters a = Angle::DEFAULT_PARAMETERS;
    a.sigma_overlay = 0.000001;
    unsigned int nu = 0;
    a.nu = std::make_shared<unsigned int>(nu);
    Star::list c = Angle(input, a).identify_all();
    ASSERT_FALSE(c.empty());
    
    for (unsigned int q = 0; q < c.size() - 1; q++) {
        auto is_found = std::find_if(input.b.begin(), input.b.end(), [&c, q] (const Star &b) -> bool {
            return b.get_label() == c[q].get_label();
        });
        EXPECT_NE(is_found, input.b.end());
    }
}

/// Check that correct result is returned with an error input.
TEST(AngleIdentifyAll, ErrorInput) {
    Chomp ch;
    Benchmark input(ch, 9);
    Angle::Parameters a = Angle::DEFAULT_PARAMETERS;
    a.sigma_overlay = 0.000001;
    input.add_extra_light(1);
    unsigned int nu = 0;
    a.nu = std::make_shared<unsigned int>(nu);
    
    std::vector<Star> c = Angle(input, a).identify_all();
    ASSERT_FALSE(c.empty());
    
    for (unsigned int q = 0; q < c.size() - 1; q++) {
        auto is_found = std::find_if(input.b.begin(), input.b.end(), [&c, q] (const Star &b) -> bool {
            return b.get_label() == c[q].get_label();
        });
        EXPECT_NE(is_found, input.b.end());
    }
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