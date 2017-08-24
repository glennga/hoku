/// @file test-planar-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestPlanarTriangle class, as well as the main function to run the tests.

#include "test-planar-triangle.h"

/// Check that query_for_trio method returns the BSC ID of the correct stars.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_trio_query () {
    Benchmark input(15, Star::chance(), Rotation::chance());
    Plane p(input, Plane::Parameters());
    
    double a = Trio::planar_area(input.stars[0], input.stars[1], input.stars[2]);
    double b = Trio::planar_moment(input.stars[0], input.stars[1], input.stars[2]);
    std::vector<Plane::hr_trio> c = p.query_for_trio(a, b);
    std::array<bool, 3> matched = {false, false, false};
    
    // Check that original input trio exists in search.
    for (const Plane::hr_trio &t : c) {
        for (int i = 0; i < 3; i++) {
            if (input.stars[i].get_hr() == t[0] || input.stars[i].get_hr() == t[1] || input.stars[i].get_hr() == t[2]) {
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
    Plane a(Benchmark(10, Star::chance(), Rotation::chance()), Plane::Parameters());
    a.input[0] = Star(a.input[0][0], a.input[0][1], a.input[0][2], 3);
    a.input[1] = Star(a.input[1][0], a.input[1][1], a.input[1][2], 4);
    a.input[2] = Star(a.input[2][0], a.input[2][1], a.input[2][2], 5);
    
    std::vector<Trio::stars> b = a.match_stars({0, 1, 2});
    return 0 * assert_true(b[0][0] == Star() && b[0][1] == Star() && b[0][2] == Star(), "CandidateOutOfFOV");
}

/// Check that the zero-length stars are returned when no matching trio is found.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_match_stars_none () {
    Plane a(Benchmark(10, Star::chance(), Rotation::chance()), Plane::Parameters());
    a.input[0] = Star(1, 1, 1.1);
    a.input[1] = Star(1, 1, 1);
    a.input[2] = Star(1.1, 1, 1);
    
    std::vector<Trio::stars> b = a.match_stars({0, 1, 2});
    return 0 * assert_true(b[0][0] == Star() && b[0][1] == Star() && b[0][2] == Star(), "CandidateNoMatchingPair");
}

/// Check that the correct stars are returned from the candidate trio query.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_match_stars_results () {
    Plane a(Benchmark(15, Star::chance(), Rotation::chance()), Plane::Parameters());
    a.input[0] = a.ch.query_bsc5(3898);
    a.input[1] = a.ch.query_bsc5(4325);
    a.input[2] = a.ch.query_bsc5(4502);
    
    std::vector<Trio::stars> b = a.match_stars({0, 1, 2});
    std::array<bool, 3> matched = {false, false, false};
    
    // Check that original input trio exists in search.
    for (const Trio::stars &t : b) {
        for (int i = 0; i < 3; i++) {
            if (a.input[i] == t[0] || a.input[i] == t[1] || a.input[i] == t[2]) {
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
    Plane a(Benchmark(15, Star::chance(), Rotation::chance()), Plane::Parameters());
    a.input[0] = a.ch.query_bsc5(3898);
    a.input[1] = a.ch.query_bsc5(4325);
    a.input[2] = a.ch.query_bsc5(4502);
    
    std::vector<Trio::stars> b = a.match_stars({0, 1, 2});
    Trio::stars c = a.pivot({0, 1, 2}, b);
    assert_true(c[0] == a.input[0] || c[0] == a.input[1] || c[0] == a.input[2], "CandidateMatchingStarPivotQueryStar0");
    assert_true(c[1] == a.input[0] || c[1] == a.input[1] || c[1] == a.input[2], "CandidateMatchingStarPivotQueryStar1");
    assert_true(c[2] == a.input[0] || c[2] == a.input[1] || c[2] == a.input[2], "CandidateMatchingStarPivotQueryStar2");
    
    return 0;
}

/// Check that the rotating match method marks the all stars as matched.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_rotating_match_correct_input () {
    Star a = Star::chance(), b = Star::chance();
    Rotation c = Rotation::chance();
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(8, Star::chance(), c);
    std::vector<Star> rev_input;
    Plane g(input, Plane::Parameters());
    
    // Reverse all input by inverse rotation.
    rev_input.reserve(input.stars.size());
    for (Star rotated : input.stars) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    std::vector<Star> h = g.rotate_stars(rev_input, c);
    assert_equal(h.size(), input.stars.size(), "RotatingMatchAllInputReturned");
    
    for (unsigned int q = 0; q < h.size(); q++) {
        std::string test_name = "RotatingMatchInputStar" + std::to_string(q + 1);
        assert_equal(h[q].get_hr(), input.stars[q].get_hr(), test_name);
    }
    
    return 0;
}

/// Check that the rotating match method marks only the correct stars as matched.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_rotating_match_error_input () {
    Star a = Star::chance(), b = Star::chance();
    Rotation c = Rotation::chance();
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(8, Star::chance(), c);
    std::vector<Star> rev_input;
    Plane g(input, Plane::Parameters());
    
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
        assert_equal(h[q].get_hr(), input.stars[q].get_hr(), test_name);
    }
    
    return 0;
}

/// Check that the rotating match method marks only the correct stars as matched, not the duplicate as well.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_rotating_match_duplicate_input () {
    Star a = Star::chance(), b = Star::chance();
    Rotation c = Rotation::chance();
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(8, Star::chance(), c);
    std::vector<Star> rev_input;
    Plane g(input, Plane::Parameters());
    
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
        assert_equal(h[q].get_hr(), input.stars[q].get_hr(), test_name);
    }
    
    return 0;
}

/// Check that correct result is returned with a clean input.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_identify_clean_input () {
    Benchmark input(8, Star::chance(), Rotation::chance());
    Plane::Parameters a;
    
    // we define a match as 66% here
    a.match_minimum = (unsigned int) (input.stars.size() / 3.0);
    
    std::vector<Star> c = Plane(input, a).identify();
    assert_equal(c.size(), input.stars.size(), "IdentificationFoundAllSize");
    
    std::string all_input = "";
    for (const Star &s : input.stars) {
        all_input += !(s == input.stars[input.stars.size() - 1]) ? s.str() + "," : s.str();
    }
    
    for (unsigned int q = 0; q < c.size() - 1; q++) {
        auto match = [c, q] (const Star &b) -> bool {
            return b.get_hr() == c[q].get_hr();
        };
        auto is_found = std::find_if(input.stars.begin(), input.stars.end(), match);
        
        std::string test_name = "IdentificationCleanInputStar" + std::to_string(q + 1);
        assert_true(is_found != input.stars.end(), test_name, c[q].str() + "," + all_input);
    }
    
    return 0;
}

/// Check that correct result is returned with an error input.
///
/// @return 0 when finished.
int TestPlanarTriangle::test_identify_error_input () {
    Benchmark input(9, Star::chance(), Rotation::chance());
    Plane::Parameters a;
    input.add_extra_light(1);
    
    // We define a match as 66% here.
    a.match_minimum = (unsigned int) ((input.stars.size() - 1) / 3.0);
    
    std::vector<Star> c = Plane(input, a).identify();
    assert_equal(c.size(), input.stars.size() - 1, "IdentificationFoundWithErrorSize");
    
    std::string all_input = "";
    for (const Star &s : input.stars) {
        all_input += !(s == input.stars[input.stars.size() - 1]) ? s.str() + "," : s.str();
    }
    
    for (unsigned int q = 0; q < c.size() - 1; q++) {
        auto match = [c, q] (const Star &b) -> bool {
            return b.get_hr() == c[q].get_hr();
        };
        auto is_found = std::find_if(input.stars.begin(), input.stars.end(), match);
        
        std::string test_name = "IdentificationErrorInputStar" + std::to_string(q + 1);
        assert_true(is_found != input.stars.end(), test_name, c[q].str() + "," + all_input);
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
    //    PlanarTriangle::generate_triangle_table(20, "PLAN20");
    //    Chomp::create_k_vector("PLAN20", "a");
    //    Nibble::polish_table("PLAN20_KVEC", "k_value");
    return TestPlanarTriangle().execute_tests();
}
