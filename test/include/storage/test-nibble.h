/// @file test-nibble.h
/// @author Glenn Galvizo
///
/// Header file for the TestNibble class, which tests the Nibble class. This assumes that the bright star table
/// generator in Chomp works.

#ifndef TEST_NIBBLE_H
#define TEST_NIBBLE_H

#include "base-test/base-test.h"
#include "storage/nibble.h"
#include "storage/chomp.h"

class TestNibble : public BaseTest {
  private:
    int test_file_existence ();
    int test_table_search_result ();
    int test_table_polish_index ();
    int test_table_polish_sort ();
    int test_table_insertion ();
    int test_in_memory_instance ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_NIBBLE_H */