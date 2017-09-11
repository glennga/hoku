/// @file test-rotation.h
/// @author Glenn Galvizo
///
/// Header file for the TestRotation class, which tests the Rotation class.

#ifndef TEST_ROTATION_H
#define TEST_ROTATION_H

#include "base-test/base-test.h"
#include "math/rotation.h"

class TestRotation : public BaseTest {
  private:
    int test_public_constructor ();
    int test_private_constructor_row_set ();
    int test_private_constructor_component_set ();
    int test_matrix_multiplication_transpose ();
    int test_matrix_to_quaternion ();
    int test_quaternion_double_cover_property ();
    int test_quaternion_unit_property ();
    int test_rotate_logic ();
    int test_rotation_identity ();
    int test_triad_property_simple ();
    int test_triad_property_random ();
    int test_triad_multiple_stars ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_ROTATION_H */