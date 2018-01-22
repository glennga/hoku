/// @file test-experiment.cpp
/// @author Glenn Galvizo
///
/// Source file for all Lumberjack class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "experiment/lumberjack.h"
#include "gmock/gmock.h"

// Import several matchers from Google Mock.
using testing::Contains;

///
TEST(LumberjackTable, )

/// Check that query_for_pair method returns the catalog ID of the correct stars.
TEST(AngleQuery, Pair) {
    Chomp ch;
    Benchmark input(ch, 15);
    
    double a = Star::angle_between(input.stars[0], input.stars[1]);
    Identification::labels_list b = Angle(input, Angle::DEFAULT_PARAMETERS).query_for_pair(a);
    std::vector<int> c = {input.stars[0].get_label(), input.stars[1].get_label()};
    std::vector<int> d = {input.stars[0].get_label(), input.stars[1].get_label()};
    EXPECT_THAT(c, Contains(b[0]));
    EXPECT_THAT(d, Contains(b[1]));
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