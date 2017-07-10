/*
 * @file: test-rotation.cpp
 *
 * @brief: Source file for the TestRotation class, as well as the main function to run the tests.
 */

#include "test-rotation.h"

/*
 * Check that the vector component is set to zero with the public constructor.
 */
void TestRotation::test_public_constructor() {
    Rotation kaph;

    assert_true(kaph.gamma == Star(0, 0, 0), "PublicConstructorGamma");
}

/*
 * Check that the vector component is correctly set with the private constructor.
 */
void TestRotation::test_private_constructor_row_set() {
    Rotation kaph(2, Star(1, 4, 5));

    assert_true(kaph.gamma == Star(1, 4, 5), "PrivateConstructorSetGamma");
}

/*
 * Check that the individual components are correctly set with the private constructor.
 */
void TestRotation::test_private_constructor_component_set() {
    Rotation kaph(2, Star(1, 4, 5));

    assert_equal(2, kaph.w, "PrivateConstructorComponentSetW");
    assert_equal(1, kaph.x, "PrivateConstructorComponentSetX");
    assert_equal(4, kaph.y, "PrivateConstructorComponentSetY");
    assert_equal(5, kaph.z, "PrivateConstructorComponentSetZ");
}

/*
 * Check that the property v * <w, x, y, z> = v * <-w, -x, -y, -z> holds.
 */
void TestRotation::test_quaternion_double_cover_property() {
    Rotation kaph = Rotation::chance();
    Rotation yodh(-kaph.w, Star(-kaph.x, -kaph.y, -kaph.z));
    Star teth = Star::chance();

    assert_true(Rotation::rotate(teth, kaph) == Rotation::rotate(teth, yodh),
                "QuaternionDoubleCoverProperty");
}

/*
 * Check that the resultant rotation is always normalized.
 */
void TestRotation::test_quaternion_unit_property() {
    Rotation kaph = Rotation::chance();
    double yodh = sqrt(kaph.w * kaph.w + kaph.x * kaph.x + kaph.y * kaph.y + kaph.z * kaph.z);

    assert_equal(yodh, 1, "QuaternionUnitProperty");
}

/*
 * Check that the matrix is correctly converted into a quaternion.
 */
void TestRotation::test_matrix_to_quaternion() {
    auto kaph = Rotation::matrix_to_quaternion({Star(1, 0, 0), Star(0, 1, 0), Star(0, 0, 1)});

    assert_equal(kaph.w, 1, "IdentityMatrixToQuaternionW");
    assert_equal(kaph.x, 0, "IdentityMatrixToQuaternionX");
    assert_equal(kaph.y, 0, "IdentityMatrixToQuaternionY");
    assert_equal(kaph.z, 0, "IdentityMatrixToQuaternionZ");
}

/*
 * Check that rotation with the identity matrix yields the same vector.
 */
void TestRotation::test_rotation_identity() {
    Star kaph = Star::chance();
    Star yodh = Rotation::rotate(kaph, Rotation::identity());

    assert_true(kaph == yodh, "RotationIdentity");
}

/*
 * Check that the matrix multiplication logic used is correct. Answers checked with WolframAlpha.
 */
void TestRotation::test_matrix_multiplication_transpose() {
    std::array<Star, 3> kaph = {Star(1, 2, 3), Star(4, 5, 6), Star(7, 8, 9)};
    std::array<Star, 3> yodh = {Star(10, 11, 12), Star(13, 14, 15), Star(16, 17, 18)};
    std::array<Star, 3> teth = Rotation::matrix_multiply_transpose(kaph, yodh);

    assert_true(teth[0] == Star(68, 86, 104), "MatrixMultiplicationTransposeRow1");
    assert_true(teth[1] == Star(167, 212, 257), "MatrixMultiplicationTransposeRow2");
    assert_true(teth[2] == Star(266, 338, 410), "MatrixMultiplicationTransposeRow3");
}

/*
 * Check that a star rotated yields the correct results. Answers checked with quaternion
 * calculator here: http://www.bluetulip.org/2014/programs/quaternions.html
 *
 * R = [w, x, y, z]
 * P = [0, p1, p2, p2]        -------> P' = H(H(R, P), R')
 * R' = [w, -x, -y, -z]
 *
 * Notes:
 * 0.08561884343100587+-0.5226063439365456i+-0.7977945435657172j+-0.2882288833263985k
 * -2.7755575615628914e-17+-0.7080355444092732i+-0.6348947648122054j+0.30918328781989235k
 *
 * Using equations found here: https://math.stackexchange.com/a/535223
 */
void TestRotation::test_rotate_logic() {
    Rotation kaph(0.78742389255495682, Star(-0.36903856465565266, 0.42001639743793967,
                                            -0.25953877766867561));
    Star yodh = Star(-0.051796588649074424, -0.69343284143642703, -0.71865708639219672);
    Star heth = Rotation::rotate(yodh, kaph);

    assert_true(heth == Star(-0.7080355444092732, -0.6348947648122054, 0.30918328781989235),
                "RotatedStarLogicCheck");
}

/*
 * Check the TRIAD property that the resultant quaternion rotates both star pairs across frames
 * correctly with the simple case of axis vectors.
 */
void TestRotation::test_triad_property_simple() {
    std::array<Star, 2> kaph = {Star(1, 0, 0), Star(0, 1, 0)};
    std::array<Star, 2> yodh = {Star(0, 0, 1), Star(0, 1, 0)};
    Rotation teth = Rotation::rotation_across_frames(kaph, yodh);

    assert_true(Rotation::rotate(yodh[0], teth) == kaph[0], "TriadPropertyUsingAxisVectors0");
    assert_true(Rotation::rotate(yodh[1], teth) == kaph[1], "TriadPropertyUsingAxisVectors1");
}

/*
 * Check the TRIAD property that the resultant quaternion rotates both star pairs across frames
 * correctly with random vectors.
 */
void TestRotation::test_triad_property_random() {
    Rotation kaph = Rotation::chance();
    std::array<Star, 2> yodh = {Star::chance(), Star::chance()};
    std::array<Star, 2> teth = {Rotation::rotate(yodh[0], kaph), Rotation::rotate(yodh[1], kaph)};
    Rotation heth = Rotation::rotation_across_frames(yodh, teth);

    assert_true(Rotation::rotate(teth[0], heth) == yodh[0], "TriadPropertyUsingChanceVectors0");
    assert_true(Rotation::rotate(teth[1], heth) == yodh[1], "TriadPropertyUsingChanceVectors1");
}

/*
 * Check that for each star in set A and the same rotated set B, there exists a quaternion H
 * such that A = HB.
 */
void TestRotation::test_triad_multiple_stars() {
    Rotation kaph = Rotation::chance(), heth;
    std::vector<Star> yodh, teth;
    yodh.reserve(5);
    teth.reserve(5);

    for (int a = 0; a < 5; a++) {
        yodh.push_back(Star::chance());
        teth.push_back(Rotation::rotate(yodh[a], kaph));
    }
    heth = Rotation::rotation_across_frames({yodh[0], yodh[1]}, {teth[0], teth[1]});

    for (int a = 0; a < 5; a++) {
        std::string test_name = "TriadPropertyStarSetStar" + std::to_string(a + 1);
        assert_true(Rotation::rotate(teth[a], heth) == yodh[a], test_name);
    }
}

/*
 * Enumerate all tests in TestRotation.
 *
 * @return -1 if the test case does not exist. 0 otherwise.
 */
int TestRotation::enumerate_tests(int test_case) {
    switch (test_case) {
        case 0: test_public_constructor();
            break;
        case 1: test_private_constructor_row_set();
            break;
        case 2: test_private_constructor_component_set();
            break;
        case 3: test_quaternion_double_cover_property();
            break;
        case 4: test_quaternion_unit_property();
            break;
        case 5: test_matrix_to_quaternion();
            break;
        case 6: test_matrix_multiplication_transpose();
            break;
        case 7: test_rotation_identity();
            break;
        case 8: test_rotate_logic();
            break;
        case 9: test_triad_property_simple();
            break;
        case 10: test_triad_property_random();
            break;
        case 11: test_triad_multiple_stars();
            break;
        default:return -1;
    }

    return 0;
}

/*
 * Run the tests in TestRotation.
 */
int main() {
    return TestRotation().execute_tests();
}
