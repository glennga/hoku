/// @file test-trio.cpp
/// @author Glenn Galvizo
///
/// Source file for all Trio class unit tests.

#define ENABLE_TESTING_ACCESS

#include <cmath>
#include "gtest/gtest.h"

#include "math/rotation.h"
#include "math/trio.h"

/// Check that the private constructor contains the correct stars passed to it.
TEST(Trio, Constructor) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Vector3 a(1, 1, 1), b(2, 2, 2), c(3, 3, 3);
    Trio t(a, b, c);
    EXPECT_EQ(*t.b_1, a);
    EXPECT_EQ(*t.b_2, b);
    EXPECT_EQ(*t.b_3, c);
}

/// Check that the side lengths generated for a planar triangle are correct. Answers checked with WolframAlpha.
TEST(Trio, PlanarLengthsComputation) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Trio::side_lengths a = Trio(Vector3(1, 1, 1), Vector3(5, 2, 0), Vector3(-1, -7, 5)).planar_lengths();
    EXPECT_NEAR(a[0], 4.24264, 0.0001);
    EXPECT_NEAR(a[1], 11.9164, 0.001);
    EXPECT_NEAR(a[2], 9.16515, 0.0001);
}

/// Check that the side lengths generated for a spherical triangle are correct.
TEST(Trio, SphericalLengthsComputation) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Trio a(Vector3(1, 1, 1), Vector3(1, -1, 1), Vector3(-1, -1, 5));
    Trio c(Vector3(1, 1, 1), Vector3(1, 1, 1), Vector3(-1, -1, 5));
    Trio::side_lengths b = a.spherical_lengths(), d = c.spherical_lengths();
    auto compute_length = [] (const Vector3 &beta_1, const Vector3 &beta_2) -> double {
        return acos(Vector3::Dot(beta_1, beta_2) / (Vector3::Magnitude(beta_1) * Vector3::Magnitude(beta_2)));
    };
    EXPECT_DOUBLE_EQ(b[0], compute_length(*a.b_1, *a.b_2));
    EXPECT_DOUBLE_EQ(b[1], compute_length(*a.b_2, *a.b_3));
    EXPECT_DOUBLE_EQ(b[2], compute_length(*a.b_3, *a.b_1));
    EXPECT_TRUE(std::isnan(d[0]));
}

/// Check that the semi perimeter is correctly computed. The should be half the triangle's perimeter. Answers checked
/// with WolframAlpha.
TEST(Trio, CommonSemiPerimeterComputation) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Trio a(Vector3(1, 1, 1), Vector3(-1, 0, -1), Vector3(2, 4, 3));
    std::array<double, 3> b = a.planar_lengths();
    double c = 13.1448 / 2.0;
    EXPECT_NEAR(a.semi_perimeter(b[0], b[1], b[2]), c, 0.0001);
}

/// Check the planar_area method. Testing involves using the approach found in the link below to verify that both
/// formulas return a close answer: https://www.algebra.com/algebra/homework/Vectors/Vectors.faq.question.674684.html
TEST(Trio, PlanarAreaComputation) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Trio a(Vector3(1, 1, 1), Vector3(-1, 0, -1), Vector3(2, 4, 3));
    Vector3 b = Vector3::Cross(*a.b_1 - *a.b_2, *a.b_1 - *a.b_3);
    EXPECT_NEAR(Vector3::Magnitude(b) * 0.5, Trio::planar_area(*a.b_1, *a.b_2, *a.b_3), 0.00000000001);
}

/// Check the planar_moment method. According to the website below, the polar moment for an equilateral triangle is
/// 0.036s^4, where s=triangle side length. This test verifies this:
/// http://www.engineersedge.com/polar-moment-inertia.htm
TEST(Trio, PlanarMomentComputation) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Trio a(Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1));
    Trio::side_lengths c = a.planar_lengths();
    double b = 0.036 * pow(c[0], 4);
    EXPECT_NEAR(b, Trio::planar_moment(*a.b_1, *a.b_2, *a.b_3), 0.001);
}

/// Check the spherical_area method. We are only checking if this function returns the same result for different
/// triangle configurations, and that no NaN values are ever returned.
TEST(Trio, SphericalAreaComputation) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    double epsilon = 0.00000000001;
    for (int i = 0; i < 30; i++) {
        std::array<Star, 3> t = {Star::chance(), Star::chance(), Star::chance()};
        EXPECT_NEAR(Trio::spherical_area(t[0], t[1], t[2]), Trio::spherical_area(t[1], t[2], t[0]), epsilon);
        EXPECT_NEAR(Trio::spherical_area(t[0], t[1], t[2]), Trio::spherical_area(t[2], t[1], t[0]), epsilon);
        EXPECT_NEAR(Trio::spherical_area(t[0], t[1], t[2]), Trio::spherical_area(t[1], t[0], t[2]), epsilon);
        EXPECT_NEAR(Trio::spherical_area(t[0], t[1], t[2]), Trio::spherical_area(t[2], t[0], t[1]), epsilon);
        EXPECT_FALSE(std::isnan(Trio::spherical_area(t[0], t[1], t[2])));
    }

    EXPECT_DOUBLE_EQ(Trio::spherical_area(Star(1, 1, 1), Star(1, 1, 1), Star(2, 2, 2)), Trio::DUPLICATE_STARS_IN_TRIO);
}

/// Check the planar_centroid method. Answers checked with WolframAlpha.
TEST(Trio, CommonPlanarCentroidComputation) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Trio a(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    Vector3 b(0.66666666666666666, 1.6666666666666666, 1.0), c = a.planar_centroid();
    Trio d(Star(-1, 0, -1), Star(1, 1, 1), Star(2, 4, 3));
    Vector3 e = d.planar_centroid();
    EXPECT_EQ(e, c);
    EXPECT_EQ(b, c);
}

/// Check that the triangle cutting method works as intended.
TEST(Trio, CutTriangle) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    auto p_area = [] (const Trio &t) -> double {
        return Trio::planar_area(*t.b_1, *t.b_2, *t.b_3);
    };
    auto p_perimeter = [] (const Trio &t) -> double {
        Trio::side_lengths a = t.planar_lengths();
        return a[0] + a[1] + a[2];
    };

    for (unsigned int i = 0; i < 20; i++) {
        Trio t(Star::chance(), Star::chance(), Star::chance());
        Trio t_1 = Trio::cut_triangle(*t.b_1, *t.b_2, *t.b_3, 0);
        Trio t_2 = Trio::cut_triangle(*t.b_1, *t.b_2, *t.b_3, 1);
        Trio t_3 = Trio::cut_triangle(*t.b_1, *t.b_2, *t.b_3, 2);
        Trio t_4 = Trio::cut_triangle(*t.b_1, *t.b_2, *t.b_3, 3);

        // Individual areas should **be less** than the total sum. Unit sphere work moves point in Cartesian space.
        EXPECT_LT(p_area(t), p_area(t_1) + p_area(t_2) + p_area(t_3) + p_area(t_4));

        // Individual perimeters should **be less** than the total sum.
        EXPECT_LT(p_perimeter(t), p_perimeter(t_1) + p_perimeter(t_2) + p_perimeter(t_3));
    }
}

/// Check the spherical_moment method. We are only checking if this function returns the same result for different
/// triangle configurations.
TEST(Trio, SphericalMomentComputation) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    double epsilon = 0.0000000001;
    for (int i = 0; i < 10; i++) {
        std::array<Star, 3> t = {Star::chance(), Star::chance(), Star::chance()};

        EXPECT_NEAR(Trio::spherical_moment(t[0], t[1], t[2]), Trio::spherical_moment(t[1], t[2], t[0]), epsilon);
        EXPECT_NEAR(Trio::spherical_moment(t[0], t[1], t[2]), Trio::spherical_moment(t[2], t[1], t[0]), epsilon);
        EXPECT_NEAR(Trio::spherical_moment(t[0], t[1], t[2]), Trio::spherical_moment(t[1], t[0], t[2]), epsilon);
        EXPECT_NEAR(Trio::spherical_moment(t[0], t[1], t[2]), Trio::spherical_moment(t[2], t[0], t[1]), epsilon);
    }

    EXPECT_DOUBLE_EQ(Trio::spherical_moment(Star(1, 1, 1), Star(1, 1, 1), Star(2, 2, 2)),
                     Trio::DUPLICATE_STARS_IN_TRIO);
}

/// This is not a test. We just want to see the effect of shifting stars on the planar area and moment.
TEST(Trio, PlanarTriangleShifts) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
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

/// This is not a test. We just want to see the effect of shifting stars on the spherical area and moment.
TEST(Trio, SphericalTriangleShifts) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    for (int i = 0; i < 100; i++) {
        std::array<Star, 3> t_original = {Star(1 - 0.001, 0, 0), Star(0, 1 - 0.001, 0), Star(0, 0, 1 - 0.001)};
        std::array<Star, 3> t_shaken = {Rotation::shake(t_original[0], 0.001), Rotation::shake(t_original[1], 0.001),
                                        Rotation::shake(t_original[2], 0.001)};

        double a_original = Trio::spherical_area(t_original[0], t_original[1], t_original[2]);
        double i_original = Trio::spherical_moment(t_original[0], t_original[1], t_original[2]);
        double a_shaken = Trio::spherical_area(t_shaken[0], t_shaken[1], t_shaken[2]);
        double i_shaken = Trio::spherical_moment(t_shaken[0], t_shaken[1], t_shaken[2]);
        RecordProperty("ShiftArea", std::to_string(fabs(a_original - a_shaken)));
        RecordProperty("ShiftMoment", std::to_string(fabs(i_original - i_shaken)));
    }
}

/// Check that the computed dot angle is consistent at 0 degrees, 90 degrees, and 180 degrees.
TEST(Trio, DotAngle) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    EXPECT_FLOAT_EQ(0, Trio::dot_angle(Vector3::Forward(), Vector3::Forward(), Vector3::Backward()));
    EXPECT_FLOAT_EQ(180.0, Trio::dot_angle(Vector3::Forward(), Vector3::Normalized(
            Vector3(0, 0, 1 + 1.0e-19)), Vector3::Normalized(
            Vector3(0, 0, 1 - 1.0e-19))));
    EXPECT_FLOAT_EQ(90.0, Trio::dot_angle(Vector3::Forward(), Vector3::Backward(), Vector3::Up()));
}