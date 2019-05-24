/// @file perform-t.cpp
/// @author Glenn Galvizo
///
/// Runner program to execute all tests.

#include "gtest/gtest.h"

#include "math/test-random-draw.cpp"
#include "math/test-star.cpp"
#include "math/test-rotation.cpp"
#include "math/test-trio.cpp"
#include "storage/test-nibble.cpp"
#include "storage/test-chomp.cpp"
#include "benchmark/test-benchmark.cpp"
#include "identification/test-identification.cpp"
//#include "experiment/test-lumberjack.cpp"
//#include "experiment/test-experiment.cpp"

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}