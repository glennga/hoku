/// @file test-pyramid.cpp
/// @author Glenn Galvizo
///
/// Source file for all Pyramid class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "identification/pyramid.h"
#include "gmock/gmock.h"

// Import several matchers from Google Mock.
using testing::UnorderedElementsAre;
using testing::Each;
using testing::Contains;

/// Check that common_stars method returns the correct star.
TEST(PyramidCommon, CommonStars) {
    Chomp ch;
    
    Pyramid::label_list_pair ei = {Pyramid::label_pair {3, 100}, Pyramid::label_pair{3, 413},
        Pyramid::label_pair {7, 87}};
    Pyramid::label_list_pair ej = {Pyramid::label_pair {3, 2}, Pyramid::label_pair{3, 5}, Pyramid::label_pair {13, 87}};
    Pyramid::label_list_pair ek = {Pyramid::label_pair {90, 12345}, Pyramid::label_pair{3, 7352},
        Pyramid::label_pair {9874, 512}}, h = ei;
    Pyramid a(Benchmark(ch, 20), Pyramid::DEFAULT_PARAMETERS);
    
    h.insert(h.end(), ej.begin(), ej.end());
    Star::list b = a.common_stars(ek, h, Star::list {});
    
    EXPECT_THAT(b, Contains(ch.query_hip(3)));
}

/// Check that find_candidate_trio method returns the correct quad.
TEST(PyramidTrio, CandidateTrioFind) {
    Chomp ch;
    Benchmark input(ch, 20);
    Identification::Parameters par = Pyramid::DEFAULT_PARAMETERS;
    par.sigma_query = 10e-7;
    Pyramid a(input, par);
    Pyramid::star_trio b = a.find_candidate_trio({a.input[0], a.input[1], a.input[2]});
    
    EXPECT_EQ(input.stars[0].get_label(), b[0].get_label());
    EXPECT_EQ(input.stars[1].get_label(), b[1].get_label());
    EXPECT_EQ(input.stars[2].get_label(), b[2].get_label());
}

/// Check that a clean input returns the expected query result.
TEST(PyramidTrial, CleanQuery) {
    Chomp ch;
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    p.sigma_query = 10e-9;
    Benchmark input(ch, 15);
    Pyramid a(Benchmark::black(), p);
    
    // We only use the first two stars for querying here.
    std::vector<Identification::labels_list> d = a.query({input.stars[0], input.stars[1]});
    Identification::labels_list ell = {input.stars[0].get_label(), input.stars[1].get_label()};
    
    std::sort(ell.begin(), ell.end());
    EXPECT_THAT(d, Contains(ell));
}

/// Check that a clean input returns the correct stars from a set of candidates.
TEST(PyramidTrial, CleanReduction) {
    Chomp ch;
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    p.sigma_query = 10e-10;
    Benchmark input(ch, 15);
    Pyramid a(input, p);
    Identification::labels_list ell = {input.stars[0].get_label(), input.stars[1].get_label(),
        input.stars[2].get_label()};
    
    std::sort(ell.begin(), ell.end());
    EXPECT_THAT(a.reduce(), UnorderedElementsAre(ell[0], ell[1], ell[2]));
}

/// Check that a clean input returns the expected alignment of stars.
TEST(PyramidTrial, CleanAlignment) {
    Chomp ch;
    Rotation q = Rotation::chance();
    Star focus = Star::chance();
    unsigned int nu = 0;
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    p.sigma_query = 10e-9;
    p.nu = std::make_shared<unsigned int>(nu);
    Benchmark input(ch, focus, q, 15, 6.0);
    
    Star::list b = {Rotation::rotate(input.stars[0], q), Rotation::rotate(input.stars[1], q),
        Rotation::rotate(input.stars[2], q), Rotation::rotate(input.stars[3], q)};
    Star::list c = {ch.query_hip(input.stars[0].get_label()), ch.query_hip(input.stars[1].get_label()),
        ch.query_hip(input.stars[2].get_label()), ch.query_hip(input.stars[3].get_label())};
    
    Pyramid a(Benchmark(b, Rotation::rotate(focus, q), 20), p);
    Star::list f = a.align();
    EXPECT_THAT(f, Contains(Star::define_label(b[0], c[0].get_label())));
    EXPECT_THAT(f, Contains(Star::define_label(b[1], c[1].get_label())));
    EXPECT_THAT(f, Contains(Star::define_label(b[2], c[2].get_label())));
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