/// @file test-identification.cpp
/// @author Glenn Galvizo
///
/// Source file for all Identification class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#define _USE_MATH_DEFINES
#include <cmath>
#include <numeric>
#include <fstream>
#include "gmock/gmock.h"
#include "third-party/inih/INIReader.h"

#include "identification/angle.h"

/// A dummy non-virtual instance of Identification.
class IdentificationDummy : public Identification {
  public:
    IdentificationDummy (const Star::list &s, const Identification::Parameters &p) : Identification() {
        this->big_i = s, parameters = p;
    }
    std::vector<labels_list> query (const Star::list &) {
        return {Identification::EMPTY_BIG_R_ELL};
    }
    labels_list reduce () {
        return Identification::EMPTY_BIG_R_ELL;
    }
    Star::list identify () {
        return this->big_i;
    }
};

/// Check that the parameter collector method transfers the appropriate parameters.
TEST(ParameterCollect, CleanInput) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    std::string temp_path = std::getenv("TEMP");
#else
    std::string temp_path = "/tmp";
#endif
    std::ofstream f1(temp_path + "/TESTCONFIG1.ini");
    ASSERT_TRUE(f1.is_open());
    
    f1 << "[query-sigma]               ; Estimated deviation for each identification method.\n"
        "angle-1 = 0.00000001    ; Standard deviation of theta^ij.\n"
        "dot-1 = 0.00000001     ; Standard deviation of theta^ic.\n"
        "dot-2 = 0.00000001     ; Standard deviation of theta^jc.\n"
        "dot-3 = 0.00000001        ; Standard deviation of phi^ijc.\n"
        "sphere-1 = 0.00000001       ; Standard deviation of spherical area (i, j, k).\n"
        "sphere-2 = 0.00000001       ; Standard deviation of spherical moment (i, j, k).\n"
        "plane-1 = 0.00000001        ; Standard deviation of planar area (i, j, k).\n"
        "plane-2 = 0.00000001        ; Standard deviation of planar moment (i, j, k).\n"
        "pyramid-1 = 0.00000001  ; Standard deviation of theta^ij.\n"
        "composite-1 = 0.00000001    ; Standard deviation of planar area (i, j, k).\n"
        "composite-2 = 0.00000001    ; Standard deviation of planar moment (i, j, k).\n"
        "[id-parameters]             ; Values used in 'Parameters' struct.\n"
        "sl = 500                    ; Tuple count returned restriction.\n"
        "nr = 1                      ; 'Pass R Set Cardinality' toggle.\n"
        "fbr = 0                     ; 'Favor Bright Stars' toggle.\n"
        "so = 0.00000001             ; Sigma overlay (degrees).\n"
        "nu-m = 50000                ; Maximum number of query star comparisons (nu max).\n"
        "wbs = TRIAD                 ; Function used to solve Wabha (possible TRIAD, SVD, Q)";
    f1.close();
    
    INIReader cf1(temp_path + "/TESTCONFIG1.ini");
    Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
    Identification::collect_parameters(p, cf1, "angle");
    
    EXPECT_EQ(p.sigma_1, 0.00000001);
    EXPECT_EQ(p.sigma_2, 0);
    EXPECT_EQ(p.sigma_3, 0);
    EXPECT_FLOAT_EQ(p.sigma_4, 0.00000001);
    EXPECT_EQ(p.sql_limit, 500);
    EXPECT_EQ(p.no_reduction, true);
    EXPECT_EQ(p.favor_bright_stars, false);
    EXPECT_EQ(p.nu_max, 50000);
    EXPECT_EQ(p.f, Rotation::triad);
}

/// Check that the parameter collector method uses default parameters under improper conditions.
TEST(ParameterCollect, ErrorInput) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    std::string temp_path = std::getenv("TEMP");
#else
    std::string temp_path = "/tmp";
#endif
    std::ofstream f2(temp_path + "/TESTCONFIG2.ini");
    ASSERT_TRUE(f2.is_open());
    
    f2 << "[query-sigma]               ; Estimated deviation for each identification method.\n"
        "angle-1 = 0.00000001    ; Standard deviation of theta^ij.\n"
        "dot-1 = 0.00000001     ; Standard deviation of theta^ic.\n"
        "dot-2 = 0.00000001     ; Standard deviation of theta^jc.\n"
        "dot-3 = 0.00000001        ; Standard deviation of phi^ijc.\n"
        "sphere-1 = 0.00000001       ; Standard deviation of spherical area (i, j, k).\n"
        "sphere-2 = 0.00000001       ; Standard deviation of spherical moment (i, j, k).\n"
        "plane-1 = 0.00000001        ; Standard deviation of planar area (i, j, k).\n"
        "plane-2 = 0.00000001        ; Standard deviation of planar moment (i, j, k).\n"
        "pyramid-1 = 0.00000001  ; Standard deviation of theta^ij.\n"
        "composite-1 = 0.00000001    ; Standard deviation of planar area (i, j, k).\n"
        "composite-2 = 0.00000001    ; Standard deviation of planar moment (i, j, k).\n"
        "[id-parameters]             ; Values used in 'Parameters' struct.\n"
        "sl = 500                    ; Tuple count returned restriction.\n"
        "nr =         a              ; 'Pass R Set Cardinality' toggle.\n"
        "fbr =         2             ; 'Favor Bright Stars' toggle.\n"
        "so = 0.001                  ; Sigma overlay (degrees).\n"
        "nu-m = 5                    ; Maximum number of query star comparisons (nu max).\n"
        "wbs = TY                    ; Function used to solve Wabha (possible TRIAD, SVD, Q)";
    f2.close();
    
    INIReader cf2(temp_path + "/TESTCONFIG2.ini");
    Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
    Identification::collect_parameters(p, cf2, "asd");
    
    EXPECT_EQ(p.sigma_1, 0);
    EXPECT_EQ(p.sigma_2, 0);
    EXPECT_EQ(p.sigma_3, 0);
    EXPECT_FLOAT_EQ(p.sigma_4, 0.001);
    EXPECT_EQ(p.sql_limit, 500);
    EXPECT_EQ(p.no_reduction, Identification::DEFAULT_NO_REDUCTION);
    EXPECT_EQ(p.favor_bright_stars, Identification::DEFAULT_FAVOR_BRIGHT_STARS);
    EXPECT_EQ(p.nu_max, 5);
    EXPECT_EQ(p.f, Rotation::triad);
}

/// Check that the rotating match method marks the all stars as matched.
TEST(FindMatches, CorrectInput) {
    Chomp ch;
    Star a = Star::chance(), b = Star::chance();
    Rotation c = Rotation::chance();
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::triad({a, b}, {d, e});
    Benchmark input(ch, Star::chance(), c, 8);
    Star::list rev_input;
    Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
    p.sigma_4 = 0.000001;
    IdentificationDummy g(input.clean_stars(), p);
    
    // Reverse all input by inverse rotation matrix.
    rev_input.reserve(input.b.size());
    for (Star rotated : input.b) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    Star::list h = g.find_positive_overlay(rev_input, c);
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
    Star::list rev_input;
    Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
    p.sigma_4 = 0.000001;
    IdentificationDummy g(input.clean_stars(), p);
    
    // Reverse all input by inverse rotation matrix.
    rev_input.reserve(input.b.size());
    for (Star rotated : input.b) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    // Append center as error.
    rev_input.push_back(Star::wrap(input.center));
    Star::list h = g.find_positive_overlay(rev_input, c);
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
    Star::list rev_input;
    Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
    p.sigma_4 = 0.000001;
    IdentificationDummy g(input.clean_stars(), p);
    
    // Reverse all input by inverse rotation matrix.
    rev_input.reserve(input.b.size());
    for (Star rotated : input.b) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    // Append first star as error.
    rev_input.push_back(rev_input[0]);
    rev_input.push_back(rev_input[0]);
    rev_input.push_back(rev_input[0]);
    Star::list h = g.find_positive_overlay(rev_input, c);
    EXPECT_EQ(h.size(), input.b.size());
    
    for (unsigned int q = 0; q < h.size(); q++) {
        EXPECT_EQ(h[q].get_label(), input.b[q].get_label());
    }
}

/// Check that the sort candidates by brightness method correctly sorts the starts.
TEST(SortBrightness, BrightestStart) {
    Chomp ch;
    Benchmark input(ch, 15);
    IdentificationDummy g(input.b, Identification::DEFAULT_PARAMETERS);
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

/// Check that the alignment output using the TRIAD method returns an alignment similar to the one used to generate the
/// image.
TEST(Alignment, CleanInputTRIAD) {
    Chomp ch;
    Star s = Star::chance();
    Rotation q = Rotation::chance();
    Benchmark input(ch, s, q, 20);
    IdentificationDummy g(input.b, Identification::DEFAULT_PARAMETERS);
    Rotation q_1 = g.align();
    
    Star a = input.b[3], b = input.b[4];
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(Rotation::rotate(a, q), Rotation::rotate(a, q_1)), 00000000000001);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(Rotation::rotate(b, q), Rotation::rotate(b, q_1)), 0.000000000001);
}

/// Check that the complete identification output returns the correct result with a clean input.
TEST(CompleteIdentification, CleanInput) {
    Chomp ch;
    Star s = Star::chance();
    Rotation q = Rotation::chance();
    Benchmark input(ch, s, q, 20);
    Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
    p.sigma_4 = 0.000001;
    IdentificationDummy g(input.b, p);
    
    Star::list s_l = g.identify_all();
    for (unsigned int i = 0; i < s_l.size(); i++) {
        EXPECT_EQ(s_l[i], input.b[i]);
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