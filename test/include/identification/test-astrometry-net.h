/// @file test-astrometry-net.h
/// @author Glenn Galvizo
///
/// Header file for the TestAstrometryNet class, which tests the AstrometryNet class.


#ifndef TEST_ASTROMETRY_NET_H
#define TEST_ASTROMETRY_NET_H

#include "base-test/base-test.h"
#include "../../../include/identification/astrometry-net.h"
#include <cstdio>

class TestAstrometryNet : public BaseTest {
  private:
    int test_astro_h_insertion();
    int test_asterism_query();
    int test_identify_clean_input ();
    int test_identify_error_input ();
  
  public:
    int enumerate_tests (int);
};

#endif /* TEST_ASTROMETRY_NET_H */