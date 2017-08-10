/// @file test-mercator.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestMercator class, as well as the main function to run the tests.

#include "test-mercator.h"

/// Check that the conversion from cartesian to mercator is one that produces coordinates with bounds of w.
///
/// @return 0 when finished.
int TestMercator::test_projection_within_bounds()
{
    Star a(3, 4, 5), b = Star::chance();

    assert_within(Mercator(a, 200).x, -100, 100, "XWithinBoundsStar1");
    assert_within(Mercator(a, 200).y, -100, 100, "YWithinBoundsStar1");
    assert_within(Mercator(b, 500).x, -250, 250, "XWithinBoundsStar2");
    return 0 * assert_within(Mercator(b, 500).y, -250, 250, "YWithinBoundsStar2");
}

/// Check that all points returned for reduction method are within the bounds of w.
///
/// @return 0 when finished.
int TestMercator::test_reduction_within_bounds()
{
    std::vector<Mercator> a;
    Mercator::list b;
    int current = 0;

    a.reserve(50);
    for (int q = 0; q < 50; q++)
    {
        a.push_back(Mercator(Star::chance(), 500));
    }
    b = Mercator(250, 250, 0).reduce_far_points(a, 200);

    for (const Mercator &m : b)
    {
        std::string partial_name = "WithinBoundsStar" + std::to_string(current++ + 1);
        assert_within(m.x, 250 - (200 / 2.0), 250 + (200 / 2.0), "X" + partial_name);
        assert_within(m.y, 250 - (200 / 2.0), 250 + (200 / 2.0), "Y" + partial_name);
    }

    return 0;
}

/// Check that the variables changed in present_projection are set correctly.
///
/// @return 0 when finished.
int TestMercator::test_present_projection()
{
    Mercator a(1, 2, 3, 4);
    double b[4];
    int c;

    a.present_projection(b[0], b[1], b[2], c);
    assert_equal(b[0], 1, "PresentProjectionXComponent");
    assert_equal(b[1], 2, "PresentProjectionYComponent");
    assert_equal(b[2], 3, "PresentProjectionW_NComponent");
    return 0 * assert_equal(c, 4, "PresentProjectionHRComponent");
}

/// Check that the corners returned actually form a box.
///
/// @return 0 when finished.
int TestMercator::test_corners_form_box()
{
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
int TestMercator::test_is_within_bounds()
{
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
int TestMercator::enumerate_tests(int test_case)
{
    switch (test_case)
    {
    case 0: return test_projection_within_bounds();
    case 1: return test_reduction_within_bounds();
    case 2: return test_present_projection();
    case 3: return test_corners_form_box();
    case 4: return test_is_within_bounds();
    default: return -1;
    }
}

/// Run the tests in TestMercator. Currently set to log all results.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main()
{
    return TestMercator().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
