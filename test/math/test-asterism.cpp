/// @file test-asterism.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestAsterism class, as well as the main function to run the tests.

#include "math/test-asterism.h"

/// Check that stars A, B, C, and D are found correctly.
///
/// @return 0 when finished.
int TestAsterism::test_abcd_star_find () {
    Asterism::stars m = {Star::chance(1), Star::chance(2), Star::chance(3), Star::chance(4)};
    Asterism::points n = {Mercator(m[0], 1), Mercator(m[1], 1), Mercator(m[2], 1), Mercator(m[3], 1)};
    Asterism p(m);
    double d_max = 0;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            double d = Mercator::distance_between(n[i], n[j]);
            d_max = (d > d_max) ? d : d_max;
        }
    }
    
    assert_equal(Mercator::distance_between(p.a, p.b), d_max, "StarsAandBFoundCorrectly");
    assert_outside(p.c.get_hr(), {p.a.get_hr(), p.b.get_hr()}, "CIsNotAOrB",
                   p.c.str() + "," + p.b.str() + "," + p.a.str());
    return 0 * assert_outside(p.d.get_hr(), {p.a.get_hr(), p.b.get_hr(), p.c.get_hr()}, "DIsNotABOrC",
                              p.d.str() + "," + p.a.str() + "," + p.b.str() + "," + p.d.str());
}

/// Local coordinates returned should be inside [-1, 1]. Run this test 50 times.
///
/// @return 0 when finished.
int TestAsterism::test_hash_normalized () {
    bool is_not_normal = false;
    
    for (int i = 0; i < 50; i++) {
        Asterism::points_cd m = Asterism::hash({Star::chance(), Star::chance(), Star::chance(), Star::chance()});
        if (fabs(m[0]) > 1 || fabs(m[1]) > 1 || fabs(m[2]) > 1 || fabs(m[3]) > 1) {
            is_not_normal = true;
        }
    }
    
    return 0 * assert_false(is_not_normal, "NormalHashesGenerated");
}

/// Ensure the conditions x_c <= x_d and x_c + x_d <= 1 hold true. Run test 50 times.
///
/// @return 0 when finished.
int TestAsterism::test_cd_symmetry () {
    bool is_not_symmetrical = false;
    
    for (int i = 0; i < 50; i++) {
        Asterism::points_cd m = Asterism::hash({Star::chance(), Star::chance(), Star::chance(), Star::chance()});
        if ((m[0] < m[2] || m[0] + m[2] > 1) && m[0] + m[1] + m[2] + m[3] != 0) {
            is_not_symmetrical = true;
        }
    }
    
    return 0 * assert_false(is_not_symmetrical, "StarsCandDNotSymmetrical");
}

/// Ensure that the center of a n=4 group of stars is **unique**. The fact that the stars actually lie dead in the
/// center of the asterism isn't important.
///
/// @return 0 when finished.
int TestAsterism::test_center () {
    Star::list a;
    a.reserve(10000);
    for (int i = 0; i < 10000; i++) {
        Asterism::stars b = {Star::chance(), Star::chance(), Star::chance(), Star::chance()};
        a.push_back(Asterism::center(b));
    }
    
    bool assertion = true;
    for (unsigned int i = 0; i < a.size(); i++) {
        for (unsigned int j = 0; j < a.size(); j++) {
            if (a[i] == a[j] && i != j) {
                assertion = false;
            }
        }
    }
    return 0 * assert_true(assertion, "NoDuplicatesExist");
}

/// Enumerate all tests in TestAsterism.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestAsterism::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_abcd_star_find();
        case 1: return test_hash_normalized();
        case 2: return test_cd_symmetry();
        case 3: return test_center();
        default: return -1;
    }
}

/// Run the tests in TestAsterism. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    return TestAsterism().execute_tests(BaseTest::FULL_PRINT_LOG_OFF);
}
