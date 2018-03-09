/// @file perform-t.cpp
/// @author Glenn Galvizo
///
/// Runner program to execute all tests.

#include "gtest/gtest.h"

#include "benchmark/test-benchmark.cpp"
#include "experiment/test-lumberjack.cpp"
#include "experiment/test-experiment.cpp"
#include "identification/test-angle.cpp"
#include "identification/test-base-triangle.cpp"
#include "identification/test-composite-pyramid.cpp"
#include "identification/test-dot-angle.cpp"
#include "identification/test-identification.cpp"
#include "identification/test-planar-triangle.cpp"
#include "identification/test-pyramid.cpp"
#include "identification/test-spherical-triangle.cpp"
#include "math/test-mercator.cpp"
#include "math/test-random-draw.cpp"
#include "math/test-rotation.cpp"
#include "math/test-star.cpp"
#include "math/test-trio.cpp"
#include "storage/test-chomp.cpp"
#include "storage/test-nibble.cpp"

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}