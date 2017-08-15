/// @file test-mercator.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestMercator class, as well as the main function to run the tests.

#include "test-mercator.h"

/// Check that the conversion from cartesian to mercator is one that produces coordinates with bounds of w.
///
/// @return 0 when finished.
int TestMercator::test_projection_within_bounds () {
    Star a(3, 4, 5), b = Star::chance();
    
    assert_within(Mercator(a, 200).x, -100, 100, "XWithinBoundsStar1");
    assert_within(Mercator(a, 200).y, -100, 100, "YWithinBoundsStar1");
    assert_within(Mercator(b, 500).x, -250, 250, "XWithinBoundsStar2");
    return 0 * assert_within(Mercator(b, 500).y, -250, 250, "YWithinBoundsStar2");
}

/// Check that the corners returned actually form a box.
///
/// @return 0 when finished.
int TestMercator::test_corners_form_box () {
    Mercator a(Star::chance(), 1000);
    Mercator::quad b = a.find_corners(100);
    
    assert_equal(b[0].y, b[1].y, "TopLineSameY");
    assert_equal(b[2].y, b[3].y, "BottomLineSameY");
    assert_equal(b[0].x, b[2].x, "LeftlineSameX");
    return 0 * assert_equal(b[1].x, b[3].x, "RightLineSameX");
}

/// Check that points are correctly distinguished from being outside and inside a given boundary.
///
/// @return 0 when finished.
int TestMercator::test_is_within_bounds () {
    Mercator::quad a = Mercator(0, 0, 1000).find_corners(100);
    std::stringstream p;
    
    p << std::setprecision(16) << std::fixed;
    p << a[0].x << "," << a[0].y << "," << a[1].x << "," << a[1].y << ",";
    p << a[2].x << "," << a[2].y << "," << a[3].x << "," << a[3].y;
    
    assert_false(Mercator(5000, 5000, 1000).is_within_bounds(a), "PointNotWithinBounds", p.str());
    return 0 * assert_true(Mercator(1, 1, 1000).is_within_bounds(a), "PointWithinBounds", p.str());
}

/// Enumerate all tests in TestMercator.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestMercator::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_projection_within_bounds();
        case 1: return test_corners_form_box();
        case 2: return test_is_within_bounds();
        default: return -1;
    }
}

/// Run the tests in TestMercator. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    return TestMercator().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}