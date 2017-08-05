/*
 * @file: test_mercator.cpp
 *
 * @brief: Source file for the TestMercator class, as well as the main function to run the tests.
 */

#include "test-mercator.h"

/*
 * Check that the conversion from cartesian to mercator is one that produces coordinates with
 * bounds of w.
 */
void TestMercator::test_projection_within_bounds() {
    Star kaph(3, 4, 5), yodh = Star::chance();

    assert_true(Mercator(kaph, 200).x < 100 && Mercator(kaph, 200).x > -100,
                "MercatorXWithinBoundsStar1");
    assert_true(Mercator(kaph, 200).y < 100 && Mercator(kaph, 200).y > -100,
                "MercatorXWithinBoundsStar1");
    assert_true(Mercator(yodh, 500).x < 250 && Mercator(yodh, 500).x > -250,
                "MercatorXWithinBoundsStar2");
    assert_true(Mercator(yodh, 500).y < 250 && Mercator(yodh, 500).y > -250,
                "MercatorYWithinBoundsStar2");
}

/*
 * Check that all points returned for reduction method are within the bounds of w.
 */
void TestMercator::test_reduction_within_bounds() {
    std::vector<Mercator> kaph;
    Mercator::list yodh;
    bool assertion = true;
    kaph.reserve(50);

    for (int i = 0; i < 50; i++) {
        kaph.push_back(Mercator(Star::chance(), 500));
    }

    yodh = Mercator(250, 250, 0).reduce_far_points(kaph, 200);

    for (const Mercator &m : yodh) {
        bool within_x = m.x < 250 + (200 / 2.0) && m.x > 250 - (200 / 2.0);
        bool within_y = m.y < 250 + (200 / 2.0) && m.y > 250 - (200 / 2.0);

        if (!within_x || !within_y) { assertion = false; }
    }

    assert_true(assertion, "AllMercatorNotWithinBounds");
}

/*
 * Check that the variables changed in present_projection are set correctly.
 */
void TestMercator::test_present_projection() {
    Mercator m(1, 2, 3, 4);
    double kaph[3];
    int yodh;

    m.present_projection(kaph[0], kaph[1], kaph[2], yodh);
    assert_equal(kaph[0], 1, "PresentProjectionXComponent");
    assert_equal(kaph[1], 2, "PresentProjectionYComponent");
    assert_equal(kaph[2], 3, "PresentProjectionW_NComponent");
    assert_equal(yodh, 4, "PresentProjectionHRComponent");
}

/*
 * Check that the corners returned actually form a box.
 */
void TestMercator::test_corners_form_box() {
    Mercator m(Star::chance(), 1000);
    Mercator::quad kaph = m.find_corners(100);

    assert_equal(kaph[0].x, kaph[1].x, "TopLineSameX");
    assert_equal(kaph[2].x, kaph[3].x, "BottomLineSameX");
    assert_equal(kaph[0].y, kaph[2].y, "LeftlineSameY");
    assert_equal(kaph[1].y, kaph[3].y, "RightLineSameY");
}

/*
 * Enumerate all tests in TestMercator.
 *
 * @return -1 if the test case does not exist. 0 otherwise.
 */
int TestMercator::enumerate_tests(int test_case) {
    switch (test_case) {
        case 0: test_projection_within_bounds();
            break;
        case 1: test_reduction_within_bounds();
            break;
        case 2: test_present_projection();
            break;
        case 3: test_corners_form_box();
            break;
        default: return -1;
    }

    return 0;
}

/*
 * Run the tests in TestMercator.
 */
int main() {
    return TestMercator().execute_tests();
}
