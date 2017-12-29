/// @file test-planar-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for all PlanarTriangle class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "identification/planar-triangle.h"
#include "gmock/gmock.h"

// Import several matchers from Google Mock.
using testing::UnorderedElementsAre;
using testing::Each;
using testing::Contains;

/// Check that query_for_trio method returns the catalog ID of the correct stars.
TEST(PlaneQuery, TrioQuery) {
    std::random_device seed;
    Chomp ch;
    Benchmark input(ch, seed, 15);
    Plane::Parameters par = Plane::DEFAULT_PARAMETERS;
    Plane p(input, par);
    
    double a = Trio::planar_area(input.stars[0], input.stars[1], input.stars[2]);
    double b = Trio::planar_moment(input.stars[0], input.stars[1], input.stars[2]);
    std::vector<Plane::label_trio> c = p.query_for_trio(a, b);
    
    // Check that original input trio exists in search.
    for (const Plane::label_trio &t : c) {
        for (int i = 0; i < 3; i++) {
            EXPECT_THAT(t, Contains(input.stars[i].get_label()));
        }
    }
}


/// Check that the zero-length stars are returned when an area between a pair of stars is greater than the current fov.
TEST(PlaneQuery, MatchStarsFOV) {
    std::random_device seed;
    Chomp ch;
    Plane::Parameters par = Plane::DEFAULT_PARAMETERS;
    Plane a(Benchmark(ch, seed, 10), par);
    a.input[0] = Star::reset_label(ch.query_hip(3));
    a.input[1] = Star::reset_label(ch.query_hip(4));
    a.input[2] = Star::reset_label(ch.query_hip(5));
    
    std::vector<Trio::stars> b = a.match_stars(Plane::STARTING_INDEX_TRIO);
    EXPECT_THAT(b[0], Each(Star::zero()));
}

/// Check that the zero-length stars are returned when no matching trio is found.
TEST(PlaneQuery, MatchStarsNone) {
    Plane::Parameters par = Plane::DEFAULT_PARAMETERS;
    par.sigma_query = std::numeric_limits<double>::epsilon();
    std::random_device seed;
    Chomp ch;
    Plane a(Benchmark(ch, seed, 10), par);
    a.input[0] = Star(1, 1, 1.1);
    a.input[1] = Star(1, 1, 1);
    a.input[2] = Star(1.1, 1, 1);
    
    std::vector<Trio::stars> b = a.match_stars(Plane::STARTING_INDEX_TRIO);
    EXPECT_THAT(b[0], Each(Star::zero()));
}

/// Check that the correct stars are returned from the candidate trio query.
TEST(PlaneQuery, MatchStarsResults) {
    Plane::Parameters par = Plane::DEFAULT_PARAMETERS;
    std::random_device seed;
    Chomp ch;
    Benchmark input(ch, seed, 20);
    Plane a(input, par);
    std::vector<Trio::stars> b = a.match_stars(Plane::STARTING_INDEX_TRIO);
    
    // Check that original input trio exists in search.
    for (const Trio::stars &t : b) {
        for (int i = 0; i < 3; i++) {
            Identification::labels_list t_ell = {t[0].get_label(), t[1].get_label(), t[2].get_label()};
            EXPECT_THAT(t_ell, Contains(input.stars[i].get_label()));
        }
    }
}

/// Check that the pivot query method returns the correct trio.
TEST(PlaneQuery, PivotResults) {
    Plane::Parameters par = Plane::DEFAULT_PARAMETERS;
    std::random_device seed;
    Chomp ch;
    Benchmark input(ch, seed, 20);
    Plane a(input, par);
    
    Trio::stars c = a.pivot(Plane::STARTING_INDEX_TRIO);
    std::vector<int> c_ell = {c[0].get_label(), c[1].get_label(), c[2].get_label()};
    EXPECT_THAT(c_ell, Contains(input.stars[0].get_label()));
    EXPECT_THAT(c_ell, Contains(input.stars[1].get_label()));
    EXPECT_THAT(c_ell, Contains(input.stars[2].get_label()));
}

/// Check that the rotating match method marks the all stars as matched.
TEST(PlaneMatch, RotatingCorrectInput) {
    Plane::Parameters par = Plane::DEFAULT_PARAMETERS;
    std::random_device seed;
    Chomp ch;
    Star a = Star::chance(seed), b = Star::chance(seed);
    Rotation c = Rotation::chance(seed);
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(ch, seed, Star::chance(seed), c, 8);
    std::vector<Star> rev_input;
    par.sigma_overlay = 0.000001;
    Plane g(input, par);
    
    // Reverse all input by inverse rotation.
    rev_input.reserve(input.stars.size());
    for (Star rotated : input.stars) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    std::vector<Star> h = g.find_matches(rev_input, c);
    ASSERT_EQ(h.size(), input.stars.size());
    for (unsigned int q = 0; q < h.size(); q++) {
        EXPECT_EQ(h[q].get_label(), input.stars[q].get_label());
    }
}

/// Check that the rotating match method marks only the correct stars as matched.
TEST(PlaneMatch, RotatingErrorInput) {
    Plane::Parameters par = Plane::DEFAULT_PARAMETERS;
    std::random_device seed;
    Chomp ch;
    Star a = Star::chance(seed), b = Star::chance(seed);
    Rotation c = Rotation::chance(seed);
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(ch, seed, Star::chance(seed), c, 8);
    std::vector<Star> rev_input;
    par.sigma_overlay = 0.000001;
    Plane g(input, par);
    
    // Reverse all input by inverse rotation.
    rev_input.reserve(input.stars.size());
    for (Star rotated : input.stars) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    // Append focus as error.
    rev_input.push_back(input.focus);
    std::vector<Star> h = g.find_matches(rev_input, c);
    ASSERT_EQ(h.size(), input.stars.size());
    for (unsigned int q = 0; q < h.size(); q++) {
        EXPECT_EQ(h[q].get_label(), input.stars[q].get_label());
    }
}

/// Check that the rotating match method marks only the correct stars as matched, not the duplicate as well.
TEST(PlaneMatch, RotatingDuplicateInput) {
    Plane::Parameters par = Plane::DEFAULT_PARAMETERS;
    std::random_device seed;
    Chomp ch;
    Star a = Star::chance(seed), b = Star::chance(seed);
    Rotation c = Rotation::chance(seed);
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(ch, seed, Star::chance(seed), c, 8);
    std::vector<Star> rev_input;
    par.sigma_overlay = 0.000001;
    Plane g(input, par);
    
    // Reverse all input by inverse rotation.
    rev_input.reserve(input.stars.size());
    for (Star rotated : input.stars) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    // Append first star as error.
    rev_input.push_back(rev_input[0]);
    rev_input.push_back(rev_input[0]);
    rev_input.push_back(rev_input[0]);
    
    std::vector<Star> h = g.find_matches(rev_input, c);
    ASSERT_EQ(h.size(), input.stars.size());
    for (unsigned int q = 0; q < h.size(); q++) {
        EXPECT_EQ(h[q].get_label(), input.stars[q].get_label());
    }
}

/// Check that correct result is returned with a clean input.
TEST(PlaneIdentify, CleanInput) {
    std::random_device seed;
    Chomp ch;
    Benchmark input(ch, seed, 8, 6.5);
    Plane::Parameters a = Plane::DEFAULT_PARAMETERS;
    unsigned int nu = 0;
    
    // We define a match as 66% here.
    a.gamma = 0.66;
    a.sigma_overlay = 0.000001;
    a.sigma_query = 10e-9;
    a.nu = std::make_shared<unsigned int>(nu);
    Star::list c = Plane(input, a).experiment_crown();
    ASSERT_GT(c.size(), a.gamma * c.size());
    
    std::string all_input = "";
    for (const Star &s : input.stars) {
        all_input += !(s == input.stars[input.stars.size() - 1]) ? s.str() + "," : s.str();
    }
    
    if (!c.empty()) {
        for (int q = 0; q < (signed) (c.size() - 1); q++) {
            auto match = [&c, q] (const Star &b) -> bool {
                return b.get_label() == c[q].get_label();
            };
            EXPECT_NE(std::find_if(input.stars.begin(), input.stars.end(), match), input.stars.end());
        }
    }
}

/// Check **a** correct result is returned with an error input.
TEST(PlaneIdentify, ErrorInput) {
    std::random_device seed;
    Chomp ch;
    Benchmark input(ch, seed, 20);
    Plane::Parameters a = Plane::DEFAULT_PARAMETERS;
    input.add_extra_light(1);
    unsigned int nu = 0;
    
    // We define a match as 25% here.
    a.gamma = 0.25;
    a.sigma_overlay = 0.0001;
    a.sigma_query = 10e-9;
    a.nu = std::make_shared<unsigned int>(nu);
    Star::list c = Plane(input, a).experiment_crown();
    ASSERT_GT(c.size(), a.gamma * c.size());
    
    if (!c.empty()) {
        std::string all_input = "";
        for (const Star &s : input.stars) {
            all_input += !(s == input.stars[input.stars.size() - 1]) ? s.str() + "," : s.str();
        }
        
        for (unsigned int q = 0; q < c.size() - 1; q++) {
            auto match = [&c, q] (const Star &b) -> bool {
                return b.get_label() == c[q].get_label();
            };
            EXPECT_NE(std::find_if(input.stars.begin(), input.stars.end(), match), input.stars.end());
        }
    }
}

/// Check that a clean input returns the expected query result.
TEST(PlaneTrial, CleanQuery) {
    std::random_device seed;
    Chomp ch;
    Plane::Parameters p = Plane::DEFAULT_PARAMETERS;
    p.sigma_query = 10e-9;
    Benchmark input(ch, seed, 15);
    Plane a(Benchmark::black(), p);
    
    std::vector<Identification::labels_list> d = a.experiment_query({input.stars[0], input.stars[1], input.stars[2]});
    Identification::labels_list ell = {input.stars[0].get_label(), input.stars[1].get_label(),
        input.stars[2].get_label()};
    
    std::sort(ell.begin(), ell.end());
    EXPECT_THAT(d, Contains(ell));
}

/// Check that a clean input returns the expected alignment of stars.
TEST(PlaneTrial, CleanFirstAlignment) {
    std::random_device seed;
    Chomp ch;
    Rotation q = Rotation::chance(seed);
    Star focus = Star::chance(seed);
    Plane::Parameters p = Plane::DEFAULT_PARAMETERS;
    p.sigma_overlay = 0.000001;
    Benchmark input(ch, seed, focus, q, 15, 6.0);
    Plane a(input, p);
    
    Star::list b = {a.input[0], a.input[1], a.input[2]}, d = {a.input[0], a.input[1]};
    Star::list c = {ch.query_hip(input.stars[0].get_label()), ch.query_hip(input.stars[1].get_label()),
        ch.query_hip(input.stars[2].get_label())};
    
    EXPECT_ANY_THROW(a.experiment_first_alignment(ch.nearby_bright_stars(focus, 20, 100), c, d));
    
    Star::list f = a.experiment_first_alignment(ch.nearby_bright_stars(focus, 20, 100), c, b);
    EXPECT_THAT(f, Contains(Star::define_label(b[0], c[0].get_label())));
    EXPECT_THAT(f, Contains(Star::define_label(b[1], c[1].get_label())));
    EXPECT_THAT(f, Contains(Star::define_label(b[2], c[2].get_label())));
}

/// Check that a clean input returns the correct stars from a set of candidates.
TEST(PlaneTrial, CleanReduction) {
    std::random_device seed;
    Chomp ch;
    Plane::Parameters p = Plane::DEFAULT_PARAMETERS;
    p.sigma_query = 10e-10;
    p.sigma_overlay = 0.0001;
    Benchmark input(ch, seed, 15);
    Plane a(input, p);
    Identification::labels_list ell = {input.stars[0].get_label(), input.stars[1].get_label(),
        input.stars[2].get_label()};
    
    std::sort(ell.begin(), ell.end());
    EXPECT_THAT(a.experiment_reduction(), UnorderedElementsAre(ell[0], ell[1], ell[2]));
}

/// Check that a clean input returns the expected alignment of stars.
TEST(PlaneTrial, CleanAlignment) {
    std::random_device seed;
    Chomp ch;
    Rotation q = Rotation::chance(seed);
    Star focus = Star::chance(seed);
    unsigned int nu;
    Plane::Parameters p = Plane::DEFAULT_PARAMETERS;
    p.sigma_query = 10e-9;
    p.sigma_overlay = 0.000001;
    p.nu = std::make_shared<unsigned int>(nu);
    Benchmark input(ch, seed, focus, q, 15, 6.0);
    
    Star::list b = {Rotation::rotate(input.stars[0], q), Rotation::rotate(input.stars[1], q),
        Rotation::rotate(input.stars[2], q)};
    Star::list c = {ch.query_hip(input.stars[0].get_label()), ch.query_hip(input.stars[1].get_label()),
        ch.query_hip(input.stars[2].get_label())};
    
    Plane a(Benchmark(seed, b, Rotation::rotate(focus, q), 20), p);
    Star::list f = a.experiment_alignment();
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