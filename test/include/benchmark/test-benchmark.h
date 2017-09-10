/// @file test-benchmark.h
/// @author Glenn Galvizo
///
/// Header file for the TestBenchmark class, which tests the Benchmark class.


#ifndef TEST_BENCHMARK_H
#define TEST_BENCHMARK_H

#include "base-test/base-test.h"
#include "benchmark/benchmark.h"

class TestBenchmark : public BaseTest {
  private:
    int test_star_shuffle ();
    int test_current_plot_file ();
    int test_error_plot_file ();
    int test_error_near_focus ();
    int test_extra_light_added ();
    int test_removed_light_removed ();
    int test_shifted_light_shifted ();
    int test_hr_number_clear ();
    int test_display_plot ();
    int test_nibble_insertion ();
    int test_compare_stars();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_BENCHMARK_H */