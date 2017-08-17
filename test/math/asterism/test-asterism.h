/// @file test-asterism.h
/// @author Glenn Galvizo
///
/// Header file for the TestAsterism class, which tests the Asterism class.

#ifndef TEST_ASTERISM_H
#define TEST_ASTERISM_H

#include "base-test.h"
#include "asterism.h"

class TestAsterism : public BaseTest {
  private:
    int test_abcd_star_find ();
    int test_hash_normalized ();
    int test_cd_symmetry ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_ASTERISM_H */