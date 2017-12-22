/// @file test-chomp.cpp
/// @author Glenn Galvizo
///
/// Source file for all Chomp class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "storage/chomp.h"
#include "gmock/gmock.h"

// Create an in-between matcher for Google Mock.
using testing::PrintToString;
MATCHER_P2(IsBetween, a, b,
           std::string(negation ? "isn't" : "is") + " between " + PrintToString(a) + " and " + PrintToString(b)) {
    return a <= arg && arg <= b;
}

/// Check that the results returned from bright_as_list are correct.
TEST(ChompLists, BrightStarGrab) {
    Chomp ch;
    Star::list a = ch.bright_as_list();
    Star b = ch.query_hip(88), c = ch.query_hip(107), d = ch.query_hip(122);
    Star e = ch.query_hip(124), f = ch.query_hip(117930);
    
    EXPECT_EQ(a[0], b);
    EXPECT_EQ(a[1], c);
    EXPECT_EQ(a[2], d);
    EXPECT_EQ(a[3], e);
    EXPECT_EQ(a[Chomp::BRIGHT_TABLE_LENGTH - 1], f);
}

/// Check that the first 10 bright stars returned are all nearby the focus.
TEST(ChompLists, NearbyBrightStars) {
    std::random_device seed;
    Chomp ch;
    Star focus = Star::chance(seed);
    std::vector<Star> nearby = ch.nearby_bright_stars(focus, 7.5, 30);
    for (int q = 0; q < 10; q++) {
        EXPECT_TRUE(Star::within_angle(nearby[q], focus, 7.5));
    }
}

/// Check that the first 10 stars returned are all nearby the focus.
TEST(ChompLists, NearbyHipStars) {
    std::random_device seed;
    Chomp ch;
    Star focus = Star::chance(seed);
    std::vector<Star> nearby = ch.nearby_hip_stars(focus, 5, 100);
    for (int q = 0; q < 10; q++) {
        EXPECT_TRUE(Star::within_angle(nearby[q], focus, 5));
    }
}

/// Check that components are correctly parsed from the given line.
TEST(ChompStarTable, ComponentsFromLine) {
    std::array<double, 6> a = {0.000911850889839031, 1.08901336539477, 0.999819374779962, 1.59119257019658e-05,
        0.0190057244380963, 9.20429992675781};
    std::string b = "     1|  5|0|1| 0.0000159148  0.0190068680|   4.55|   -4.55|   -1.19|  1.29|  0.66|  1.33|  1.25| "
        " 0.75| 90| 0.91| 0|   0.0|   0| 9.2043|0.0020|0.017|0| 0.482|0.025| 0.550|   1.19  -0.71   1.00  -0.02   0.02 "
        "1.00   0.45  -0.05   0.03   1.09  -0.41   0.09   0.08  -0.60   1.00";
    std::array<double, 6> c = Chomp().components_from_line(b);
    
    EXPECT_NEAR(a[0], c[0], 0.000001);
    EXPECT_NEAR(a[1], c[1], 0.000001);
    EXPECT_NEAR(a[2], c[2], 0.000001);
    EXPECT_NEAR(a[3], c[3], 0.000001);
    EXPECT_NEAR(a[4], c[4], 0.000001);
    EXPECT_NEAR(a[5], c[5], 0.01);
}

/// Check that the both the bright stars table and the hip table are present after running the generators.
TEST(ChompStarTable, TableExistence) {
    Chomp ch;
    ch.generate_hip_table();
    ch.generate_bright_table();
    EXPECT_EQ(ch.generate_hip_table(), Chomp::TABLE_EXISTS);
    EXPECT_EQ(ch.generate_bright_table(), Chomp::TABLE_EXISTS);
}

/// Check that the hip query method returns the expected values.
TEST(ChompQuery, Hip) {
    Star a = Chomp().query_hip(3);
    EXPECT_DOUBLE_EQ(a[0], 0.778689180572338);
    EXPECT_DOUBLE_EQ(a[1], 6.80614031952957e-05);
    EXPECT_DOUBLE_EQ(a[2], 0.627409878330925);
}

/// Check that the regular query returns correct results. This test is just used to compare against the k-vector
/// query time.
TEST(ChompQuery, SimpleBound) {
    Chomp ch;
    ch.select_table("PYRA_20");
    Nibble::tuples_d a = ch.simple_bound_query("theta", "theta", 5.004, 5.005, 90);
    for (Nibble::tuple_d &q : a) {
        EXPECT_THAT(q[0], IsBetween(5.003, 5.006));
    }
}

/// Check that the k-vector query returns the correct results.
TEST(ChompQuery, KVector) {
    Chomp ch;
    ch.select_table("PYRA_20");
    Nibble::tuples_d a = ch.k_vector_query("theta", "theta", 5.004, 5.005, 90);
    for (Nibble::tuple_d &q : a) {
        EXPECT_THAT(q[0], IsBetween(5.003, 5.006));
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