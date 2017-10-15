/// @file test-chomp.h
/// @author Glenn Galvizo
///
/// Header file for the TestChomp class, which tests the Chomp class.

#ifndef TEST_CHOMP_H
#define TEST_CHOMP_H

#include "base-test/base-test.h"
#include "storage/chomp.h"
#include <cstdio>

class TestChomp : public BaseTest {
  private:
    int test_regular_query ();
    int test_k_vector_query ();
    int test_bright_star_grab();
    int test_nearby_bright_star_grab();
    int test_nearby_hip_star_grab();
    int test_components_from_line();
    int test_star_table_existence();
    int test_hip_query_result();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_CHOMP_H */