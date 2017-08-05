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
    Rotation a;
    Star b(a.i, a.j, a.j);

    assert_equal(b, Star(0, 0, 0), "PublicConstructorGamma", b.str() + "," + Star(0, 0, 0).str());
}

/*
 * Check that the vector component is correctly set with the private constructor.
 */
void TestRotation::test_private_constructor_row_set() {
    Star a(1, 4, 5);
    Rotation b(2, Star(1, 4, 5));
    Star c = Star(b.i, b.j, b.k);

    assert_equal(c, a, "PrivateConstructorSetGamma", c.str() + "," + a.str());
}

/*
 * Check that the individual components are correctly set with the private constructor.
 */
void TestRotation::test_private_constructor_component_set() {
    Rotation a(2, Star(1, 4, 5));

    assert_equal(2, a.w, "PrivateConstructorComponentSetW");
    assert_equal(1, a.i, "PrivateConstructorComponentSetX");
    assert_equal(4, a.j, "PrivateConstructorComponentSetY");
    assert_equal(5, a.k, "PrivateConstructorComponentSetZ");
}

/*
 * Check that the property v * <w, i, j, k> = v * <-w, -i, -j, -k> holds.
 */
void TestRotation::test_quaternion_double_cover_property() {
    Rotation a = Rotation::chance();
    Rotation b(-a.w, Star(-a.i, -a.j, -a.k));
    Star c = Star::chance();
    Star d = Rotation::rotate(c, a), e = Rotation::rotate(c, b);

    assert_equal(d, e, "QuaternionDoubleCoverProperty", d.str() + "," + e.str());
}

/*
 * Check that the resultant rotation is always normalized.
 */
void TestRotation::test_quaternion_unit_property() {
    Rotation a = Rotation::chance();
    double b = sqrt(a.w * a.w + a.i * a.i + a.j * a.j + a.k * a.k);

    assert_equal(b, 1, "QuaternionUnitProperty");
}

/*
 * Check that the matrix is correctly converted into a quaternion.
 */
void TestRotation::test_matrix_to_quaternion() {
    auto a = Rotation::matrix_to_quaternion({Star(1, 0, 0), Star(0, 1, 0), Star(0, 0, 1)});

    assert_equal(a.w, 1, "IdentityMatrixToQuaternionW");
    assert_equal(a.i, 0, "IdentityMatrixToQuaternionX");
    assert_equal(a.j, 0, "IdentityMatrixToQuaternionY");
    assert_equal(a.k, 0, "IdentityMatrixToQuaternionZ");
}

/*
 * Check that rotation with the identity matrix yields the same vector.
 */
void TestRotation::test_rotation_identity() {
    Star a = Star::chance();
    Star b = Rotation::rotate(a, Rotation::identity());

    assert_equal(a, b, "RotationIdentity", a.str() + "," + b.str());
}

/*
 * Check that the matrix multiplication logic used is correct. Answers checked with WolframAlpha.
 */
void TestRotation::test_matrix_multiplication_transpose() {
    std::array<Star, 3> a = {Star(1, 2, 3), Star(4, 5, 6), Star(7, 8, 9)};
    std::array<Star, 3> b = {Star(10, 11, 12), Star(13, 14, 15), Star(16, 17, 18)};
    std::array<Star, 3> c = Rotation::matrix_multiply_transpose(a, b);
    Star d(68, 86, 104), e(167, 212, 257), f(266, 338, 410);

    assert_equal(c[0], d, "MatrixMultiplicationTransposeRow1", c[0].str() + "," + d.str());
    assert_equal(c[1], e, "MatrixMultiplicationTransposeRow2", c[1].str() + "," + e.str());
    assert_equal(c[2], f, "MatrixMultiplicationTransposeRow3", c[2].str() + "," + f.str());
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
    Rotation a(0.78742389255495682, Star(-0.36903856465565266, 0.42001639743793967,
                                         -0.25953877766867561));
    Star b(-0.051796588649074424, -0.69343284143642703, -0.71865708639219672);
    Star c(-0.7080355444092732, -0.6348947648122054, 0.30918328781989235);
    Star d = Rotation::rotate(b, a);

    assert_equal(d, c, "RotatedStarLogicCheck", d.str() + "," + c.str());
}

/*
 * Check the TRIAD property that the resultant quaternion rotates both star pairs across frames
 * correctly with the simple case of axis vectors.
 */
void TestRotation::test_triad_property_simple() {
    std::array<Star, 2> a = {Star(1, 0, 0), Star(0, 1, 0)};
    std::array<Star, 2> b = {Star(0, 0, 1), Star(0, 1, 0)};
    Rotation c = Rotation::rotation_across_frames(a, b);
    Star d = Rotation::rotate(b[0], c), e = Rotation::rotate(b[1], c);

    assert_equal(d, a[0], "TriadPropertyUsingAxisVectors0", d.str() + "," + a[0].str());
    assert_equal(e, a[1], "TriadPropertyUsingAxisVectors1", e.str() + "," + a[1].str());
}

/*
 * Check the TRIAD property that the resultant quaternion rotates both star pairs across frames
 * correctly with random vectors.
 */
void TestRotation::test_triad_property_random() {
    Rotation a = Rotation::chance();
    std::array<Star, 2> b = {Star::chance(), Star::chance()};
    std::array<Star, 2> c = {Rotation::rotate(b[0], a), Rotation::rotate(b[1], a)};
    Rotation d = Rotation::rotation_across_frames(b, c);
    Star e = Rotation::rotate(c[0], d), f = Rotation::rotate(c[1], d);

    assert_equal(e, b[0], "TriadPropertyUsingChanceVectors0", e.str() + "," + b[0].str());
    assert_equal(f, b[1], "TriadPropertyUsingChanceVectors1", f.str() + "," + b[1].str());
}

/*
 * Check that for each star in set A and the same rotated set B, there exists a quaternion H
 * such that A = HB.
 */
void TestRotation::test_triad_multiple_stars() {
    Rotation a = Rotation::chance(), d;
    std::vector<Star> b, c;
    b.reserve(5);
    c.reserve(5);

    for (int q = 0; q < 5; q++) {
        b.push_back(Star::chance());
        c.push_back(Rotation::rotate(b[q], a));
    }
    d = Rotation::rotation_across_frames({b[0], b[1]}, {c[0], c[1]});

    for (int q = 0; q < 5; q++) {
        Star e = Rotation::rotate(c[q], d);
        std::string test_name = "TriadPropertyStarSetStar" + std::to_string(q + 1);
        assert_equal(e, b[q], test_name, e.str() + "," + b[q].str());
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
 * Run the tests in TestRotation. Currently set to print and log all results.
 */
int main() {
    return TestRotation().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
