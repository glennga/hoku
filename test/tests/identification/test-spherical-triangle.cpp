/// @file test-spherical-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for all SphericalTriangle class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "gmock/gmock.h"

#include "identification/spherical-triangle.h"
#include "math/trio.h"

// Import several matchers from Google Mock.
using testing::UnorderedElementsAre;
using testing::Each;
using testing::Contains;

/// Check that query_for_trio method returns the catalog ID of the correct stars.
TEST(SphereQuery, TrioQuery) {
    Chomp ch;
    Benchmark input(ch, 15);
    Sphere::Parameters par = Sphere::DEFAULT_PARAMETERS;
    par.sigma_query = 0.000000001;
    Sphere p(input, par);
    
    double a = Trio::spherical_area(input.b[0], input.b[1], input.b[2]);
    double b = Trio::spherical_moment(input.b[0], input.b[1], input.b[2]);
    std::vector<Sphere::label_trio> c = p.query_for_trio(a, b);
    
    // Check that original input trio exists in search.
    for (const Sphere::label_trio &t : c) {
        for (int i = 0; i < 3; i++) {
            EXPECT_THAT(t, Contains(input.b[i].get_label()));
        }
    }
}


/// Check that the zero-length stars are returned when an area between a pair of stars is greater than the current fov.
TEST(SphereQuery, MatchStarsFOV) {
    Chomp ch;
    Sphere::Parameters par = Sphere::DEFAULT_PARAMETERS;
    Sphere a(Benchmark(ch, 10), par);
    a.big_i[0] = Star::reset_label(ch.query_hip(3));
    a.big_i[1] = Star::reset_label(ch.query_hip(4));
    a.big_i[2] = Star::reset_label(ch.query_hip(5));
    
    std::vector<Star::trio> b = a.query_for_trios(Sphere::STARTING_INDEX_TRIO);
    EXPECT_THAT(b[0], Each(Star::zero()));
}

/// Check that the zero-length stars are returned when no matching trio is found.
TEST(SphereQuery, MatchStarsNone) {
    Sphere::Parameters par = Sphere::DEFAULT_PARAMETERS;
    par.sigma_query = std::numeric_limits<double>::epsilon();
    Chomp ch;
    Sphere a(Benchmark(ch, 10), par);
    a.big_i[0] = Star(1, 1, 1.1);
    a.big_i[1] = Star(1, 1, 1);
    a.big_i[2] = Star(1.1, 1, 1);
    
    std::vector<Star::trio> b = a.query_for_trios(Sphere::STARTING_INDEX_TRIO);
    EXPECT_THAT(b[0], Each(Star::zero()));
}

/// Check that the correct stars are returned from the candidate trio query.
TEST(SphereQuery, MatchStarsResults) {
    Sphere::Parameters par = Sphere::DEFAULT_PARAMETERS;
    par.sigma_query = 10e-9;
    Chomp ch;
    Benchmark input(ch, 20);
    Sphere a(input, par);
    std::vector<Star::trio> b = a.query_for_trios(Sphere::STARTING_INDEX_TRIO);
    
    // Check that original input trio exists in search.
    for (const Star::trio &t : b) {
        for (int i = 0; i < 3; i++) {
            Identification::labels_list t_ell = {t[0].get_label(), t[1].get_label(), t[2].get_label()};
            EXPECT_THAT(t_ell, Contains(input.b[i].get_label()));
        }
    }
}

/// Check that the pivot query method returns the correct trio.
TEST(SphereQuery, PivotResults) {
    Sphere::Parameters par = Sphere::DEFAULT_PARAMETERS;
    par.sigma_query = 10e-9;
    par.sigma_overlay = 0.000001;
    Chomp ch;
    Benchmark input(ch, 20);
    Sphere a(input, par);
    
    Star::trio c = a.pivot(Sphere::STARTING_INDEX_TRIO);
    std::vector<int> c_ell = {c[0].get_label(), c[1].get_label(), c[2].get_label()};
    EXPECT_THAT(c_ell, Contains(input.b[0].get_label()));
    EXPECT_THAT(c_ell, Contains(input.b[1].get_label()));
    EXPECT_THAT(c_ell, Contains(input.b[2].get_label()));
}

/// Check that the rotating match method marks the all stars as matched.
TEST(SphereMatch, RotatingCorrectInput) {
    Sphere::Parameters par = Sphere::DEFAULT_PARAMETERS;
    Chomp ch;
    Star a = Star::chance(), b = Star::chance();
    Rotation c = Rotation::chance();
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::triad({a, b}, {d, e});
    Benchmark input(ch, Star::chance(), c, 8);
    std::vector<Star> rev_input;
    par.sigma_overlay = 0.000001;
    Sphere g(input, par);
    
    // Reverse all input by inverse rotation.
    rev_input.reserve(input.b.size());
    for (Star rotated : input.b) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    std::vector<Star> h = g.find_positive_overlay(rev_input, c);
    ASSERT_EQ(h.size(), input.b.size());
    for (unsigned int q = 0; q < h.size(); q++) {
        EXPECT_EQ(h[q].get_label(), input.b[q].get_label());
    }
}

/// Check that the rotating match method marks only the correct stars as matched.
TEST(SphereMatch, RotatingErrorInput) {
    Sphere::Parameters par = Sphere::DEFAULT_PARAMETERS;
    Chomp ch;
    Star a = Star::chance(), b = Star::chance();
    Rotation c = Rotation::chance();
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::triad({a, b}, {d, e});
    Benchmark input(ch, Star::chance(), c, 8);
    std::vector<Star> rev_input;
    par.sigma_overlay = 0.000001;
    Sphere g(input, par);
    
    // Reverse all input by inverse rotation.
    rev_input.reserve(input.b.size());
    for (Star rotated : input.b) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    // Append center as error.
    rev_input.push_back(input.center);
    std::vector<Star> h = g.find_positive_overlay(rev_input, c);
    ASSERT_EQ(h.size(), input.b.size());
    for (unsigned int q = 0; q < h.size(); q++) {
        EXPECT_EQ(h[q].get_label(), input.b[q].get_label());
    }
}

/// Check that the rotating match method marks only the correct stars as matched, not the duplicate as well.
TEST(SphereMatch, RotatingDuplicateInput) {
    Sphere::Parameters par = Sphere::DEFAULT_PARAMETERS;
    Chomp ch;
    Star a = Star::chance(), b = Star::chance();
    Rotation c = Rotation::chance();
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::triad({a, b}, {d, e});
    Benchmark input(ch, Star::chance(), c, 8);
    std::vector<Star> rev_input;
    par.sigma_overlay = 0.000001;
    Sphere g(input, par);
    
    // Reverse all input by inverse rotation.
    rev_input.reserve(input.b.size());
    for (Star rotated : input.b) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    // Append first star as error.
    rev_input.push_back(rev_input[0]);
    rev_input.push_back(rev_input[0]);
    rev_input.push_back(rev_input[0]);
    
    std::vector<Star> h = g.find_positive_overlay(rev_input, c);
    ASSERT_EQ(h.size(), input.b.size());
    for (unsigned int q = 0; q < h.size(); q++) {
        EXPECT_EQ(h[q].get_label(), input.b[q].get_label());
    }
}

/// Check that a clean input returns the expected query result.
TEST(SphereTrial, CleanQuery) {
    Chomp ch;
    Sphere::Parameters p = Sphere::DEFAULT_PARAMETERS;
    p.sigma_query = 10e-9;
    Benchmark input(ch, 15);
    Sphere a(Benchmark::black(), p);
    
    std::vector<Identification::labels_list> d = a.query({input.b[0], input.b[1], input.b[2]});
    Identification::labels_list ell = {input.b[0].get_label(), input.b[1].get_label(),
        input.b[2].get_label()};
    
    std::sort(ell.begin(), ell.end());
    EXPECT_THAT(d, Contains(ell));
}

/// Check that a clean input returns the correct stars from a set of candidates.
TEST(SphereTrial, CleanReduction) {
    Chomp ch;
    Sphere::Parameters p = Sphere::DEFAULT_PARAMETERS;
    p.sigma_query = 10e-10;
    p.sigma_overlay = 0.0001;
    Benchmark input(ch, 15);
    Sphere a(input, p);
    Identification::labels_list ell = {input.b[0].get_label(), input.b[1].get_label(),
        input.b[2].get_label()};
    
    std::sort(ell.begin(), ell.end());
    EXPECT_THAT(a.reduce(), UnorderedElementsAre(ell[0], ell[1], ell[2]));
}

/// Check that a clean input returns the expected identification of stars.
TEST(SphereTrial, CleanIdentify) {
    Chomp ch;
    Rotation q = Rotation::chance();
    Star focus = Star::chance();
    unsigned int nu = 0;
    Sphere::Parameters p = Sphere::DEFAULT_PARAMETERS;
    p.sigma_query = 10e-9;
    p.sigma_overlay = 0.000001;
    p.nu = std::make_shared<unsigned int>(nu);
    Benchmark input(ch, focus, q, 15, 6.0);
    
    Star::list b = {Rotation::rotate(input.b[0], q), Rotation::rotate(input.b[1], q),
        Rotation::rotate(input.b[2], q)};
    Star::list c = {ch.query_hip(input.b[0].get_label()), ch.query_hip(input.b[1].get_label()),
        ch.query_hip(input.b[2].get_label())};
    
    Sphere a(Benchmark(b, Rotation::rotate(focus, q), 20), p);
    Star::list f = a.identify();
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