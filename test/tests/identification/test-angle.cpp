/// @file test-angle.cpp
/// @author Glenn Galvizo
///
/// Source file for all Angle class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "identification/angle.h"
#include "gmock/gmock.h"

// Import several matchers from Google Mock.
using testing::Contains;
using testing::Not;

/// Check that query_for_pair method returns the catalog ID of the correct stars.
TEST(AngleQuery, Pair) {
    Chomp ch;
    std::random_device seed;
    Benchmark input(ch, seed, 15);
    
    double a = Star::angle_between(input.stars[0], input.stars[1]);
    Identification::labels_list b = Angle(input, Angle::DEFAULT_PARAMETERS).query_for_pair(a);
    std::vector<int> c = {input.stars[0].get_label(), input.stars[1].get_label()};
    std::vector<int> d = {input.stars[0].get_label(), input.stars[1].get_label()};
    EXPECT_THAT(c, Contains(b[0]));
    EXPECT_THAT(d, Contains(b[1]));
}

/// Check that a theta and epsilon with three choices returns the catalog ID of the correct stars.
TEST(AngleQuery, MultipleChoice) {
    Chomp ch;
    std::random_device seed;
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.sigma_query = 10e-7;
    Angle a(Benchmark(ch, seed, 15), p);
    Star b = ch.query_hip(103215), c = ch.query_hip(103217);
    Identification::labels_list d = a.query_for_pair(Star::angle_between(b, c));
    
    std::vector<int> e = {103215, 103217}, f = {103215, 103217};
    EXPECT_THAT(e, Contains(d[0]));
    EXPECT_THAT(f, Contains(d[1]));
}

/// Check that the zero-length stars are returned wgn theta is greater than the current fov.
TEST(AngleQuery, FOV) {
    Chomp ch;
    std::random_device seed;
    Angle a(Benchmark(ch, seed, 10), Angle::DEFAULT_PARAMETERS);
    Star b(0.928454687492219, 0.132930961972911, 0.346844709665121);
    Star c(0.998078771188383, -0.0350062881876723, 0.0511207031486225);
    
    std::array<Star, 2> d = a.find_candidate_pair(b, c);
    EXPECT_EQ(d[0], Star::zero());
    EXPECT_EQ(d[1], Star::zero());
}

/// Check that the zero-length stars are returned when no matching theta is found.
TEST(AngleQuery, None) {
    Chomp ch;
    std::random_device seed;
    Angle a(Benchmark(ch, seed, 10), Angle::DEFAULT_PARAMETERS);
    
    std::array<Star, 2> b = a.find_candidate_pair(Star(1, 1, 1), Star(1.1, 1, 1));
    EXPECT_EQ(b[0], Star::zero());
    EXPECT_EQ(b[1], Star::zero());
}

/// Check that the correct stars are returned from the candidate pair query.
TEST(AngleResults, Query) {
    Chomp ch;
    std::random_device seed;
    Benchmark input(ch, seed, 15);
    Angle b(input, Angle::DEFAULT_PARAMETERS);
    
    std::array<Star, 2> c = b.find_candidate_pair(input.stars[0], input.stars[1]);
    std::vector<int> d = {input.stars[0].get_label(), input.stars[1].get_label()};
    std::vector<int> e = {input.stars[0].get_label(), input.stars[1].get_label()};
    EXPECT_THAT(d, Contains(c[0].get_label()));
    EXPECT_THAT(e, Contains(c[1].get_label()));
}

/// Check that the rotating match method marks the all stars as matched.
TEST(AngleMatch, CorrectInput) {
    Chomp ch;
    std::random_device seed;
    Star a = Star::chance(seed), b = Star::chance(seed);
    Rotation c = Rotation::chance(seed);
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(ch, seed, Star::chance(seed), c, 8);
    std::vector<Star> rev_input;
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.sigma_overlay = 0.000001;
    Angle g(input, p);
    
    // Reverse all input by inverse rotation matrix.
    rev_input.reserve(input.stars.size());
    for (Star rotated : input.stars) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    std::vector<Star> h = g.find_matches(rev_input, c);
    EXPECT_EQ(h.size(), input.stars.size());
    for (unsigned int q = 0; q < h.size(); q++) {
        EXPECT_EQ(h[q].get_label(), input.stars[q].get_label());
    }
}

/// Check that the rotating match method marks only the correct stars as matched.
TEST(AngleMatch, ErrorInput) {
    Chomp ch;
    std::random_device seed;
    Star a = Star::chance(seed), b = Star::chance(seed);
    Rotation c = Rotation::chance(seed);
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(ch, seed, Star::chance(seed), c, 8);
    std::vector<Star> rev_input;
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.sigma_overlay = 0.000001;
    Angle g(input, p);
    
    // Reverse all input by inverse rotation matrix.
    rev_input.reserve(input.stars.size());
    for (Star rotated : input.stars) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    // Append focus as error.
    rev_input.push_back(input.focus);
    std::vector<Star> h = g.find_matches(rev_input, c);
    EXPECT_EQ(h.size(), input.stars.size());
    
    for (unsigned int q = 0; q < h.size(); q++) {
        EXPECT_EQ(h[q].get_label(), input.stars[q].get_label());
    }
}

/// Check that the rotating match method marks only the correct stars as matched, not the duplicate as well.
TEST(AngleMatch, RotatingDuplicateInput) {
    Chomp ch;
    std::random_device seed;
    Star a = Star::chance(seed), b = Star::chance(seed);
    Rotation c = Rotation::chance(seed);
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(ch, seed, Star::chance(seed), c, 8);
    std::vector<Star> rev_input;
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.sigma_overlay = 0.000001;
    Angle g(input, p);
    
    // Reverse all input by inverse rotation matrix.
    rev_input.reserve(input.stars.size());
    for (Star rotated : input.stars) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    // Append first star as error.
    rev_input.push_back(rev_input[0]);
    rev_input.push_back(rev_input[0]);
    rev_input.push_back(rev_input[0]);
    std::vector<Star> h = g.find_matches(rev_input, c);
    EXPECT_EQ(h.size(), input.stars.size());
    
    for (unsigned int q = 0; q < h.size(); q++) {
        EXPECT_EQ(h[q].get_label(), input.stars[q].get_label());
    }
}

/// Check that correct result is returned with a clean input.
TEST(AngleIdentify, CleanInput) {
    Chomp ch;
    std::random_device seed;
    Benchmark input(ch, seed, 10, 6.5);
    Angle::Parameters a = Angle::DEFAULT_PARAMETERS;
    a.sigma_overlay = 0.000001;
    unsigned int nu;
    a.nu = std::make_shared<unsigned int>(nu);
    
    // We define a match as 66% here.
    a.gamma = (unsigned int) (input.stars.size() * (2.0 / 3.0));
    Star::list c = Angle(input, a).experiment_crown();
    EXPECT_GT(c.size(), input.stars.size() * (2.0 / 3.0));
    std::string all_input;
    for (const Star &s : input.stars) {
        all_input += !(s == input.stars[input.stars.size() - 1]) ? s.str() + "," : s.str();
    }
    
    for (unsigned int q = 0; q < c.size() - 1; q++) {
        auto match = [&c, q] (const Star &b) -> bool {
            return b.get_label() == c[q].get_label();
        };
        auto is_found = std::find_if(input.stars.begin(), input.stars.end(), match);
        EXPECT_NE(is_found, input.stars.end());
    }
}

/// Check that correct result is returned with an error input.
TEST(AngleIdentify, ErrorInput) {
    Chomp ch;
    std::random_device seed;
    Benchmark input(ch, seed, 9);
    Angle::Parameters a = Angle::DEFAULT_PARAMETERS;
    a.sigma_overlay = 0.000001;
    input.add_extra_light(1);
    unsigned int nu;
    a.nu = std::make_shared<unsigned int>(nu);
    
    // We define a match as 66% here.
    a.gamma = (unsigned int) ((input.stars.size() - 1) * (2.0 / 3.0));
    std::vector<Star> c = Angle(input, a).experiment_crown();
    EXPECT_GT(c.size(), (input.stars.size() - 1) * (2.0 / 3.0));
    std::string all_input;
    for (const Star &s : input.stars) {
        all_input += !(s == input.stars[input.stars.size() - 1]) ? s.str() + "," : s.str();
    }
    
    for (unsigned int q = 0; q < c.size() - 1; q++) {
        auto match = [&c, q] (const Star &b) -> bool {
            return b.get_label() == c[q].get_label();
        };
        auto is_found = std::find_if(input.stars.begin(), input.stars.end(), match);
        EXPECT_NE(is_found, input.stars.end());
    }
}

/// Check that a match minimum higher than the input size will default to just the size of the input itself.
TEST(AngleIdentify, SaturationMatch) {
    Chomp ch;
    std::random_device seed;
    Benchmark input(ch, seed, 15);
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.sigma_overlay = 0.000001;
    unsigned int nu;
    p.nu = std::make_shared<unsigned int>(nu);
    
    // Some ridiculous number...
    p.gamma = 10000;
    
    Star::list a = Angle(input, p).experiment_crown();
    EXPECT_NE(a.size(), 0);
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