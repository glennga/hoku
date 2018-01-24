/// @file test-experiment.cpp
/// @author Glenn Galvizo
///
/// Source file for all Experiment class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "experiment/experiment.h"
#include "gmock/gmock.h"

/// Ensure that the benchmark
TEST(ExperimentAll, PresentBenchmark) {
    // 5 stars, is shuffled
}

TEST(ExperimentQuery, GenerateNStars) {

}

TEST(ExperimentQuery, SetExistence) {

}

TEST(ExperimentQuery, Trial) {

}

TEST(ExperimentReduction, IsCorrectlyIdentified) {

}

TEST(ExperimentReduction, Trial) {

}

TEST(ExperimentAlignment, IsCorrectlyAligned) {

}

TEST(ExperimentAlignment, Trial) {

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