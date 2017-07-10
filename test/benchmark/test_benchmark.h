/*
 * @file: test_benchmark.h
 *
 * @brief: Header file for the TestBenchmark class, which tests the Benchmark class.
 */

#ifndef TEST_BENCHMARK_H
#define TEST_BENCHMARK_H

#include "base_test.h"
#include "benchmark.h"

class TestBenchmark : public BaseTest {
    private:
        // test the shuffle method
        void test_star_shuffle();

        // test the star set recording method
        void test_current_plot_file();
        void test_error_plot_file();

        // test that all error methods place stars near focus
        void test_error_near_focus();

        // test the light adding method
        void test_extra_light_added();

        // test the light removal method
        void test_removed_light_removed();

        // test the light shifting method
        void test_shifted_light_shifted();
        
        // test the data presenting method
        void test_bsc_id_clear();

    public:
        int enumerate_tests(int);
};

#endif /* TEST_BENCHMARK_H */