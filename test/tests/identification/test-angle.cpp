/// @file test-angle.cpp
/// @author Glenn Galvizo
///
/// Source file for all Angle class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "identification/angle.h"
#include "gmock/gmock.h"

// Import several matchers from Google Mock.
using testing::UnorderedElementsAre;
using testing::Contains;
using testing::Not;

/// Check that query_for_pair method returns the catalog ID of the correct stars.
TEST(AngleQuery, Pair) {
    Chomp ch;
    Benchmark input(ch, 15);
    
    double a = Star::angle_between(input.b[0], input.b[1]);
    Identification::labels_list b = Angle(input, Angle::DEFAULT_PARAMETERS).query_for_pair(a);
    std::vector<int> c = {input.b[0].get_label(), input.b[1].get_label()};
    std::vector<int> d = {input.b[0].get_label(), input.b[1].get_label()};
    EXPECT_THAT(c, Contains(b[0]));
    EXPECT_THAT(d, Contains(b[1]));
}

/// Check that the zero-length stars are returned wgn theta is greater than the current fov.
TEST(AngleQuery, FOV) {
    Chomp ch;
    Angle a(Benchmark(ch, 10), Angle::DEFAULT_PARAMETERS);
    Star b(0.928454687492219, 0.132930961972911, 0.346844709665121);
    Star c(0.998078771188383, -0.0350062881876723, 0.0511207031486225);
    
    std::array<Star, 2> d = a.find_candidate_pair(b, c);
    EXPECT_EQ(d[0], Star::zero());
    EXPECT_EQ(d[1], Star::zero());
}

/// Check that the zero-length stars are returned when no matching theta is found.
TEST(AngleQuery, None) {
    Chomp ch;
    Angle a(Benchmark(ch, 10), Angle::DEFAULT_PARAMETERS);
    
    std::array<Star, 2> b = a.find_candidate_pair(Star(1, 1, 1), Star(1.1, 1, 1));
    EXPECT_EQ(b[0], Star::zero());
    EXPECT_EQ(b[1], Star::zero());
}

/// Check that the correct stars are returned from the candidate pair query.
TEST(AngleResults, Query) {
    Chomp ch;
    Benchmark input(ch, 15);
    Angle b(input, Angle::DEFAULT_PARAMETERS);
    
    std::array<Star, 2> c = b.find_candidate_pair(input.b[0], input.b[1]);
    std::vector<int> d = {input.b[0].get_label(), input.b[1].get_label()};
    std::vector<int> e = {input.b[0].get_label(), input.b[1].get_label()};
    EXPECT_THAT(d, Contains(c[0].get_label()));
    EXPECT_THAT(e, Contains(c[1].get_label()));
}

/// Check that a clean input returns the expected query result.
TEST(AngleTrial, CleanQuery) {
    Chomp ch;
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.sigma_query = 10e-7;
    Angle a(Benchmark::black(), p);
    Star b = ch.query_hip(103215), c = ch.query_hip(103217);
    
    std::vector<Identification::labels_list> d = a.query({b, c});
    EXPECT_THAT(d, Contains(Identification::labels_list {103215, 103217}));
}

/// Check that a clean input returns the correct stars from a set of candidates.
TEST(AngleTrial, CleanReduction) {
    Chomp ch;
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.sigma_query = 10e-9;
    Benchmark i = Benchmark::black();
    i.b = {ch.query_hip(103215), ch.query_hip(103217)};
    
    Angle a(i, p);
    EXPECT_THAT(a.reduce(), UnorderedElementsAre(103215, 103217));
}

/// Check that a clean input returns the expected identification of stars.
TEST(AngleTrial, CleanIdentify) {
    Chomp ch;
    unsigned int nu;
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.nu = std::make_shared<unsigned int>(nu);
    p.sigma_query = 10e-9;
    p.sigma_overlay = 0.000001;
    
    Rotation q = Rotation::chance();
    Star b = ch.query_hip(103215), c = ch.query_hip(103217);
    Star d = Rotation::rotate(b, q), e = Rotation::rotate(c, q);
    
    Angle a(Benchmark({d, e}, d, 20), p);
    Star::list f = a.identify();
    EXPECT_THAT(f, Contains(Star::define_label(d, 103215)));
    EXPECT_THAT(f, Contains(Star::define_label(e, 103217)));
}

/// Runs all tests defined in this file.
///
/// @param argc Argument count. Used in Google Test initialization.
/// @param argv Argument vector. Used in Google Test initialization.
/// @return The result of running all tests.
int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}