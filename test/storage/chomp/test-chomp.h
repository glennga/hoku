/*
 * @file: test-nibble.h
 *
 * @brief: Header file for the TestNibble namespace, which tests the Nibble namespace.
 */

#ifndef TEST_CHOMP_H
#define TEST_CHOMP_H

#include "base-test.h"
#include "chomp.h"
#include <cstdio>

class TestChomp : public BaseTest {
    private:
        // test regular queries vs k-vector queries
        void test_regular_query();
        void test_k_vector_query();

//        // test the line component grabbing method
//        void test_components_from_line();
//
//        // test the bsc5 table generate method
//        void test_file_existence();
//        void test_bsc5_table_existence();
//
//        // test the bsc5 query method
//        void test_bsc5_query_result();
//        void test_bsc5_db_query_result();
//
//        // test the general table component grabbing method
//        void test_table_search_result();
//        void test_table_search_result_index();
//
//        // test the table polish method
//        void test_table_polish_index();
//        void test_table_polish_sort();
//
//        // test the table insertion method
//        void test_table_insertion();
//
//        // test the id grabbing method
//        void test_bsc5_all_stars_grab();
//
//        // test the nearby star grabbing method
//        void test_nearby_star_grab();

    public:
        int enumerate_tests(int);
};

#endif /* TEST_CHOMP_H */