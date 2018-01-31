/// @file test-rotation.cpp
/// @author Glenn Galvizo
///
/// Source file for all Rotation class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "math/rotation.h"
#include "gtest/gtest.h"

/// Check that the vector component is set to zero with the public constructor.
TEST(RotationConstructor, Public) {
    Rotation a;
    Star b(a.i, a.j, a.j);
    EXPECT_EQ(b, Star::zero());
}

/// Check that the vector component is correctly set with the private constructor.
TEST(RotationConstructor, PrivateRowSet) {
    Star a(1, 4, 5);
    Rotation b(2, Star(1, 4, 5));
    Star c = Star(b.i, b.j, b.k);
    EXPECT_EQ(c, a);
}

/// Check that the individual components are correctly set with the private constructor.
TEST(RotationConstructor, PrivateComponentSet) {
    Rotation a(2, Star(1, 4, 5));
    EXPECT_DOUBLE_EQ(2, a.w);
    EXPECT_DOUBLE_EQ(1, a.i);
    EXPECT_DOUBLE_EQ(4, a.j);
    EXPECT_DOUBLE_EQ(5, a.k);
}

/// Check that the property v * <w, i, j, k> = v * <-w, -i, -j, -k> holds.
TEST(RotationQuaternion, DoubleCoverProperty) {
    Rotation a = Rotation::chance();
    Rotation b(-a.w, Star(-a.i, -a.j, -a.k));
    Star c = Star::chance();
    Star d = Rotation::rotate(c, a), e = Rotation::rotate(c, b);
    EXPECT_EQ(d, e);
}

/// Check that the resultant rotation is always normalized.
TEST(RotationQuaternion, UnitProperty) {
    Rotation a = Rotation::chance();
    double b = sqrt(a.w * a.w + a.i * a.i + a.j * a.j + a.k * a.k);
    EXPECT_DOUBLE_EQ(b, 1);
}

/// Check that the matrix is correctly converted into a quaternion.
TEST(RotationQuaternion, MatrixToQuarternion) {
    auto a = Rotation::matrix_to_quaternion({Star(1, 0, 0), Star(0, 1, 0), Star(0, 0, 1)});
    EXPECT_EQ(a.w, 1);
    EXPECT_EQ(a.i, 0);
    EXPECT_EQ(a.j, 0);
    EXPECT_EQ(a.k, 0);
}

/// Check that rotation with the identity matrix yields the same vector.
TEST(RotationIdentity, Identity) {
    Star a = Star::chance();
    Star b = Rotation::rotate(a, Rotation::identity());
    EXPECT_EQ(a, b);
}

/// Check that the matrix multiplication logic used is correct. Answers checked with WolframAlpha.
TEST(RotationMatrix, Transpose) {
    std::array<Star, 3> a = {Star(1, 2, 3), Star(4, 5, 6), Star(7, 8, 9)};
    std::array<Star, 3> b = {Star(10, 11, 12), Star(13, 14, 15), Star(16, 17, 18)};
    std::array<Star, 3> c = Rotation::matrix_multiply_transpose(a, b);
    Star d(68, 86, 104), e(167, 212, 257), f(266, 338, 410);
    EXPECT_EQ(c[0], d);
    EXPECT_EQ(c[1], e);
    EXPECT_EQ(c[2], f);
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
TEST(RotationLogic, Rotate) {
    Rotation a(0.78742389255495682, Star(-0.36903856465565266, 0.42001639743793967, -0.25953877766867561));
    Star b(-0.051796588649074424, -0.69343284143642703, -0.71865708639219672);
    Star c(-0.7080355444092732, -0.6348947648122054, 0.30918328781989235);
    Star d = Rotation::rotate(b, a);
    EXPECT_EQ(d, c);
}

/// Check the TRIAD property that the resultant quaternion rotates both star pairs across frames correctly with the
/// simple case of axis vectors.
TEST(RotationTRIAD, Simple) {
    Star::list a = {Star(1, 0, 0), Star(0, 1, 0)};
    Star::list b = {Star(0, 0, 1), Star(0, 1, 0)};
    Rotation c = Rotation::triad(a, b);
    Star d = Rotation::rotate(b[0], c), e = Rotation::rotate(b[1], c);
    EXPECT_TRUE(Star::is_equal(d, a[0], 0.0000000001));
    EXPECT_TRUE(Star::is_equal(e, a[1], 0.0000000001));
}

/// Check the TRIAD property that the resultant quaternion rotates both star pairs across frames correctly with
/// random vectors.
TEST(RotationTRIAD, Chance) {
    Rotation a = Rotation::chance();
    Star::list b = {Star::chance(), Star::chance()};
    Star::list c = {Rotation::rotate(b[0], a), Rotation::rotate(b[1], a)};
    Rotation d = Rotation::triad(b, c);
    Star e = Rotation::rotate(c[0], d), f = Rotation::rotate(c[1], d);
    EXPECT_TRUE(Star::is_equal(e, b[0], 0.0000000001));
    EXPECT_TRUE(Star::is_equal(f, b[1], 0.0000000001));
}

/// Check that for each star in set A and the same rotated set B, there exists a quaternion H such that A = HB.
TEST(RotationTRIAD, MultipleStars) {
    Rotation a = Rotation::chance(), d;
    std::vector<Star> b, c;
    b.reserve(5), c.reserve(5);
    
    for (int q = 0; q < 5; q++) {
        b.push_back(Star::chance());
        c.push_back(Rotation::rotate(b[q], a));
    }
    d = Rotation::triad({b[0], b[1]}, {c[0], c[1]});
    
    for (int q = 0; q < 5; q++) {
        Star e = Rotation::rotate(c[q], d);
        EXPECT_TRUE(Star::is_equal(e, b[q], 0.0000000001));
    }
}

/// Check that the shake method doesn't shake with the deviation is 0, and returns a unique star when the deviation
/// is non-zero.
TEST(RotationChance, Shake) {
    Star a = Star::chance(), b = Rotation::shake(a, 0);
    Star c = Rotation::shake(a, 30);
    EXPECT_EQ(a, b);
    EXPECT_FALSE(a == c);
    EXPECT_GT(Star::angle_between(a, c), 1.0);
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