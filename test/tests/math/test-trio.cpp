/// @file test-trio.cpp
/// @author Glenn Galvizo
///
/// Source file for all Trio class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "math/rotation.h"
#include "math/trio.h"
#include "gtest/gtest.h"

/// Check that the side lengths generated for a planar triangle are correct. Answers checked with WolframAlpha.
TEST(TrioPlanar, LengthsComputation) {
    std::array<double, 3> a = Trio(Star(1, 1, 1), Star(5, 2, 0), Star(-1, -7, 5)).planar_lengths();
    EXPECT_NEAR(a[0], 4.24264, 0.0001);
    EXPECT_NEAR(a[1], 11.9164, 0.001);
    EXPECT_NEAR(a[2], 9.16515, 0.0001);
}

/// Check that the side lengths generated for a spherical triangle are correct.
TEST(TrioSpherical, LengthsComputation) {
    Trio a(Star(1, 1, 1), Star(1, -1, 1), Star(-1, -1, 5));
    std::array<double, 3> b = a.spherical_lengths();
    auto compute_length = [] (const Star &beta_1, const Star &beta_2) -> double {
        return acos(Star::dot(beta_1, beta_2) / (beta_1.norm() * beta_2.norm()));
    };
    EXPECT_DOUBLE_EQ(b[0], compute_length(a.b_1, a.b_2));
    EXPECT_DOUBLE_EQ(b[1], compute_length(a.b_2, a.b_3));
    EXPECT_DOUBLE_EQ(b[2], compute_length(a.b_3, a.b_1));
}

/// Check that the semi perimeter is correctly computed. The should be half the triangle's perimeter. Answers checked
/// with WolframAlpha.
TEST(TrioCommon, SemiPerimeterComputation) {
    Trio a(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    std::array<double, 3> b = a.planar_lengths();
    double c = 13.1448 / 2.0;
    EXPECT_NEAR(a.semi_perimeter(b[0], b[1], b[2]), c, 0.0001);
}

/// Check the planar_area method. Testing involves using the approach found in the link below to verify that both
/// formulas return a close answer: https://www.algebra.com/algebra/homework/Vectors/Vectors.faq.question.674684.html
TEST(TrioPlanar, AreaComputation) {
    Trio a(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    Star b = Star::cross(a.b_1 - a.b_2, a.b_1 - a.b_3);
    EXPECT_NEAR(b.norm() * 0.5, Trio::planar_area(a.b_1, a.b_2, a.b_3), 0.00000000001);
}

/// Check the planar_moment method. According to the website below, the polar moment for an equilateral triangle is
/// 0.036s^4, where s=triangle side length. This test verifies this:
/// http://www.engineersedge.com/polar-moment-inertia.htm
TEST(TrioPlanar, MomentComputation) {
    Trio a(Star(1, 0, 0), Star(0, 1, 0), Star(0, 0, 1));
    std::array<double, 3> kaph_tau = a.planar_lengths();
    double b = 0.036 * pow(kaph_tau[0], 4);
    EXPECT_NEAR(b, Trio::planar_moment(a.b_1, a.b_2, a.b_3), 0.001);
}

/// Check the spherical_area method. We are only checking if this function returns the same result for different
/// triangle configurations.
TEST(TrioSpherical, AreaComputation) {
    std::random_device seed;
    double epsilon = 0.00000000001;
    for (int i = 0; i < 10; i++) {
        std::array<Star, 3> t = {Star::chance(seed), Star::chance(seed), Star::chance(seed)};
        EXPECT_NEAR(Trio::spherical_area(t[0], t[1], t[2]), Trio::spherical_area(t[1], t[2], t[0]), epsilon);
        EXPECT_NEAR(Trio::spherical_area(t[0], t[1], t[2]), Trio::spherical_area(t[2], t[1], t[0]), epsilon);
        EXPECT_NEAR(Trio::spherical_area(t[0], t[1], t[2]), Trio::spherical_area(t[1], t[0], t[2]), epsilon);
        EXPECT_NEAR(Trio::spherical_area(t[0], t[1], t[2]), Trio::spherical_area(t[2], t[0], t[1]), epsilon);
    }
}

/// Check the planar_centroid method. Answers checked with WolframAlpha.
TEST(TrioCommon, PlanarCentroidComputation) {
    Trio a(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    Star b(0.66666666666666666, 1.6666666666666666, 1.0), c = a.planar_centroid();
    Trio d(Star(-1, 0, -1), Star(1, 1, 1), Star(2, 4, 3));
    Star e = d.planar_centroid();
    EXPECT_EQ(e, c);
    EXPECT_EQ(b, c);
}

/// Check the spherical_moment method. We are only checking if this function returns the same result for different
/// triangle configurations.
TEST(TrioSpherical, MomentComputation) {
    std::random_device seed;
    double epsilon = 0.00000000001;
    for (int i = 0; i < 10; i++) {
        std::array<Star, 3> t = {Star::chance(seed), Star::chance(seed), Star::chance(seed)};
        
        EXPECT_NEAR(Trio::spherical_moment(t[0], t[1], t[2]), Trio::spherical_moment(t[1], t[2], t[0]), epsilon);
        EXPECT_NEAR(Trio::spherical_moment(t[0], t[1], t[2]), Trio::spherical_moment(t[2], t[1], t[0]), epsilon);
        EXPECT_NEAR(Trio::spherical_moment(t[0], t[1], t[2]), Trio::spherical_moment(t[1], t[0], t[2]), epsilon);
        EXPECT_NEAR(Trio::spherical_moment(t[0], t[1], t[2]), Trio::spherical_moment(t[2], t[0], t[1]), epsilon);
    }
}

/// This is not a test. We just want to see the effect of shifting stars on the planar area and moment.
TEST(TrioPlanar, TriangleShifts) {
    std::random_device seed;
    
    for (int i = 0; i < 100; i++) {
        std::array<Star, 3> t_original = {Star(1 - 0.001, 0, 0), Star(0, 1 - 0.001, 0), Star(0, 0, 1 - 0.001)};
        std::array<Star, 3> t_shaken = {Rotation::shake(t_original[0], 0.001, seed),
            Rotation::shake(t_original[1], 0.001, seed), Rotation::shake(t_original[2], 0.001, seed)};
        
        double a_original = Trio::planar_area(t_original[0], t_original[1], t_original[2]);
        double i_original = Trio::planar_moment(t_original[0], t_original[1], t_original[2]);
        double a_shaken = Trio::planar_area(t_shaken[0], t_shaken[1], t_shaken[2]);
        double i_shaken = Trio::planar_moment(t_shaken[0], t_shaken[1], t_shaken[2]);
        RecordProperty("ShiftArea", std::to_string(fabs(a_original - a_shaken)));
        RecordProperty("ShiftMoment", std::to_string(fabs(i_original - i_shaken)));
    }
}

/// This is not a test. We just want to see the effect of shifting stars on the spherical area and moment.
TEST(TrioSpherical, TriangleShifts) {
    std::random_device seed;
    
    for (int i = 0; i < 100; i++) {
        std::array<Star, 3> t_original = {Star(1 - 0.001, 0, 0), Star(0, 1 - 0.001, 0), Star(0, 0, 1 - 0.001)};
        std::array<Star, 3> t_shaken = {Rotation::shake(t_original[0], 0.001, seed),
            Rotation::shake(t_original[1], 0.001, seed), Rotation::shake(t_original[2], 0.001, seed)};
        
        double a_original = Trio::spherical_area(t_original[0], t_original[1], t_original[2]);
        double i_original = Trio::spherical_moment(t_original[0], t_original[1], t_original[2]);
        double a_shaken = Trio::spherical_area(t_shaken[0], t_shaken[1], t_shaken[2]);
        double i_shaken = Trio::spherical_moment(t_shaken[0], t_shaken[1], t_shaken[2]);
        RecordProperty("ShiftArea", std::to_string(fabs(a_original - a_shaken)));
        RecordProperty("ShiftMoment", std::to_string(fabs(i_original - i_shaken)));
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