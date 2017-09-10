/// @file test-nibble.h
/// @author Glenn Galvizo
///
/// Header file for the TestNibble class, which tests the Nibble class.

#ifndef TEST_NIBBLE_H
#define TEST_NIBBLE_H

#include "base-test/base-test.h"
#include "../../../include/storage/nibble.h"

class TestNibble : public BaseTest {
  private:
    int test_components_from_line ();
    int test_file_existence ();
    int test_bsc5_table_existence ();
    int test_bsc5_query_result ();
    int test_table_search_result ();
    int test_table_search_result_index ();
    int test_table_polish_index ();
    int test_table_polish_sort ();
    int test_table_insertion ();
    int test_bsc5_all_stars_grab ();
    int test_nearby_star_grab ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_NIBBLE_H */