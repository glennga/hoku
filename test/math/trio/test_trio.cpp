/*
 * @file: test_trio.cpp
 *
 * @brief: Source file for the TestTrio class, as well as the main function to run the tests.
 */

#include "test_trio.h"

/*
 * Check that the side lengths generated for a planar triangle are correct. Answers checked with
 * WolframAlpha.
 */
void TestTrio::test_planar_length_computation() {
    Trio kaph(Star(1, 1, 1), Star(5, 2, 0), Star(-1, -7, 5));
    std::array<double, 3> yodh = kaph.planar_lengths();

    assert_equal(yodh[0], 4.24264, "PlanarLengthComputationAB", 0.0001);
    assert_equal(yodh[1], 11.9164, "PlanarLengthComputationBC", 0.001);
    assert_equal(yodh[2], 9.16515, "PlanarLengthComputationCA", 0.0001);
}

/*
 * Check that the side lengths generated for a spherical triangle are correct.
 */
void TestTrio::test_spherical_length_computation() {
    Trio kaph(Star(1, 1, 1), Star(1, -1, 1), Star(-1, -1, 5));
    std::array<double, 3> yodh = kaph.spherical_lengths();

    assert_equal(yodh[0], Star::angle_between(kaph.a, kaph.b), "SphericalLengthComputationAB");
    assert_equal(yodh[1], Star::angle_between(kaph.b, kaph.c), "SphericalLengthComputationBC");
    assert_equal(yodh[2], Star::angle_between(kaph.c, kaph.a), "SphericalLengthComputationCA");
}

/*
 * Check that the semi perimeter is correctly computed. The should be half the triangle's
 * perimeter. Answers checked with WolframAlpha.
 */
void TestTrio::test_semi_perimeter_computation() {
    Trio kaph(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    std::array<double, 3> yodh = kaph.planar_lengths();

    assert_equal(kaph.semi_perimeter(yodh[0], yodh[1], yodh[2]), 13.1448 / 2.0,
                 "SemiPerimeterComputationPlanar", 0.0001);
}

/*
 * Check the planar_area method. Testing involves using the approach found in the link below to
 * verify that both formulas return a close answer:
 * https://www.algebra.com/algebra/homework/Vectors/Vectors.faq.question.674684.html
 */
void TestTrio::test_planar_area_computation() {
    Trio kaph(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    Star yodh = Star::cross(kaph.a - kaph.b, kaph.a - kaph.c);

    assert_equal(yodh.norm() * 0.5, Trio::planar_area(kaph.a, kaph.b, kaph.c),
                 "PlanarAreaComputation");
}

/*
 * Check the planar_moment method. According to the website below, the polar moment for an
 * equilateral triangle is 0.036s^4, where s=triangle side length. This test verifies this.
 * http://www.engineersedge.com/polar-moment-inertia.htm
 */
void TestTrio::test_planar_moment_computation() {
    Trio kaph(Star(1, 0, 0), Star(0, 1, 0), Star(0, 0, 1));
    std::array<double, 3> kaph_tau = kaph.planar_lengths();
    double yodh = 0.036 * pow(kaph_tau[0], 4);

    assert_equal(yodh, Trio::planar_moment(kaph.a, kaph.b, kaph.c),
                 "PlanarMomentIdentityComputation", 0.001);
}

/*
 * Check the spherical_area method. **no current method to verify this**
 */
void TestTrio::test_spherical_area_computation() {

}


/*
 * Check the planar_centroid method. Answers checked with WolframAlpha.
 */
void TestTrio::test_planar_centroid_computation() {
    Trio kaph(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    Star yodh(0.666666667, 1.666666667, 1.0);

    assert_true(Star::is_equal(yodh, kaph.planar_centroid(), 0.00000001),
                "PlanarCentroidComputation");
}

/*
 * Check the cut_triangle method. Sum of partitions should add up to the original.
 */
void TestTrio::test_cut_triangle_computation() {
    Trio kaph(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    Trio yodh_a = Trio::cut_triangle(kaph.a, kaph.b, kaph.c, kaph.a);
    Trio yodh_b = Trio::cut_triangle(kaph.a, kaph.b, kaph.c, kaph.b);
    Trio yodh_c = Trio::cut_triangle(kaph.a, kaph.b, kaph.c, kaph.c);
    Trio yodh_k = Trio::cut_triangle(kaph.a, kaph.b, kaph.c);

    double kaph_area = Trio::planar_area(kaph.a, kaph.b, kaph.c);
    double yodh_area = Trio::planar_area(yodh_a.a, yodh_a.b, yodh_a.c) +
                       Trio::planar_area(yodh_b.a, yodh_b.b, yodh_b.c) +
                       Trio::planar_area(yodh_c.a, yodh_c.b, yodh_c.c) +
                       Trio::planar_area(yodh_k.a, yodh_k.b, yodh_k.c);

    assert_equal(kaph_area, yodh_area, "CutTriangleSummation");
}

/*
 * Check the spherical_moment method. **no current method to verify this**
 */
void TestTrio::test_spherical_moment_computation() {
//    for (int i = 0; i < 10; i ++) {
//        Trio kaph(Star::chance(), Star::chance(), Star::chance());
//        double yodh = Trio::spherical_moment(kaph.a, kaph.b, kaph.c);
//        std::cout << "SphericalMomentComputation: " << yodh << std::endl;
//    }
}

/*
 * Enumerate all tests in TestTrio.
 *
 * @return -1 if the test case does not exist. 0 otherwise.
 */
int TestTrio::enumerate_tests(int test_case) {
    switch (test_case) {
        case 0:
            test_planar_length_computation();
            break;
        case 1:
            test_spherical_length_computation();
            break;
        case 2:
            test_semi_perimeter_computation();
            break;
        case 3:
            test_planar_area_computation();
            break;
        case 4:
            test_planar_moment_computation();
            break;
        case 5:
            test_spherical_area_computation();
            break;
        case 6:
            test_planar_centroid_computation();
            break;
        case 7:
            test_cut_triangle_computation();
            break;
        case 8:
            test_spherical_moment_computation();
            break;
        default:
            return -1;
    }

    return 0;
}

/*
 * Run the tests in TestTrio.
 */
int main() {
    return TestTrio().execute_tests();
}
