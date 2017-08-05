/*
 * @file: test-trio.cpp
 *
 * @brief: Source file for the TestTrio class, as well as the main function to run the tests.
 */

#include "test-trio.h"

/*
 * Check that the side lengths generated for a planar triangle are correct. Answers checked with
 * WolframAlpha.
 */
void TestTrio::test_planar_length_computation() {
    Trio a(Star(1, 1, 1), Star(5, 2, 0), Star(-1, -7, 5));
    std::array<double, 3> b = a.planar_lengths();

    assert_equal(b[0], 4.24264, "PlanarLengthComputationAB", 0.0001);
    assert_equal(b[1], 11.9164, "PlanarLengthComputationBC", 0.001);
    assert_equal(b[2], 9.16515, "PlanarLengthComputationCA", 0.0001);
}

/*
 * Check that the side lengths generated for a spherical triangle are correct.
 */
void TestTrio::test_spherical_length_computation() {
    Trio a(Star(1, 1, 1), Star(1, -1, 1), Star(-1, -1, 5));
    std::array<double, 3> b = a.spherical_lengths();

    assert_equal(b[0], Star::angle_between(a.b_1, a.b_2), "SphericalLengthComputationAB");
    assert_equal(b[1], Star::angle_between(a.b_2, a.b_3), "SphericalLengthComputationBC");
    assert_equal(b[2], Star::angle_between(a.b_3, a.b_1), "SphericalLengthComputationCA");
}

/*
 * Check that the semi perimeter is correctly computed. The should be half the triangle's
 * perimeter. Answers checked with WolframAlpha.
 */
void TestTrio::test_semi_perimeter_computation() {
    Trio a(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    std::array<double, 3> b = a.planar_lengths();

    assert_equal(a.semi_perimeter(b[0], b[1], b[2]), 13.1448 / 2.0,
                 "SemiPerimeterComputationPlanar", 0.0001);
}

/*
 * Check the planar_area method. Testing involves using the approach found in the link below to
 * verify that both formulas return a close answer:
 * https://www.algebra.com/algebra/homework/Vectors/Vectors.faq.question.674684.html
 */
void TestTrio::test_planar_area_computation() {
    Trio a(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    Star b = Star::cross(a.b_1 - a.b_2, a.b_1 - a.b_3);

    assert_equal(b.norm() * 0.5, Trio::planar_area(a.b_1, a.b_2, a.b_3),
                 "PlanarAreaComputation");
}

/*
 * Check the planar_moment method. According to the website below, the polar moment for an
 * equilateral triangle is 0.036s^4, where s=triangle side length. This test verifies this.
 * http://www.engineersedge.com/polar-moment-inertia.htm
 */
void TestTrio::test_planar_moment_computation() {
    Trio a(Star(1, 0, 0), Star(0, 1, 0), Star(0, 0, 1));
    std::array<double, 3> kaph_tau = a.planar_lengths();
    double b = 0.036 * pow(kaph_tau[0], 4);

    assert_equal(b, Trio::planar_moment(a.b_1, a.b_2, a.b_3),
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
    Trio a(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    Star b(0.66666666666666666, 1.6666666666666666, 1.0), c = a.planar_centroid();

    assert_equal(b, c, "PlanarCentroidComputation", b.str() + "," + c.str());
}

/*
 * Check the cut_triangle method. Sum of partitions should add up to the original.
 */
void TestTrio::test_cut_triangle_computation() {
    Trio a(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    Trio yodh_a = Trio::cut_triangle(a.b_1, a.b_2, a.b_3, a.b_1);
    Trio yodh_b = Trio::cut_triangle(a.b_1, a.b_2, a.b_3, a.b_2);
    Trio yodh_c = Trio::cut_triangle(a.b_1, a.b_2, a.b_3, a.b_3);
    Trio yodh_k = Trio::cut_triangle(a.b_1, a.b_2, a.b_3);

    double kaph_area = Trio::planar_area(a.b_1, a.b_2, a.b_3);
    double yodh_area = Trio::planar_area(yodh_a.b_1, yodh_a.b_2, yodh_a.b_3) +
                       Trio::planar_area(yodh_b.b_1, yodh_b.b_2, yodh_b.b_3) +
                       Trio::planar_area(yodh_c.b_1, yodh_c.b_2, yodh_c.b_3) +
                       Trio::planar_area(yodh_k.b_1, yodh_k.b_2, yodh_k.b_3);

    assert_equal(kaph_area, yodh_area, "CutTriangleSummation");
}

/*
 * Check the spherical_moment method. **no current method to verify this**
 */
void TestTrio::test_spherical_moment_computation() {
//    for (int i = 0; i < 10; i ++) {
//        Trio a(Star::chance(), Star::chance(), Star::chance());
//        double b = Trio::spherical_moment(a.b_1, a.b_2, a.b_3);
//        std::cout << "SphericalMomentComputation: " << b << std::endl;
//    }
}

/*
 * Enumerate all tests in TestTrio.
 *
 * @return -1 if the test case does not exist. 0 otherwise.
 */
int TestTrio::enumerate_tests(int test_case) {
    switch (test_case) {
        case 0: test_planar_length_computation();
            break;
        case 1: test_spherical_length_computation();
            break;
        case 2: test_semi_perimeter_computation();
            break;
        case 3: test_planar_area_computation();
            break;
        case 4: test_planar_moment_computation();
            break;
        case 5: test_spherical_area_computation();
            break;
        case 6: test_planar_centroid_computation();
            break;
        case 7: test_cut_triangle_computation();
            break;
        case 8: test_spherical_moment_computation();
            break;
        default:return -1;
    }

    return 0;
}

/*
 * Run the tests in TestTrio. Current set to print and log all data.
 */
int main() {
    return TestTrio().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
