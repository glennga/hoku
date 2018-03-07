/// @file test-experiment.cpp
/// @author Glenn Galvizo
///
/// Source file for all Experiment class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include <chrono>
#include <fstream>
#include "gmock/gmock.h"

#include "identification/angle.h"
#include "identification/spherical-triangle.h"
#include "experiment/experiment.h"

// Create an in-between matcher for Google Mock.
using testing::PrintToString;
MATCHER_P2(IsBetween, a, b,
           std::string(negation ? "isn't" : "is") + " between " + PrintToString(a) + " and " + PrintToString(b)) {
    return a <= arg && arg <= b;
}

/// Contents for all configuration files for all trials.
const std::string ALL_INI = "[hardware]                  ; Description of hardware and time.\n"
    "fov = 20                    ; Field-of-view of camera.\n"
    "[general-experiment]        ; Testing parameters for all experiments.\n"
    "samples = 1                 ; Number of samples to retrieve for each trial.\n"
    "[query-sigma]               ; Estimated deviation for each identification method.\n"
    "angle-1 = 0.00000001        ; Standard deviation of theta^ij.\n"
    "dot-1 = 0.00000001          ; Standard deviation of theta^ic.\n"
    "dot-2 = 0.00000001          ; Standard deviation of theta^jc.\n"
    "dot-3 = 0.00000001          ; Standard deviation of phi^ijc.\n"
    "sphere-1 = 0.00000001       ; Standard deviation of spherical area (i, j, k).\n"
    "sphere-2 = 0.00000001       ; Standard deviation of spherical moment (i, j, k).\n"
    "plane-1 = 0.00000001        ; Standard deviation of planar area (i, j, k).\n"
    "plane-2 = 0.00000001        ; Standard deviation of planar moment (i, j, k).\n"
    "pyramid-1 = 0.00000001      ; Standard deviation of theta^ij.\n"
    "composite-1 = 0.00000001    ; Standard deviation of planar area (i, j, k).\n"
    "composite-2 = 0.00000001    ; Standard deviation of planar moment (i, j, k).\n"
    "[id-parameters]             ; Values used in 'Parameters' struct.\n"
    "so = 0.00001                ; Sigma overlay.\n"
    "sl = 500                    ; Tuple count returned restriction.\n"
    "nr = 1                      ; 'Pass R Set Cardinality' toggle.\n"
    "fbr = 0                     ; 'Favor Bright Stars' toggle.\n"
    "so = 0.00001                ; Sigma overlay (degrees).\n"
    "nu-m = 50000                ; Maximum number of query star comparisons (nu max).\n"
    "wbs = TRIAD                 ; Function used to solve Wabha (possible TRIAD, SVD, Q)\n"
    "[table-names]               ; Table names in Nibble database.\n"
    "hip = HIP                   ; All star entries in the Hipparcos catalog.\n"
    "bright = HIP_BRIGHT         ; All star entries in Hipparcos with m < 6.\n"
    "angle = ANGLE_20            ; Name of table used by Angle method.\n"
    "dot = DOT_20                ; Name of table used by Dot Angle method.\n"
    "sphere = SPHERE_20          ; Name of table used by Spherical Triangle method.\n"
    "plane = PLANE_20            ; Name of table used by Planar Triangle method.\n"
    "pyramid = PYRAMID_20        ; Name of table used by Pyramid method.\n"
    "composite = COMPOSITE_20    ; Name of table used by Composite Pyramid method.\n"
    "[table-focus]               ; *DO NOT MODIFY!* Field used for B-Tree index / K-Vector.\n"
    "angle = theta               ; Focus of Angle method.\n"
    "dot = theta_1               ; Focus of Dot Angle method.\n"
    "sphere = a                  ; Focus of Spherical Triangle method.\n"
    "plane = a                   ; Focus of Planar Triangle method.\n"
    "pyramid = theta             ; Focus of Pyramid method.\n"
    "composite = a               ; Focus of Composite Pyramid method.\n";

/// Contents of the configuration file for query trials.
const std::string QUERY_INI = "[query-experiment]          ; Testing parameters for the query experiment.\n"
    "lu = QUERY                  ; Name of the Lumberjack table to log results to.\n"
    "ss-step = 0.000000001       ; Shift sigma multiplier for each variation.\n"
    "ss-iter = 5                 ; Number of shift sigma variations.\n";

/// Contents of the configuration file for reducition trials.
const std::string REDUCTION_INI = "[reduction-experiment]      ; Testing parameters for the reduction experiment.\n"
    "lu = REDUCTION              ; Name of the Lumberjack table to log results to.\n"
    "ss-step = 0.000000001       ; Shift sigma multiplier for each variation.\n"
    "ss-iter = 5                 ; Number of shift sigma variations.\n"
    "mb-min = 6                  ; Starting apparent magnitude for each variation.\n"
    "mb-step = 0.25              ; Step to increment magnitude bound with.\n"
    "mb-iter = 5                 ; Number of magnitude bound variations.\n";

/// Contents of the configuration file for identification trials.
const std::string IDENTIFICATION_INI = "[identification-experiment] ; Some comment...\n"
    "lu = IDENTIFICATION         ; Name of the Lumberjack table to log results to.\n"
    "ss-step = 0.000000001       ; Shift sigma multiplier for each variation.\n"
    "ss-iter = 5                 ; Number of shift sigma variations.\n"
    "mb-min = 6                  ; Starting apparent magnitude for each variation.\n"
    "mb-step = 0.25              ; Step to increment magnitude bound with.\n"
    "mb-iter = 5                 ; Number of magnitude bound variations.\n"
    "es-min = 0                  ; Starting number of false stars to add to image.\n"
    "es-step = 3                 ; Step to increment false star count with.\n"
    "es-iter = 5                 ; Number of false star count variations.\n";

/// Create the three objects required for an experiment: Chomp connection, Lumberjack connection, and the configuration
/// file. Return these through the parameters.
///
/// @param cf Reference to the pointer to configuration reader.
/// @param ch Reference to the pointer to chomp connection.
/// @param lu Reference to the ointer to lumberjack connection.
/// @param method Name of the identification method (used in configuration file).
/// @param trial Name of the trial (used in configuration file).
/// @param m_c Name of identification method to record in lumberjack.
void setup_experiment (std::shared_ptr<INIReader> &cf, std::shared_ptr<Chomp> &ch, std::shared_ptr<Lumberjack> &lu,
                       const std::string &method, const std::string &trial, const std::string &m_c) {
    std::ostringstream l;
    using clock = std::chrono::high_resolution_clock;
    l << clock::to_time_t(clock::now() - std::chrono::hours(24));

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    std::string temp_path = std::getenv("TEMP");
#else
    std::string temp_path = "/tmp";
#endif
    std::ofstream f1(temp_path + "/TESTCONFIG.ini");
    if (trial == "query") {
        f1 << ALL_INI << QUERY_INI;
    }
    else if (trial == "reduction") {
        f1 << ALL_INI << REDUCTION_INI;
    }
    else {
        f1 << ALL_INI << IDENTIFICATION_INI;
    }
    f1.close();
    
    cf = std::make_shared<INIReader>(temp_path + "/TESTCONFIG.ini");
    ch = std::make_shared<Chomp>();
    //    ch = std::make_shared<Chomp>((*cf).Get("table-names", method, ""), (*cf).Get("table-focus", method, ""));
    lu = std::make_shared<Lumberjack>((*cf).Get(trial + "-experiment", "lu", ""), m_c, l.str());
}

/// Ensure that the benchmark presentation is random, and that the specifications are met.
TEST(ExperimentAll, PresentBenchmark) {
    Chomp ch;
    Star::list big_i, old_big_i = {Star::chance()};
    Star center, old_center = Star::chance();
    std::array<double, 2> fov_p = {20, 10}, m_p = {6.0, 7.0};
    
    for (int p = 0; p < 2; p++) {
        for (int i = 0; i < 100; i++) {
            Experiment::present_benchmark(ch, big_i, center, fov_p[p], m_p[p]);
            
            // Ensure that at-least 5 stars exist for each generated benchmark.
            EXPECT_GT(big_i.size(), 5);
            
            // Expect uniqueness.
            EXPECT_NE(old_big_i[0], big_i[0]);
            EXPECT_NE(center, old_center);
            
            // Expect all stars are near each other.
            EXPECT_TRUE(Star::within_angle(big_i, fov_p[p]));
            EXPECT_TRUE(Star::within_angle(big_i[0], center, fov_p[p] / 2.0));
            
            // Expect that all stars lower than the specified magnitude.
            for (int j = 0; j < 5; j++) {
                EXPECT_LT(big_i[j].get_magnitude(), m_p[p]);
            }
            
            old_big_i = big_i, old_center = center;
        }
    }
}

/// Check that the correct number of stars are generated, and that this set is random.
TEST(ExperimentQuery, GenerateNStars) {
    Chomp ch;
    Star center, old_center = Star::chance();
    std::array<double, 2> fov_p = {20, 18};
    
    for (int p = 0; p < 2; p++) {
        for (unsigned int n = 0; n < 10; n++) {
            for (int i = 0; i < 100; i++) {
                Star::list a = Experiment::Query::generate_n_stars(ch, n, center, fov_p[p]);
                EXPECT_EQ(a.size(), n);
                
                if (n > 1) {
                    EXPECT_TRUE(Star::within_angle(a, fov_p[p]));
                    EXPECT_TRUE(Star::within_angle(a[0], center, fov_p[p] / 2.0));
                }
                
                EXPECT_NE(center, old_center);
                old_center = center;
            }
        }
    }
}

/// Check that the check for set existence is correct.
TEST(ExperimentQuery, SetExistence) {
    std::vector<Identification::labels_list> a = {{1, 2, 3}, {4, 5, 6}, {6, 7, 8}};
    std::vector<Identification::labels_list> a1 = {{1, 2, 3}, {4, 5, 6}, {6, 7, 8}};
    std::vector<Identification::labels_list> a2 = {{1, 2, 3}, {4, 5, 6}, {6, 7, 8}};
    std::vector<Identification::labels_list> a3 = {};
    
    Identification::labels_list b = {1, 2, 3};
    Identification::labels_list b1 = {6, 4, 5};
    Identification::labels_list b2 = {123, 15123, 12312};
    Identification::labels_list b3 = {1, 2, 3};
    
    EXPECT_TRUE(Experiment::Query::set_existence(a, b));
    EXPECT_TRUE(Experiment::Query::set_existence(a1, b1));
    EXPECT_FALSE(Experiment::Query::set_existence(a1, b2));
    EXPECT_FALSE(Experiment::Query::set_existence(a3, b3));
}

/// Check that the query experiment works for the angle method.
TEST(ExperimentQueryAngle, Trial) {
    std::shared_ptr<Lumberjack> lu;
    std::shared_ptr<INIReader> cf;
    std::shared_ptr<Chomp> ch;
    setup_experiment(cf, ch, lu, "angle", "query", "Angle");
    
    Nibble::tuples_d a = (*lu).search_table("*",
                                            "IdentificationMethod = 'Angle' AND Timestamp = '" + (*lu).timestamp + "'",
                                            100);
    double count_b = a.size();
    
    Experiment::Query::trial<Angle>((*ch), (*lu), (*cf), "angle");
    (*lu).flush_buffer();
    
    Nibble::tuples_d b = (*lu).search_table("Sigma1, Sigma2, Sigma3, ShiftDeviation, CandidateSetSize, SExistence",
                                            "IdentificationMethod = 'Angle' AND Timestamp = '" + (*lu).timestamp + "'",
                                            10);
    ASSERT_EQ(b.size(), count_b + 5);
    
    for (const Nibble::tuple_d b_d : b) {
        EXPECT_EQ(b_d[0], (*cf).GetReal("query-sigma", "angle-1", 0));
        EXPECT_EQ(b_d[1], (*cf).GetReal("query-sigma", "angle-2", 0));
        EXPECT_EQ(b_d[2], (*cf).GetReal("query-sigma", "angle-3", 0));
        EXPECT_THAT(b_d[3], IsBetween(0, (*cf).GetReal("query-experiment", "ss-step", 0)
                                         * ((*cf).GetReal("query-experiment", "ss-iter", 0) - 1)));
        EXPECT_THAT(b_d[4], IsBetween(0, 1));
    }
    
    SQLite::Transaction transaction(*(*lu).conn);
    (*(*lu).conn).exec(
        "DELETE FROM QUERY WHERE Timestamp = '" + (*lu).timestamp + "' AND IdentificationMethod = 'Angle'");
    transaction.commit();
}

/// Check that lists are correctly identified.
TEST(ExperimentReduction, IsCorrectlyIdentified) {
    Star::list a = {Star(0, 0, 0, 1), Star(0, 0, 0, 2), Star(0, 0, 0, 3)};
    Identification::labels_list b = {1, 2, 3}, c = {3, 2, 1}, d = {0, 0, 0};
    
    EXPECT_TRUE(Experiment::Reduction::is_correctly_identified(a, b));
    EXPECT_TRUE(Experiment::Reduction::is_correctly_identified(a, c));
    EXPECT_FALSE(Experiment::Reduction::is_correctly_identified(a, d));
}

/// Check that the reduction experiment works for the angle method.
TEST(ExperimentReductionAngle, Trial) {
    std::shared_ptr<Lumberjack> lu;
    std::shared_ptr<INIReader> cf;
    std::shared_ptr<Chomp> ch;
    setup_experiment(cf, ch, lu, "angle", "reduction", "Angle");
    
    Nibble::tuples_d a = (*lu).search_table("*",
                                            "IdentificationMethod = 'Angle' AND Timestamp = '" + (*lu).timestamp + "'",
                                            100);
    double count_b = a.size();
    
    Experiment::Reduction::trial<Angle>((*ch), (*lu), (*cf), "angle");
    (*lu).flush_buffer();
    
    Nibble::tuples_d b = (*lu).search_table("Sigma1, Sigma2, Sigma3, Sigma4, ShiftDeviation, CameraSensitivity, "
                                                "ResultMatchesInput",
                                            "IdentificationMethod = 'Angle' AND Timestamp = '" + (*lu).timestamp + "'",
                                            10);
    ASSERT_EQ(b.size(), count_b + (5 * 5));
    
    for (const Nibble::tuple_d b_d : b) {
        EXPECT_EQ(b_d[0], (*cf).GetReal("query-sigma", "angle-1", 0));
        EXPECT_EQ(b_d[1], (*cf).GetReal("query-sigma", "angle-2", 0));
        EXPECT_EQ(b_d[2], (*cf).GetReal("query-sigma", "angle-3", 0));
        EXPECT_EQ(b_d[3], (*cf).GetReal("id-parameters", "so", 0));
        EXPECT_THAT(b_d[4], IsBetween(0, (*cf).GetReal("reduction-experiment", "ss-step", 0)
                                         * ((*cf).GetReal("reduction-experiment", "ss-iter", 0) - 1)));
        EXPECT_THAT(b_d[5], IsBetween((*cf).GetReal("reduction-experiment", "mb-min", 0),
                                      (*cf).GetReal("reduction-experiment", "mb-min", 0)
                                      + (*cf).GetReal("reduction-experiment", "mb-step", 0)
                                        * ((*cf).GetReal("reduction-experiment", "mb-iter", 0) - 1)));
        EXPECT_THAT(b_d[6], IsBetween(0, 1));
    }
    
    SQLite::Transaction transaction(*(*lu).conn);
    (*(*lu).conn).exec(
        "DELETE FROM REDUCTION WHERE Timestamp = '" + (*lu).timestamp + "' AND IdentificationMethod = 'Angle'");
    transaction.commit();
}

TEST(ExperimentIdentification, PercentageCorrect) {
    Chomp ch;
    Star::list a = {ch.query_hip(26220), ch.query_hip(26221), ch.query_hip(26235), ch.query_hip(26224),
        ch.query_hip(26427)};
    
    Star::list c = {ch.query_hip(26221), ch.query_hip(26235), ch.query_hip(26220)};
    Star::list d = {ch.query_hip(26220), ch.query_hip(26221), ch.query_hip(262220)};
    Star::list e = {ch.query_hip(1), ch.query_hip(2), ch.query_hip(3)};
    
    EXPECT_FLOAT_EQ(1.0, Experiment::Map::percentage_correct(a, a));
    EXPECT_FLOAT_EQ(1.0, Experiment::Map::percentage_correct(a, c));
    EXPECT_FLOAT_EQ((2.0 / 3.0), Experiment::Map::percentage_correct(a, d));
    EXPECT_FLOAT_EQ(0.0, Experiment::Map::percentage_correct(a, e));
}

/// Check that the map experiment works for the angle method.
TEST(ExperimentIdentificationAngle, Trial) {
    std::shared_ptr<Lumberjack> lu;
    std::shared_ptr<INIReader> cf;
    std::shared_ptr<Chomp> ch;
    setup_experiment(cf, ch, lu, "angle", "identification", "Angle");
    
    Nibble::tuples_d a = (*lu).search_table("*",
                                            "IdentificationMethod = 'Angle' AND Timestamp = '" + (*lu).timestamp + "'",
                                            1);
    double count_b = a.size();
    
    Experiment::Map::trial<Angle>((*ch), (*lu), (*cf), "angle");
    (*lu).flush_buffer();
    
    Nibble::tuples_d b = (*lu).search_table("Sigma1, Sigma2, Sigma3, Sigma4, ShiftDeviation, CameraSensitivity, "
                                                "FalseStars, ComparisonCount, PercentageCorrect",
                                            "IdentificationMethod = 'Angle' AND Timestamp = '" + (*lu).timestamp + "'",
                                            10);
    ASSERT_EQ(b.size(), count_b + (5 * 5 * 5));
    
    for (const Nibble::tuple_d b_d : b) {
        EXPECT_EQ(b_d[0], (*cf).GetReal("query-sigma", "angle-1", 0));
        EXPECT_EQ(b_d[1], (*cf).GetReal("query-sigma", "angle-2", 0));
        EXPECT_EQ(b_d[2], (*cf).GetReal("query-sigma", "angle-3", 0));
        EXPECT_EQ(b_d[3], (*cf).GetReal("id-parameters", "so", 0));
        EXPECT_THAT(b_d[4], IsBetween(0, (*cf).GetReal("identification-experiment", "ss-step", 0)
                                         * ((*cf).GetReal("identification-experiment", "ss-iter", 0) - 1)));
        EXPECT_THAT(b_d[5], IsBetween((*cf).GetReal("identification-experiment", "mb-min", 0),
                                      (*cf).GetReal("identification-experiment", "mb-min", 0)
                                      + (*cf).GetReal("identification-experiment", "mb-step", 0)
                                        * ((*cf).GetReal("identification-experiment", "mb-iter", 0) - 1)));
        EXPECT_THAT(b_d[6], IsBetween((*cf).GetReal("identification-experiment", "es-min", 0),
                                      (*cf).GetReal("identification-experiment", "es-min", 0)
                                      + (*cf).GetReal("identification-experiment", "es-step", 0)
                                        * ((*cf).GetReal("identification-experiment", "es-iter", 0) - 1)));
        EXPECT_THAT(b_d[7], IsBetween(1, (*cf).GetReal("id-parameters", "nu-m", 0)));
        EXPECT_THAT(b_d[8], IsBetween(0, 1));
    }
    
    SQLite::Transaction transaction(*(*lu).conn);
    (*(*lu).conn).exec(
        "DELETE FROM IDENTIFICATION WHERE Timestamp = '" + (*lu).timestamp + "' AND IdentificationMethod = 'Angle'");
    transaction.commit();
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