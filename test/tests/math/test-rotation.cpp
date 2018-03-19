/// @file test-rotation.cpp
/// @author Glenn Galvizo
///
/// Source file for all Rotation class unit tests.

#define ENABLE_TESTING_ACCESS

#define _USE_MATH_DEFINES
#include <cmath>
#include "gtest/gtest.h"

#include "math/rotation.h"

/// Check that the string returned by the stream method is correct.
TEST(Rotation, OperatorStream) {
    std::stringstream s;
    s << Rotation(8, 1, 1, 1);
    EXPECT_EQ(s.str(), "(8.0000000000000000:1.0000000000000000:1.0000000000000000:1.0000000000000000)");
}

/// Check that the property v * <w, i, j, k> = v * <-w, -i, -j, -k> holds.
TEST(Rotation, QuaternionDoubleCoverProperty) {
    Rotation a = Rotation::chance();
    Rotation b = Rotation::wrap(-a);
    Star c = Star::chance();
    Star d = Rotation::rotate(c, a), e = Rotation::rotate(c, b);
    EXPECT_EQ(d, e);
}

/// Check that the resultant rotation is always normalized.
TEST(Rotation, QuaternionUnitProperty) {
    Rotation a = Rotation::chance();
    double b = Quaternion::Norm(a);
    EXPECT_DOUBLE_EQ(b, 1);
}

/// Check that rotation with the identity matrix yields the same vector.
TEST(Rotation, Identity) {
    Star a = Star::chance();
    Star b = Rotation::rotate(a, Rotation::identity());
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(a, b), 0.00000000001);
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

/// Check that the SLERP method moves toward a defined star (more of a logic check for me).
TEST(Rotation, Slerp) {
    for (unsigned int i = 0; i < 20; i++) {
        Star a = Star::chance(), b = Star::chance();
        double theta = Vector3::Angle(a, b);
        
        EXPECT_EQ(theta, Vector3::Angle(b, Rotation::slerp(a, b, 0)));
        EXPECT_EQ(a, Rotation::slerp(a, b, 0));
        
        EXPECT_GT(theta, Vector3::Angle(b, Rotation::slerp(a, b, 0.1)));
        EXPECT_LT(theta, Vector3::Angle(b, Rotation::slerp(a, b, -0.1)));
        EXPECT_GT(Vector3::Angle(b, Rotation::slerp(a, b, 0.1)), Vector3::Angle(b, Rotation::slerp(a, b, 0.2)));
    }
}

/// Check that the shake method doesn't shake with the deviation is 0, and returns a unique star when the deviation
/// is non-zero.
TEST(Rotation, Shake) {
    Star a = Star::chance(), b = Rotation::shake(a, 0);
    Star c = Rotation::shake(a, 30);
    EXPECT_EQ(a, b);
    EXPECT_FALSE(a == c);
    EXPECT_GT((180.0 / M_PI) * Vector3::Angle(a, c), 1.0);
}

/// Check that the random rotations are unique, and are normalized.
TEST(Rotation, Chance) {
    EXPECT_NE(Rotation::chance(), Rotation::chance());
    EXPECT_EQ(Quaternion::Norm(Rotation::chance()), 1.0);
}

/// Check the TRIAD method rotates both star pairs across frames correctly with the simple case of axis vectors.
TEST(Rotation, TRIADSimple) {
    Star::list a = {Star(1, 0, 0), Star(0, 1, 0)};
    Star::list b = {Star(0, 0, 1), Star(0, 1, 0)};
    Rotation c = Rotation::triad(a, b);
    Star d = Rotation::rotate(b[0], c), e = Rotation::rotate(b[1], c);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(d, a[0]), 0.000000001);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(e, a[1]), 0.000000001);
}

/// Check the TRIAD method rotates both star pairs across frames correctly with random vectors.
TEST(Rotation, TRIADChance) {
    Rotation a = Rotation::chance();
    Star::list b = {Star::chance(), Star::chance()};
    Star::list c = {Rotation::rotate(b[0], a), Rotation::rotate(b[1], a)};
    Rotation d = Rotation::triad(b, c);
    Star e = Rotation::rotate(c[0], d), f = Rotation::rotate(c[1], d);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(e, b[0]), 0.000001);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(f, b[1]), 0.000001);
}

/// Check that for each star in set A and the same rotated set B, there exists a quaternion H such that A = HB.
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
        EXPECT_LT((180.0 / M_PI) * Vector3::Angle(e, b[q]), 0.000001);
    }
}

/// Check the SVD method rotates both star pairs across frames correctly with the simple case of axis vectors.
TEST(Rotation, SVDSimple) {
    Star::list a = {Star(1, 0, 0), Star(0, 1, 0)};
    Star::list b = {Star(0, 0, 1), Star(0, 1, 0)};
    Rotation c = Rotation::svd(a, b);
    Star d = Rotation::rotate(b[0], c), e = Rotation::rotate(b[1], c);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(d, a[0]), 0.000000001);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(e, a[1]), 0.000000001);
}

/// Check the SVD method rotates both star pairs across frames correctly with random vectors.
TEST(Rotation, SVDChance) {
    Rotation a = Rotation::chance();
    Star::list b = {Star::chance(), Star::chance(), Star::chance()};
    Star::list c = {Rotation::rotate(b[0], a), Rotation::rotate(b[1], a), Rotation::rotate(b[2], a)};
    Rotation d = Rotation::svd(b, c);
    Star e = Rotation::rotate(c[0], d), f = Rotation::rotate(c[1], d);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(e, b[0]), 0.000001);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(f, b[1]), 0.000001);
}

/// Check that for each star in set A and the same rotated set B, there exists a quaternion H such that A = HB.
TEST(Rotation, SVDMultipleStars) {
    Rotation a = Rotation::chance();
    std::vector<Star> b, c;
    b.reserve(5), c.reserve(5);
    
    for (int q = 0; q < 5; q++) {
        b.push_back(Star::chance());
        c.push_back(Rotation::rotate(b[q], a));
    }
    Rotation d = Rotation::svd({b[0], b[1], b[2], b[3]}, {c[0], c[1], c[2], c[3]});
    
    for (int q = 0; q < 5; q++) {
        Star e = Rotation::rotate(c[q], d);
        EXPECT_LT((180.0 / M_PI) * Vector3::Angle(e, b[q]), 0.000002);
    }
}

/// Check the Q method rotates both star pairs across frames correctly with the simple case of axis vectors.
TEST(Rotation, QSimple) {
    Star::list a = {Star(1, 0, 0), Star(0, 1, 0)};
    Star::list b = {Star(0, 0, 1), Star(0, 1, 0)};
    Rotation c = Rotation::q_method(a, b);
    Star d = Rotation::rotate(b[0], c), e = Rotation::rotate(b[1], c);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(d, a[0]), 0.000000001);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(e, a[1]), 0.000000001);
}

/// Check the Q method rotates both star pairs across frames correctly with random vectors.
TEST(Rotation, QChance) {
    Rotation a = Rotation::chance();
    Star::list b = {Star::chance(), Star::chance(), Star::chance()};
    Star::list c = {Rotation::rotate(b[0], a), Rotation::rotate(b[1], a), Rotation::rotate(b[2], a)};
    Rotation d = Rotation::q_method(b, c);
    Star e = Rotation::rotate(c[0], d), f = Rotation::rotate(c[1], d);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(e, b[0]), 0.00001);
    EXPECT_LT((180.0 / M_PI) * Vector3::Angle(f, b[1]), 0.00001);
}

/// Check that for each star in set A and the same rotated set B, there exists a quaternion H such that A = HB.
TEST(Rotation, QMultipleStars) {
    Rotation a = Rotation::chance();
    std::vector<Star> b, c;
    b.reserve(5), c.reserve(5);
    
    for (int q = 0; q < 5; q++) {
        b.push_back(Star::chance());
        c.push_back(Rotation::rotate(b[q], a));
    }
    Rotation d = Rotation::q_method({b[0], b[1]}, {c[0], c[1]});
    
    for (int q = 0; q < 5; q++) {
        Star e = Rotation::rotate(c[q], d);
        EXPECT_LT((180.0 / M_PI) * Vector3::Angle(e, b[q]), 0.00001);
    }
}
