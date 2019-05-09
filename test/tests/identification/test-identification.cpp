/// @file test-identification.cpp
/// @author Glenn Galvizo
///
/// Source file for all Identification class unit tests.

#define ENABLE_TESTING_ACCESS

#define _USE_MATH_DEFINES

#include <cmath>
#include <numeric>
#include <fstream>
#include "gmock/gmock.h"
#include "third-party/inih/INIReader.h"

#include "identification/angle.h"

using testing::Contains;

/// A dummy non-virtual instance of Identification.
class IdentificationDummy : public Identification {
public:
    IdentificationDummy (const Star::list &s, const Identification::Parameters &p) : Identification() {
        this->big_i = std::make_unique<Star::list>(s), parameters = std::make_unique<Identification::Parameters>(p);
    }

    std::vector<Identification::labels_list> query (const Star::list &) override {
        return {};
    }

    Identification::stars_either reduce () override {
        return stars_either{{}, Identification::NO_CONFIDENT_R_EITHER};
    }

    Identification::stars_either identify () override {
        return stars_either{*this->big_i, 0};
    }
};

/// Check that the parameter collector method transfers the appropriate parameters.
TEST(BaseIdentification, ParameterCollectCleanInput) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
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
    Identification::Parameters p = Identification::collect_parameters(cf1, "angle");

    EXPECT_EQ(p.sigma_1, 0.00000001);
    EXPECT_EQ(p.sigma_2, 0);
    EXPECT_EQ(p.sigma_3, 0);
    EXPECT_FLOAT_EQ(p.sigma_4, 0.00000001);
    EXPECT_EQ(p.sql_limit, 500);
    EXPECT_EQ(p.no_reduction, true);
    EXPECT_EQ(p.favor_bright_stars, false);
    EXPECT_EQ(p.nu_max, 50000);
//    EXPECT_EQ(p.f, Rotation::triad);
}

/// Check that the parameter collector method uses default parameters under improper conditions.
TEST(BaseIdentification, ParameterCollectErrorInput) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
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
    Identification::Parameters p = Identification::collect_parameters(cf2, "asd");

    EXPECT_EQ(p.sigma_1, 0);
    EXPECT_EQ(p.sigma_2, 0);
    EXPECT_EQ(p.sigma_3, 0);
    EXPECT_FLOAT_EQ(p.sigma_4, 0.001);
    EXPECT_EQ(p.sql_limit, 500);
    EXPECT_EQ(p.no_reduction, false);
    EXPECT_EQ(p.favor_bright_stars, false);
    EXPECT_EQ(p.nu_max, 5);
//    EXPECT_EQ(p.f, Rotation::triad);
}

/// Check that the rotating match method marks the all stars as matched.
TEST(BaseIdentification, FindMatchesCorrectInput) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;

    // Find some rotation, and it's inverse.
    Rotation c = Rotation::chance();
    Rotation c_inv = Rotation::wrap(Quaternion::Inverse(c));
    Benchmark input(ch, Star::chance(), c, 10);

    // We apply no changes here other than stripping the identifier.
    Identification::Parameters p;
    p.sigma_4 = 0.000001;
    IdentificationDummy g(input.clean_stars(), p);

    // We pass it the *inverse*, the result of b_inv * c = input.b, input.b * c_inv = b_inv.
    Star::list b_inv;
    for (const Star &s: *input.b) {
        b_inv.emplace_back(Rotation::rotate(s, c_inv));
    }
    Star::list h = g.find_positive_overlay(b_inv, c);

    // Apply our checks.
    EXPECT_EQ(h.size(), input.b->size());
    for (const Star &s: h) {
        bool is_near = false;

        // Check for stars near s.
        for (const Star &s_1 : *input.b) {
            double theta = (180.0 / M_PI) * Vector3::Angle(s_1.get_vector(), s.get_vector());
            if (theta < 0.1 && s_1.get_label() == s.get_label()) {
                is_near = true;
                break;
            }
        }
        EXPECT_TRUE(is_near);
    }
}

/// Check that the rotating match method marks only the correct stars as matched.
TEST(BaseIdentification, FindMatchesErrorInput) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;

    // Find some rotation, and it's inverse.
    Rotation c = Rotation::chance();
    Rotation c_inv = Rotation::wrap(Quaternion::Inverse(c));
    Benchmark input(ch, Star::chance(), c, 10);
    Star::list err_input = *input.b;

    // Append center as error.
    Identification::Parameters p;
    p.sigma_4 = 0.000001;
    err_input.push_back(Star::wrap(input.center));
    IdentificationDummy g(err_input, p);

    // We pass it the *inverse*, the result of b_inv * c = input.b, input.b * c_inv = b_inv.
    Star::list b_inv;
    for (const Star &s: *input.b) {
        b_inv.emplace_back(Rotation::rotate(s, c_inv));
    }
    Star::list h = g.find_positive_overlay(b_inv, c);

    // Apply our checks.
    EXPECT_EQ(h.size(), input.b->size());
    for (const Star &s: h) {
        bool is_near = false;

        // Check for stars near s.
        for (const Star &s_1 : *input.b) {
            double theta = (180.0 / M_PI) * Vector3::Angle(s_1.get_vector(), s.get_vector());
            if (theta < 0.1 && s_1.get_label() == s.get_label()) {
                is_near = true;
                break;
            }
        }
        EXPECT_TRUE(is_near);
    }
}

/// Check that the sort candidates by brightness method correctly sorts the starts.
TEST(BaseIdentification, SortBrightnessBrightestStart) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 15);
    IdentificationDummy g(*input.b, Identification::Parameters());
    std::vector<Identification::labels_list> ell = {{1,  2,  3},
                                                    {4,  5,  6},
                                                    {7,  8,  9},
                                                    {10, 11, 12},
                                                    {13, 14, 15}};
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
TEST(BaseIdentification, AlignmentCleanInputTRIAD) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Star s = Star::chance();
    Rotation q = Rotation::chance();
    Benchmark input(ch, s, q, 20);
    IdentificationDummy g(*input.b, Identification::Parameters());
    Rotation q_1 = g.align();

    Star a = (*input.b)[0], b = (*input.b)[1];
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(Rotation::rotate(a, q).get_vector(),
                                              Rotation::rotate(a, q_1).get_vector()), 0.000000000001);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(Rotation::rotate(b, q).get_vector(),
                                              Rotation::rotate(b, q_1).get_vector()), 0.000000000001);
}

/// Check that the complete identification output returns the correct result with a clean input.
TEST(BaseIdentification, CompleteIdentificationCleanInput) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Star s_z = Star::chance();
    Rotation q = Rotation::chance();
    Benchmark input(ch, s_z, q, 20);
    Identification::Parameters p;
    p.sigma_4 = 0.000001;
    IdentificationDummy g(*input.b, p);
    Star::list h = g.identify_all();

    // Apply our checks.
    EXPECT_EQ(h.size(), input.b->size());
    for (const Star &s: h) {
        bool is_near = false;

        // Check for stars near s.
        for (const Star &s_1 : *input.b) {
            double theta = (180.0 / M_PI) * Vector3::Angle(s_1.get_vector(), s.get_vector());
            if (theta < 0.1 && s_1.get_label() == s.get_label()) {
                is_near = true;
                break;
            }
        }
        EXPECT_TRUE(is_near);
    }
}