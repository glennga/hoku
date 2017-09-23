/// @file test-trio.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestTrio class, as well as the main function to run the tests.

#include "math/test-trio.h"

/// Check that the side lengths generated for a planar triangle are correct. Answers checked with WolframAlpha.
///
/// @return 0 when finished.
int TestTrio::test_planar_length_computation () {
    std::array<double, 3> a = Trio(Star(1, 1, 1), Star(5, 2, 0), Star(-1, -7, 5)).planar_lengths();
    
    assert_equal(a[0], 4.24264, "PlanarLengthComputationAB", 0.0001);
    assert_equal(a[1], 11.9164, "PlanarLengthComputationBC", 0.001);
    return 0 * assert_equal(a[2], 9.16515, "PlanarLengthComputationCA", 0.0001);
}

/// Check that the side lengths generated for a spherical triangle are correct.
///
/// @return 0 when finished.
int TestTrio::test_spherical_length_computation () {
    Trio a(Star(1, 1, 1), Star(1, -1, 1), Star(-1, -1, 5));
    std::array<double, 3> b = a.spherical_lengths();
    auto compute_length = [] (const Star &beta_1, const Star &beta_2) -> double {
        return acos(Star::dot(beta_1, beta_2) / (beta_1.norm() * beta_2.norm()));
    };
    
    assert_equal(b[0], compute_length(a.b_1, a.b_2), "SphericalLengthComputationAB");
    assert_equal(b[1], compute_length(a.b_2, a.b_3), "SphericalLengthComputationBC");
    return 0 * assert_equal(b[2], compute_length(a.b_3, a.b_1), "SphericalLengthComputationCA");
}

/// Check that the semi perimeter is correctly computed. The should be half the triangle's perimeter. Answers checked
/// with WolframAlpha.
///
/// @return 0 when finished.
int TestTrio::test_semi_perimeter_computation () {
    Trio a(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    std::array<double, 3> b = a.planar_lengths();
    double c = 13.1448 / 2.0;
    
    return 0 * assert_equal(a.semi_perimeter(b[0], b[1], b[2]), c, "SemiPerimeterComputationPlanar", 0.0001);
}

/// Check the planar_area method. Testing involves using the approach found in the link below to verify that both
/// formulas return a close answer: https://www.algebra.com/algebra/homework/Vectors/Vectors.faq.question.674684.html
///
/// @return 0 when finished.
int TestTrio::test_planar_area_computation () {
    Trio a(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    Star b = Star::cross(a.b_1 - a.b_2, a.b_1 - a.b_3);
    
    return 0 * assert_equal(b.norm() * 0.5, Trio::planar_area(a.b_1, a.b_2, a.b_3), "PlanarAreaComputation");
}

/// Check the planar_moment method. According to the website below, the polar moment for an equilateral triangle is
/// 0.036s^4, where s=triangle side length. This test verifies this:
/// http://www.engineersedge.com/polar-moment-inertia.htm
///
/// @return 0 when finished.
int TestTrio::test_planar_moment_computation () {
    Trio a(Star(1, 0, 0), Star(0, 1, 0), Star(0, 0, 1));
    std::array<double, 3> kaph_tau = a.planar_lengths();
    double b = 0.036 * pow(kaph_tau[0], 4);
    
    return 0 * assert_equal(b, Trio::planar_moment(a.b_1, a.b_2, a.b_3), "PlanarMomentIdentityComputation", 0.001);
}

/// Check the spherical_area method. **no current method to verify this**
///
/// @return 0 when finished.
int TestTrio::test_spherical_area_computation () {
    return 0;
}

/// Check the planar_centroid method. Answers checked with WolframAlpha.
///
/// @return 0 when finished.
int TestTrio::test_planar_centroid_computation () {
    Trio a(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    Star b(0.66666666666666666, 1.6666666666666666, 1.0), c = a.planar_centroid();
    
    return 0 * assert_equal(b, c, "PlanarCentroidComputation", b.str() + "," + c.str());
}

/// Check the cut_triangle method. Sum of partitions should add up to the original.
///
/// @return 0 when finished.
int TestTrio::test_cut_triangle_computation () {
    Trio a(Star(1, 1, 1), Star(-1, 0, -1), Star(2, 4, 3));
    Trio b_a = Trio::cut_triangle(a.b_1, a.b_2, a.b_3, a.b_1);
    Trio b_b = Trio::cut_triangle(a.b_1, a.b_2, a.b_3, a.b_2);
    Trio b_c = Trio::cut_triangle(a.b_1, a.b_2, a.b_3, a.b_3);
    Trio b_k = Trio::cut_triangle(a.b_1, a.b_2, a.b_3);
    
    double kaph_area = Trio::planar_area(a.b_1, a.b_2, a.b_3);
    double yodh_area = Trio::planar_area(b_a.b_1, b_a.b_2, b_a.b_3) + Trio::planar_area(b_b.b_1, b_b.b_2, b_b.b_3)
                       + Trio::planar_area(b_c.b_1, b_c.b_2, b_c.b_3) + Trio::planar_area(b_k.b_1, b_k.b_2, b_k.b_3);
    
    return 0 * assert_equal(kaph_area, yodh_area, "CutTriangleSummation");
}

/// Check the spherical_moment method. **no current method to verify this**
///
/// @return 0 when finished.
int TestTrio::test_spherical_moment_computation () {
    //    for (int i = 0; i < 10; i ++)
    //    {
    //        Trio a(Star::chance(), Star::chance(), Star::chance());
    //        double b = Trio::spherical_moment(a.b_1, a.b_2, a.b_3);
    //        std::cout << "SphericalMomentComputation: " << b << std::endl;
    //    }
    return 0;
}

/// Enumerate all tests in TestTrio.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestTrio::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_planar_length_computation();
        case 1: return test_spherical_length_computation();
        case 2: return test_semi_perimeter_computation();
        case 3: return test_planar_area_computation();
        case 4: return test_planar_moment_computation();
        case 5: return test_spherical_area_computation();
        case 6: return test_planar_centroid_computation();
        case 7: return test_cut_triangle_computation();
        case 8: return test_spherical_moment_computation();
        default: return -1;
    }
}

/// Run the tests in TestTrio. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    return TestTrio().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
