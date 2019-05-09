/// @file test-rotation.cpp
/// @author Glenn Galvizo
///
/// Source file for all Rotation class unit tests.

#define _USE_MATH_DEFINES

#include <cmath>
#include "gtest/gtest.h"

#include "math/rotation.h"

TEST(Rotation, OperatorStream) {
    std::stringstream s;
    s << Rotation(8, 1, 1, 1);
    EXPECT_EQ(s.str(), "(8.0000000000000000:1.0000000000000000:1.0000000000000000:1.0000000000000000)");
}
TEST(Rotation, QuaternionDoubleCoverProperty) {
    Rotation a = Rotation::chance();
    Rotation b = Rotation::wrap(-a);
    Star c = Star::chance();
    Star d = Rotation::rotate(c, a), e = Rotation::rotate(c, b);
    EXPECT_EQ(d, e);
}
TEST(Rotation, QuaternionUnitProperty) {
    Rotation a = Rotation::chance();
    double b = Quaternion::Norm(a);
    EXPECT_DOUBLE_EQ(b, 1);
}
TEST(Rotation, Identity) {
    Star a = Star::chance();
    Star b = Rotation::rotate(a, Rotation::identity());
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(a.get_vector(), b.get_vector()), 0.00000000001);
}
/// Check that a star rotated yields the correct results. Answers checked with quaternion calculator here:
/// http://www.bluetulip.org/2014/programs/quaternions.html
///
/// R = [w, x, y, z] <br>
/// P = [0, p1, p2, p2] ==============> P' = H(H(R, P), R') <br>
/// R' = [w, -x, -y, -z] <br>
///
/// Notes: 0.08561884343100587+-0.5226063439365456i+-0.7977945435657172j+-0.2882288833263985k
///        -2.7755575615628914e-17+-0.7080355444092732i+-0.6348947648122054j+0.30918328781989235k
///
/// Using equations found here: https://math.stackexchange.com/a/535223
TEST(Rotation, LogicRotate) { 
    Quaternion a(Vector3(-0.36903856465565266, 0.42001639743793967, -0.25953877766867561), 0.78742389255495682);
    Star b(-0.051796588649074424, -0.69343284143642703, -0.71865708639219672);
    Star c(-0.7080355444092732, -0.6348947648122054, 0.30918328781989235);
    Star d = Rotation::rotate(b, Rotation::wrap(a));
    EXPECT_EQ(d, c);
}
TEST(Rotation, Slerp) {
    for (unsigned int i = 0; i < 20; i++) {
        Star a = Star::chance(), b = Star::chance();
        double theta = Vector3::Angle(a.get_vector(), b.get_vector());

        EXPECT_EQ(theta, Vector3::Angle(b.get_vector(), Rotation::slerp(a, b, 0).get_vector()));
        EXPECT_EQ(a, Rotation::slerp(a, b, 0));

        EXPECT_GT(theta, Vector3::Angle(b.get_vector(), Rotation::slerp(a, b, 0.1).get_vector()));
        EXPECT_LT(theta, Vector3::Angle(b.get_vector(), Rotation::slerp(a, b, -0.1).get_vector()));
        EXPECT_GT(Vector3::Angle(b.get_vector(), Rotation::slerp(a, b, 0.1).get_vector()),
                  Vector3::Angle(b.get_vector(), Rotation::slerp(a, b, 0.2).get_vector()));
    }
}
TEST(Rotation, Shake) {
    Star a = Star::chance(), b = Rotation::shake(a, 0);
    Star c = Rotation::shake(a, 30);
    EXPECT_EQ(a, b);
    EXPECT_FALSE(a.get_vector() == c.get_vector());
    EXPECT_GT((180.0 / M_PI) * Vector3::Angle(a.get_vector(), c.get_vector()), 1.0);
}
TEST(Rotation, ShakeDeviation) {
    static auto sd = [] (const std::vector<double> &samples) -> double {
        int size = samples.size();
        double variance = 0, t = samples[0];

        for (int i = 1; i < size; i++) {
            t += samples[i];
            double diff = ((i + 1) * samples[i]) - t;
            variance += (diff * diff) / ((i + 1.0) * i);
        }

        return sqrt(variance / (size - 1));
    };

    Star a = Star::chance();
//    std::vector<double> sigma = {1.0e-13, 1.0e-12, 1.0e-11, 1.0e-9, 1.0e-7, 1.0e-5, 1.0e-3};
    std::vector<double> sigma = {1.0e-7, 1.0e-6, 1.0e-5, 0.1};
    for (const double &s : sigma) {
        std::vector<double> theta_std(10000);
        for (unsigned int i = 0; i < 10000; i++) {
            Star b = Rotation::shake(a, s);
            theta_std[i] = (180.0 / M_PI) * Vector3::Angle(a.get_vector(), b.get_vector());
        }

        EXPECT_NEAR(sd(theta_std), s, s);
    }
}
TEST(Rotation, Chance) {
    EXPECT_NE(Rotation::chance(), Rotation::chance());
    EXPECT_EQ(Quaternion::Norm(Rotation::chance()), 1.0);
}
TEST(Rotation, TRIADSimple) {
    Star::list a = {Star(1, 0, 0), Star(0, 1, 0)};
    Star::list b = {Star(0, 0, 1), Star(0, 1, 0)};
    Rotation c = Rotation::triad(a, b);
    Star d = Rotation::rotate(b[0], c), e = Rotation::rotate(b[1], c);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(d.get_vector(), a[0]), 0.000000001);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(e.get_vector(), a[1]), 0.000000001);
}
TEST(Rotation, TRIADChance) {
    Rotation a = Rotation::chance();
    Star::list b = {Star::chance(), Star::chance()};
    Star::list c = {Rotation::rotate(b[0], a), Rotation::rotate(b[1], a)};
    Rotation d = Rotation::triad(b, c);
    Star e = Rotation::rotate(c[0], d), f = Rotation::rotate(c[1], d);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(e.get_vector(), b[0]), 0.000001);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(f.get_vector(), b[1]), 0.000001);
}
TEST(Rotation, TRIADMultipleStars) {
    Rotation a = Rotation::chance();
    std::vector<Star> b, c;
    b.reserve(5), c.reserve(5);

    for (int q = 0; q < 5; q++) {
        b.push_back(Star::chance());
        c.push_back(Rotation::rotate(b[q], a));
    }
    Rotation d = Rotation::triad({b[0], b[1]}, {c[0], c[1]});

    for (int q = 0; q < 5; q++) {
        Star e = Rotation::rotate(c[q], d);
        EXPECT_LT((180.0 / M_PI) * Vector3::Angle(e.get_vector(), b[q]), 0.000001);
    }
}
