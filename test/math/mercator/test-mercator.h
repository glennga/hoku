/// @file test-mercator.h
/// @author Glenn Galvizo
///
/// Header file for the TestMercator class, which tests the Mercator class.

#ifndef TEST_MERCATOR_H
#define TEST_MERCATOR_H

#include "base-test.h"
#include "mercator.h"

class TestMercator: public BaseTest
{
private:
    int test_projection_within_bounds();
    int test_reduction_within_bounds();
    int test_present_projection();
    int test_corners_form_box();
    int test_is_within_bounds();

public:
    int enumerate_tests(int);
};

#endif /* TEST_MERCATOR_H */