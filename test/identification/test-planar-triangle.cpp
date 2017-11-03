/// @file test-planar-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestPlanarTriangle class, as well as the main function to run the tests.

#include "identification/test-planar-triangle.h"

/// Check that query_for_trio method returns the catalog ID of the correct stars.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_trio_query () {
    std::random_device seed;
    Chomp ch;
    
    Benchmark input(ch, seed, 15);
    Plane::Parameters par;
    par.table_name = "PLANE_20";
    Plane p(input, par);
    
    double a = Trio::planar_area(input.stars[0], input.stars[1], input.stars[2]);
    double b = Trio::planar_moment(input.stars[0], input.stars[1], input.stars[2]);
    std::vector<Plane::label_trio> c = p.query_for_trio(a, b);
    std::array<bool, 3> matched = {false, false, false};
    
    // Check that original input trio exists in search.
    for (const Plane::label_trio &t : c) {
        for (int i = 0; i < 3; i++) {
            if (input.stars[i].get_label() == t[0] || input.stars[i].get_label() == t[1]
                || input.stars[i].get_label() == t[2]) {
                matched[i] = true;
            }
        }
    }
    
    assert_true(matched[0], "QueryTrioInsideInputStar0");
    assert_true(matched[1], "QueryTrioInsideInputStar1");
    return 0 * assert_true(matched[2], "QueryTrioInsideInputStar2");
}

/// Check that the zero-length stars are returned when a theta between a pair of stars is greater than the current fov.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_match_stars_fov () {
    Plane::Parameters par;
    par.table_name = "PLANE_20";
    std::random_device seed;
    Chomp ch;
    
    Plane a(Benchmark(ch, seed, 10), par);
    a.input[0] = Star::reset_label(ch.query_hip(3));
    a.input[1] = Star::reset_label(ch.query_hip(4));
    a.input[2] = Star::reset_label(ch.query_hip(5));
    
    std::vector<Trio::stars> b = a.match_stars({0, 1, 2});
    return 0 * assert_true(std::all_of(b[0].begin(), b[0].end(), [] (const Star &s) -> bool {
        return s == Star::zero();
    }), "CandidateOutOfFOV", b[0][0].str() + "," + b[0][1].str() + "," + b[0][2].str());
}

/// Check that the zero-length stars are returned when no matching trio is found.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_match_stars_none () {
    Plane::Parameters par;
    par.table_name = "PLANE_20";
    par.sigma_a = std::numeric_limits<double>::epsilon();
    std::random_device seed;
    Chomp ch;
    
    Plane a(Benchmark(ch, seed, 10), par);
    a.input[0] = Star(1, 1, 1.1);
    a.input[1] = Star(1, 1, 1);
    a.input[2] = Star(1.1, 1, 1);
    
    std::vector<Trio::stars> b = a.match_stars({0, 1, 2});
    return 0 * assert_true(std::all_of(b[0].begin(), b[0].end(), [] (const Star &s) -> bool {
        return s == Star::zero();
    }), "CandidateNoMatchingPair", b[0][0].str() + "," + b[0][1].str() + "," + b[0][2].str());
}

/// Check that the correct stars are returned from the candidate trio query.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_match_stars_results () {
    Plane::Parameters par;
    par.table_name = "PLANE_20";
    std::random_device seed;
    Chomp ch;
    
    Benchmark input(ch, seed, 20);
    Plane a(input, par);
    std::vector<Trio::stars> b = a.match_stars({0, 1, 2});
    std::array<bool, 3> matched = {false, false, false};
    
    // Check that original input trio exists in search.
    for (const Trio::stars &t : b) {
        for (int i = 0; i < 3; i++) {
            if (input.stars[i].get_label() == t[0].get_label() || input.stars[i].get_label() == t[1].get_label()
                || input.stars[i].get_label() == t[2].get_label()) {
                matched[i] = true;
            }
        }
    }
    
    assert_true(matched[0], "CandidateMatchingStar0");
    assert_true(matched[1], "CandidateMatchingStar1");
    return 0 * assert_true(matched[2], "CandidateMatchingStar2");
}

/// Check that the pivot query method returns the correct trio.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_pivot_query_results () {
    Plane::Parameters par;
    par.table_name = "PLANE_20";
    std::random_device seed;
    Chomp ch;
    
    Benchmark input(ch, seed, 20);
    Plane a(input, par);
    
    Trio::stars c = a.pivot({0, 1, 2});
    assert_true(c[0].get_label() == input.stars[0].get_label() || c[0].get_label() == input.stars[1].get_label()
                || c[0].get_label() == input.stars[2].get_label(), "CandidateMatchingStarPivotQueryStar0");
    assert_true(c[1].get_label() == input.stars[0].get_label() || c[1].get_label() == input.stars[1].get_label()
                || c[1].get_label() == input.stars[2].get_label(), "CandidateMatchingStarPivotQueryStar0");
    assert_true(c[2].get_label() == input.stars[0].get_label() || c[2].get_label() == input.stars[1].get_label()
                || c[2].get_label() == input.stars[2].get_label(), "CandidateMatchingStarPivotQueryStar0");
    
    return 0;
}

/// Check that the rotating match method marks the all stars as matched.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_rotating_match_correct_input () {
    Plane::Parameters par;
    std::random_device seed;
    Chomp ch;
    
    Star a = Star::chance(seed), b = Star::chance(seed);
    Rotation c = Rotation::chance(seed);
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(ch, seed, Star::chance(seed), c, 8);
    std::vector<Star> rev_input;
    
    par.table_name = "PLANE_20";
    Plane g(input, par);
    
    // Reverse all input by inverse rotation.
    rev_input.reserve(input.stars.size());
    for (Star rotated : input.stars) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    std::vector<Star> h = g.rotate_stars(rev_input, c);
    assert_equal(h.size(), input.stars.size(), "RotatingMatchAllInputReturned");
    
    for (unsigned int q = 0; q < h.size(); q++) {
        std::string test_name = "RotatingMatchInputStar" + std::to_string(q + 1);
        assert_equal(h[q].get_label(), input.stars[q].get_label(), test_name);
    }
    
    return 0;
}

/// Check that the rotating match method marks only the correct stars as matched.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_rotating_match_error_input () {
    Plane::Parameters par;
    std::random_device seed;
    Chomp ch;
    
    Star a = Star::chance(seed), b = Star::chance(seed);
    Rotation c = Rotation::chance(seed);
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(ch, seed, Star::chance(seed), c, 8);
    std::vector<Star> rev_input;
    
    par.table_name = "PLANE_20";
    Plane g(input, par);
    
    // Reverse all input by inverse rotation.
    rev_input.reserve(input.stars.size());
    for (Star rotated : input.stars) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    // Append focus as error.
    rev_input.push_back(input.focus);
    
    std::vector<Star> h = g.rotate_stars(rev_input, c);
    assert_equal(h.size(), input.stars.size(), "RotatingMatchOnlyOriginalInputReturned");
    
    for (unsigned int q = 0; q < h.size(); q++) {
        std::string test_name = "RotatingMatchInputWithStar" + std::to_string(q + 1);
        assert_equal(h[q].get_label(), input.stars[q].get_label(), test_name);
    }
    
    return 0;
}

/// Check that the rotating match method marks only the correct stars as matched, not the duplicate as well.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_rotating_match_duplicate_input () {
    Plane::Parameters par;
    std::random_device seed;
    Chomp ch;
    
    Star a = Star::chance(seed), b = Star::chance(seed);
    Rotation c = Rotation::chance(seed);
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(ch, seed, Star::chance(seed), c, 8);
    std::vector<Star> rev_input;
    
    par.table_name = "PLANE_20";
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
    
    std::vector<Star> h = g.rotate_stars(rev_input, c);
    assert_equal(h.size(), input.stars.size(), "RotatingMatchOnlyNotDuplicateReturned");
    
    for (unsigned int q = 0; q < h.size(); q++) {
        std::string test_name = "RotatingMatchInputWithDuplicateStar" + std::to_string(q + 1);
        assert_equal(h[q].get_label(), input.stars[q].get_label(), test_name);
    }
    
    return 0;
}

/// Check that correct result is returned with a clean input.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_identify_clean_input () {
    std::random_device seed;
    Chomp ch;
    
    Benchmark input(ch, seed, 8, 6.5);
    Plane::Parameters a;
    
    // We define a match as 66% here.
    a.match_minimum = (unsigned int) (input.stars.size() * (2.0 / 3.0));
    a.table_name = "PLANE_20";
    
    std::vector<Star> c = Plane::identify(input, a);
    assert_greater_than(c.size(), input.stars.size() * (2.0 / 3.0), "IdentificationFoundAllSize");
    
    std::string all_input = "";
    for (const Star &s : input.stars) {
        all_input += !(s == input.stars[input.stars.size() - 1]) ? s.str() + "," : s.str();
    }
    
    if (!c.empty()) {
        for (int q = 0; q < (signed) (c.size() - 1); q++) {
            auto match = [&c, q] (const Star &b) -> bool {
                return b.get_label() == c[q].get_label();
            };
            auto is_found = std::find_if(input.stars.begin(), input.stars.end(), match);
            
            std::string test_name = "IdentificationCleanInputStar" + std::to_string(q + 1);
            assert_true(is_found != input.stars.end(), test_name, c[q].str() + "," + all_input);
        }
    }
    
    return 0;
}

/// Check **a** correct result is returned with an error input.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_identify_error_input () {
    std::random_device seed;
    Chomp ch;
    
    Benchmark input(ch, seed, 20);
    Plane::Parameters a;
    input.add_extra_light(1);
    
    // We define a match as 5 stars here.
    a.match_minimum = 5;
    a.match_sigma = 0.0001;
    a.table_name = "PLANE_20";
    
    std::vector<Star> c = Plane::identify(input, a);
    assert_greater_than(c.size(), a.match_minimum, "IdentificationFoundWithErrorSize");
    
    if (!c.empty()) {
        std::string all_input = "";
        for (const Star &s : input.stars) {
            all_input += !(s == input.stars[input.stars.size() - 1]) ? s.str() + "," : s.str();
        }
        
        for (unsigned int q = 0; q < c.size() - 1; q++) {
            auto match = [&c, q] (const Star &b) -> bool {
                return b.get_label() == c[q].get_label();
            };
            auto is_found = std::find_if(input.stars.begin(), input.stars.end(), match);
            
            std::string test_name = "IdentificationErrorInputStar" + std::to_string(q + 1);
            assert_true(is_found != input.stars.end(), test_name, c[q].str() + "," + all_input);
        }
    }
    
    return 0;
}

/// Enumerate all tests in TestPlanarTriangle.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestPlanarTriangle::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_trio_query();
        case 1: return test_match_stars_fov();
        case 2: return test_match_stars_none();
        case 3: return test_match_stars_results();
        case 4: return test_pivot_query_results();
        case 5: return test_rotating_match_correct_input();
        case 6: return test_rotating_match_error_input();
        case 7: return test_rotating_match_duplicate_input();
        case 8: return test_identify_clean_input();
        case 9: return test_identify_error_input();
        default: return -1;
    }
}

/// Run the tests in TestPlanarTriangle. Currently set to log all results. 
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    return TestPlanarTriangle().execute_tests(BaseTest::Flavor::FULL_PRINT_LOG_ON);
}
