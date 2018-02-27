/// @file test-chomp.cpp
/// @author Glenn Galvizo
///
/// Source file for all Chomp class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#define _USE_MATH_DEFINES
#include "gmock/gmock.h"
#include <fstream>

#include "storage/chomp.h"


// Create an in-between matcher for Google Mock.
using testing::PrintToString;
MATCHER_P2(IsBetween, a, b,
           std::string(negation ? "isn't" : "is") + " between " + PrintToString(a) + " and " + PrintToString(b)) {
    return a <= arg && arg <= b;
}

/// Check that the constructor correctly sets the right fields, and generates the HIP and BRIGHT tables.
/// 'load_tables' is tested here as well.
TEST(ChompConstructor, NoArguments) {
    INIReader cf(std::string(std::getenv("HOKU_PROJECT_PATH")) + std::string("/CONFIG.ini"));
    std::string hip = cf.Get("table-names", "hip", "");
    std::string bright = cf.Get("table-names", "bright", "");
    
    Nibble nb;
    (*nb.conn).exec("DROP TABLE IF EXISTS " + hip);
    (*nb.conn).exec("DROP TABLE IF EXISTS " + bright);
    
    // These tables should not exist now.
    EXPECT_THROW(nb.select_table(hip, true);, std::runtime_error);
    EXPECT_THROW(nb.select_table(bright, true);, std::runtime_error);
    
    // Ensure the fields are set correctly.
    Chomp ch;
    EXPECT_EQ(ch.bright_table, bright);
    EXPECT_EQ(ch.hip_table, hip);
    EXPECT_EQ(ch.all_bright_stars.size(), Chomp::BRIGHT_TABLE_LENGTH);
    EXPECT_EQ(ch.all_hip_stars.size(), Chomp::HIP_TABLE_LENGTH);
    
    // Tables should exist in nibble.db
    EXPECT_NO_THROW(ch.select_table(bright););
    EXPECT_NO_THROW(ch.select_table(hip););
}

/// Check that the in-memory instance constructor provides Chomp query functionality, and the table can be queried.
/// Components checked with Matlab's sph2cart.
TEST(ChompConstructor, InMemory) {
    INIReader cf(std::string(std::getenv("HOKU_PROJECT_PATH")) + std::string("/CONFIG.ini"));
    std::string hip = cf.Get("table-names", "hip", "");
    std::string bright = cf.Get("table-names", "bright", "");
    Chomp ch(bright, "label");
    
    // Ensure the fields are set correctly.
    EXPECT_EQ(ch.bright_table, bright);
    EXPECT_EQ(ch.hip_table, hip);
    EXPECT_EQ(ch.all_bright_stars.size(), Chomp::BRIGHT_TABLE_LENGTH);
    EXPECT_EQ(ch.all_hip_stars.size(), Chomp::HIP_TABLE_LENGTH);
    
    // No other table other than the bright table should exist in the database.
    EXPECT_THROW(ch.select_table(hip, true);, std::runtime_error);
    EXPECT_NO_THROW(ch.select_table(bright, true););
    
    // Ensure that the correct results are returned from a query of the BRIGHT table.
    EXPECT_EQ(ch.search_table("i, j, k", "label = 1", 1).size(), 0);
    EXPECT_EQ(ch.search_table("i, j, k", "label = 88", 1).size(), 1);
    Nibble::tuples_d a = ch.search_table("i, j, k", "label = 88", 3);
    EXPECT_FLOAT_EQ(a[0][0], 0.658552173330720);
    EXPECT_FLOAT_EQ(a[0][1], 0.003092250084512);
    EXPECT_FLOAT_EQ(a[0][2], -0.752528719047187);
}

/// Check that components are correctly parsed from the given line.
TEST(ChompStarTable, ComponentsFromLineNoYear) {
    std::array<double, 7> a = {0.000911850889839031, 1.08901336539477, 0.999819374779962, 1.59119257019658e-05,
        0.0190057244380963, 9.20429992675781, 1};
    std::string b = "     1|  5|0|1| 0.0000159148  0.0190068680|   4.55|   -4.55|   -1.19|  1.29|  0.66|  1.33|  1.25| "
        " 0.75| 90| 0.91| 0|   0.0|   0| 9.2043|0.0020|0.017|0| 0.482|0.025| 0.550|   1.19  -0.71   1.00  -0.02   0.02 "
        "1.00   0.45  -0.05   0.03   1.09  -0.41   0.09   0.08  -0.60   1.00";
    std::array<double, 7> c = Chomp().components_from_line(b, 0);
    
    EXPECT_NEAR(a[0], c[0], 0.000001);
    EXPECT_NEAR(a[1], c[1], 0.000001);
    EXPECT_NEAR(a[2], c[2], 0.000001);
    EXPECT_NEAR(a[3], c[3], 0.000001);
    EXPECT_NEAR(a[4], c[4], 0.000001);
    EXPECT_NEAR(a[5], c[5], 0.01);
    EXPECT_EQ(a[6], c[6]);
}

/// Check that components are correctly parsed from the given line, using a one year difference.
TEST(ChompStarTable, ComponentsFromLine1Year) {
    std::array<double, 7> a = {0.0009105870008971603, 1.0890130348391953, 0.99981937488996198, 1.5889870664709241e-005,
        0.019005718669855325, 9.2042999267578125, 1};
    std::string b = "     1|  5|0|1| 0.0000159148  0.0190068680|   4.55|   -4.55|   -1.19|  1.29|  0.66|  1.33|  1.25| "
        " 0.75| 90| 0.91| 0|   0.0|   0| 9.2043|0.0020|0.017|0| 0.482|0.025| 0.550|   1.19  -0.71   1.00  -0.02   0.02 "
        "1.00   0.45  -0.05   0.03   1.09  -0.41   0.09   0.08  -0.60   1.00";
    std::array<double, 7> c = Chomp().components_from_line(b, 1);
    
    EXPECT_NEAR(a[0], c[0], 0.000000001);
    EXPECT_NEAR(a[1], c[1], 0.000000001);
    EXPECT_NEAR(a[2], c[2], 0.000000001);
    EXPECT_NEAR(a[3], c[3], 0.000000001);
    EXPECT_NEAR(a[4], c[4], 0.00001);
    EXPECT_NEAR(a[5], c[5], 0.00001);
    EXPECT_EQ(a[6], c[6]);
}

/// Check that the year difference method fails when appropriate, and returns the correct time.
TEST(ChompStarTable, YearDifference) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    std::string temp_path = std::getenv("TEMP");
#else
    std::string temp_path = "/tmp";
#endif
    
    // Create an invalid and valid CONFIG file.
    std::ofstream f1(temp_path + "/year-temp-hoku-wrong.ini");
    std::ofstream f2(temp_path + "/year-temp-hoku-correct.ini");
    f1 << "[hardware]\ntime = asd" << std::endl;
    f2 << "[hardware]\ntime = 03-1992" << std::endl;
    f1.close(), f2.close();
    
    INIReader cf1(temp_path + "/year-temp-hoku-wrong.ini");
    INIReader cf2(temp_path + "/year-temp-hoku-correct.ini");
    EXPECT_THROW(Chomp::year_difference(cf1), std::runtime_error);
    EXPECT_EQ(Chomp::year_difference(cf2), 1);
}

/// Check that the both the bright stars table and the hip table are present after running the generators.
TEST(ChompStarTable, TableExistence) {
    Chomp ch;
    INIReader cf(std::string(std::getenv("HOKU_PROJECT_PATH")) + std::string("/CONFIG.ini"));
    ch.bright_table = cf.Get("table-names", "bright", "");
    ch.hip_table = cf.Get("table-names", "hip", "");
    ch.generate_table(cf, true);
    ch.generate_table(cf, false);
    
    bool assertion = true;
    try {
        ch.select_table(ch.bright_table, true);
        ch.select_table(ch.bright_table, false);
    }
    catch (std::exception &e) {
        assertion = false;
    }
    EXPECT_TRUE(assertion);
}

/// Check that the angle between NU.03 Canis Majoris and Alpha Canis Majoris (Sirius) are correct.
TEST(ChompStarTable, CorrectAngleBetweenStars) {
    Chomp ch;
    Star a = ch.query_hip(31700), b = ch.query_hip(32349);
    EXPECT_NEAR(2.3011, Vector3::Angle(a, b) * (180.0 / M_PI), 0.005);
}

/// Check that NU.03 Canis Majoris has the correct elements. Using KStars for current position.
TEST(ChompQuery, Query31700) {
    Chomp ch;
    Nibble::tuples_d a = ch.search_table("*", "label = 31700", 1);
    
    EXPECT_NEAR(a[0][0], 99.6708, 0.5);
    EXPECT_NEAR(a[0][1], -18.2592, 0.5);
    EXPECT_FLOAT_EQ(a[0][5], 4.5975);
}

/// Check that Alpha Canis Majoris has the correct elements. Using KStars for current position.
TEST(ChompQuery, Query32349) {
    Chomp ch;
    Nibble::tuples_d a = ch.search_table("*", "label = 32349", 1);
    
    EXPECT_NEAR(a[0][0], 101.4875, 0.5);
    EXPECT_NEAR(a[0][1], -16.7439, 0.5);
    EXPECT_FLOAT_EQ(a[0][5], -1.0876);
}

/// Check that the hip query method returns the expected values.
TEST(ChompQuery, Hip) {
    Star a = Chomp().query_hip(3);
    EXPECT_DOUBLE_EQ(a[0], 0.778689441368632);
    EXPECT_DOUBLE_EQ(a[1], 6.84644278384085e-05);
    EXPECT_DOUBLE_EQ(a[2], 0.627409554608177);
}

/// Check that the results returned from bright_as_list are correct.
TEST(ChompLists, BrightStarGrab) {
    Chomp ch;
    Star::list a = ch.bright_as_list();
    Star b = ch.query_hip(88), c = ch.query_hip(107), d = ch.query_hip(122);
    Star e = ch.query_hip(124), f = ch.query_hip(118322);
    
    EXPECT_EQ(a[0], b);
    EXPECT_EQ(a[1], c);
    EXPECT_EQ(a[2], d);
    EXPECT_EQ(a[3], e);
    EXPECT_EQ(a[Chomp::BRIGHT_TABLE_LENGTH - 1], f);
}

/// Check that the first 10 bright stars returned are all nearby the focus.
TEST(ChompNearby, NearbyBrightStars) {
    Chomp ch;
    Star focus = Star::chance();
    std::vector<Star> nearby = ch.nearby_bright_stars(focus, 7.5, 30);
    for (int q = 0; q < 10; q++) {
        EXPECT_TRUE(Star::within_angle(nearby[q], focus, 7.5));
    }
}

/// Check that the first 10 stars returned are all nearby the focus.
TEST(ChompNearby, NearbyHipStars) {
    Chomp ch;
    Star focus = Star::chance();
    std::vector<Star> nearby = ch.nearby_hip_stars(focus, 5, 100);
    for (int q = 0; q < 10; q++) {
        EXPECT_TRUE(Star::within_angle(nearby[q], focus, 5));
    }
}

/// Check that the regular query returns correct results. This test is just used to compare against the k-vector
/// query time.
TEST(ChompQuery, SimpleBound) {
    Chomp ch;
    ch.select_table("ANGLE_20");
    Nibble::tuples_d a = ch.simple_bound_query("theta", "theta", 5.004, 5.005, 90);
    for (Nibble::tuple_d &q : a) {
        EXPECT_THAT(q[0], IsBetween(5.003, 5.006));
    }
}

/// Verify that the k-vector query table is correct. This tests the helper method 'build_k_vector_table' as well.
TEST(ChompKVector, KVectorCreate) {
    // TODO: Finish this test. K-Vector is not being used at the moment, so this is not a huge priority,
}

/// Check that the k-vector query returns the correct results.
TEST(ChompKVector, KVectorQuery) {
    // TODO: Finish this test. K-Vector is not being used at the moment, so this is not a huge priority,
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