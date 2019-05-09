/// @file test-trio.cpp
/// @author Glenn Galvizo
///
/// Source file for all Trio class unit tests.

#include <cmath>
#include "gtest/gtest.h"

#include "math/rotation.h"
#include "math/trio.h"

TEST(Trio, PlanarAreaComputation) {
    Vector3 a_1 = Vector3(1, 1, 1);
    Vector3 a_2 = Vector3(-1, 0, -1);
    Vector3 a_3 = Vector3(2, 4, 3);
    Vector3 b = Vector3::Cross(a_1 - a_2, a_1 - a_3);
    EXPECT_NEAR(Vector3::Magnitude(b) * 0.5, Trio::planar_area(a_1, a_2, a_3), 0.00000000001);
}

TEST(Trio, SphericalAreaComputation) {
    double epsilon = 0.00000000001;
    for (int i = 0; i < 30; i++) {
        std::array<Star, 3> t = {Star::chance(), Star::chance(), Star::chance()};
        EXPECT_NEAR(Trio::spherical_area(t[0], t[1], t[2]).result,
                    Trio::spherical_area(t[1], t[2], t[0]).result, epsilon);
        EXPECT_NEAR(Trio::spherical_area(t[0], t[1], t[2]).result,
                    Trio::spherical_area(t[2], t[1], t[0]).result, epsilon);
        EXPECT_NEAR(Trio::spherical_area(t[0], t[1], t[2]).result,
                    Trio::spherical_area(t[1], t[0], t[2]).result, epsilon);
        EXPECT_NEAR(Trio::spherical_area(t[0], t[1], t[2]).result,
                    Trio::spherical_area(t[2], t[0], t[1]).result, epsilon);
        EXPECT_FALSE(std::isnan(Trio::spherical_area(t[0], t[1], t[2]).result));
    }

    EXPECT_EQ(Trio::spherical_area(Star(1, 1, 1), Star(1, 1, 1), Star(2, 2, 2)).error, 0);
    EXPECT_DOUBLE_EQ(Trio::spherical_area(Star(1, 1, 1), Star(1, 1, 1), Star(2, 2, 2)).result, 0);
}

TEST(Trio, SphericalMomentComputation) {
    double epsilon = 0.0000000001;
    for (int i = 0; i < 10; i++) {
        std::array<Star, 3> t = {Star::chance(), Star::chance(), Star::chance()};

        EXPECT_NEAR(Trio::spherical_moment(t[0], t[1], t[2]).result,
                    Trio::spherical_moment(t[1], t[2], t[0]).result, epsilon);
        EXPECT_NEAR(Trio::spherical_moment(t[0], t[1], t[2]).result,
                    Trio::spherical_moment(t[2], t[1], t[0]).result, epsilon);
        EXPECT_NEAR(Trio::spherical_moment(t[0], t[1], t[2]).result,
                    Trio::spherical_moment(t[1], t[0], t[2]).result, epsilon);
        EXPECT_NEAR(Trio::spherical_moment(t[0], t[1], t[2]).result,
                    Trio::spherical_moment(t[2], t[0], t[1]).result, epsilon);
    }

    EXPECT_EQ(Trio::spherical_moment(Star(1, 1, 1), Star(1, 1, 1), Star(2, 2, 2)).error, 0);
    EXPECT_DOUBLE_EQ(Trio::spherical_moment(Star(1, 1, 1), Star(1, 1, 1), Star(2, 2, 2)).result, 0);
}

TEST(Trio, PlanarTriangleShifts) {
    for (int i = 0; i < 100; i++) {
        std::array<Star, 3> t_original = {Star(1 - 0.001, 0, 0), Star(0, 1 - 0.001, 0), Star(0, 0, 1 - 0.001)};
        std::array<Star, 3> t_shaken = {Rotation::shake(t_original[0], 0.001), Rotation::shake(t_original[1], 0.001),
                                        Rotation::shake(t_original[2], 0.001)};

        double a_original = Trio::planar_area(t_original[0], t_original[1], t_original[2]);
        double i_original = Trio::planar_moment(t_original[0], t_original[1], t_original[2]);
        double a_shaken = Trio::planar_area(t_shaken[0], t_shaken[1], t_shaken[2]);
        double i_shaken = Trio::planar_moment(t_shaken[0], t_shaken[1], t_shaken[2]);
        RecordProperty("ShiftArea", std::to_string(fabs(a_original - a_shaken)));
        RecordProperty("ShiftMoment", std::to_string(fabs(i_original - i_shaken)));
    }
}

TEST(Trio, SphericalTriangleShifts) {
    for (int i = 0; i < 100; i++) {
        std::array<Star, 3> t_original = {Star(1 - 0.001, 0, 0), Star(0, 1 - 0.001, 0), Star(0, 0, 1 - 0.001)};
        std::array<Star, 3> t_shaken = {Rotation::shake(t_original[0], 0.001), Rotation::shake(t_original[1], 0.001),
                                        Rotation::shake(t_original[2], 0.001)};

        double a_original = Trio::spherical_area(t_original[0], t_original[1], t_original[2]).result;
        double i_original = Trio::spherical_moment(t_original[0], t_original[1], t_original[2]).result;
        double a_shaken = Trio::spherical_area(t_shaken[0], t_shaken[1], t_shaken[2]).result;
        double i_shaken = Trio::spherical_moment(t_shaken[0], t_shaken[1], t_shaken[2]).result;
        RecordProperty("ShiftArea", std::to_string(fabs(a_original - a_shaken)));
        RecordProperty("ShiftMoment", std::to_string(fabs(i_original - i_shaken)));
    }
}

TEST(Trio, DotAngle) {
    EXPECT_FLOAT_EQ(0, Trio::dot_angle(Vector3::Forward(), Vector3::Forward(), Vector3::Backward()));
    EXPECT_FLOAT_EQ(180.0, Trio::dot_angle(Vector3::Forward(), Vector3::Normalized(
            Vector3(0, 0, 1 + 1.0e-19)), Vector3::Normalized(
            Vector3(0, 0, 1 - 1.0e-19))));
    EXPECT_FLOAT_EQ(90.0, Trio::dot_angle(Vector3::Forward(), Vector3::Backward(), Vector3::Up()));
}