/// @file test-angle.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestAngle class, as well as the main function to run the tests.

#include "identification/test-angle.h"

/// Check that query_for_pair method returns the catalog ID of the correct stars.
///
/// @return 0 when finished.
int TestAngle::test_pair_query () {
    Chomp ch;
    std::random_device seed;
    Benchmark input(ch, seed, 15);
    
    double a = Star::angle_between(input.stars[0], input.stars[1]);
    std::array<int, 2> b = Angle(input, Angle::Parameters()).query_for_pair(a);
    
    assert_inside(b[0], {input.stars[0].get_label(), input.stars[1].get_label()}, "QueryPairInsideInputStar0",
                  std::to_string(b[0]) + "," + std::to_string(input.stars[0].get_label()) + ","
                  + std::to_string(input.stars[1].get_label()));
    
    assert_inside(b[1], {input.stars[0].get_label(), input.stars[1].get_label()}, "QueryPairInsideInputStar1",
                  std::to_string(b[1]) + "," + std::to_string(input.stars[0].get_label()) + ","
                  + std::to_string(input.stars[1].get_label()));
    return 0;
}

/// Check that a theta and epsilon with three choices returns the catalog ID of the correct stars.
///
/// @return 0 when finished.
int TestAngle::test_pair_multiple_choice_query () {
    Chomp ch;
    std::random_device seed;
    Angle a(Benchmark(ch, seed, 15), Angle::Parameters());
    Star b = ch.query_hip(103215), c = ch.query_hip(103217);
    a.parameters.query_sigma = 10e-7;
    
    std::array<int, 2> d = a.query_for_pair(Star::angle_between(b, c));
    assert_inside(d[0], {103215, 103217}, "QueryPairMultipleChoicesStar0", std::to_string(d[0]) + ",103215, 103217");
    return 0 * assert_inside(d[1], {103215, 103217}, "QueryPairMultipleChoicesStar1",
                             std::to_string(d[1]) + ",103215, 103217");
}

/// Check that the zero-length stars are returned wgn theta is greater than the current fov.
///
/// @return 0 when finished.
int TestAngle::test_candidate_fov_query () {
    Chomp ch;
    std::random_device seed;
    Angle a(Benchmark(ch, seed, 10), Angle::Parameters());
    Star b(0.928454687492219, 0.132930961972911, 0.346844709665121);
    Star c(0.998078771188383, -0.0350062881876723, 0.0511207031486225);
    
    std::array<Star, 2> d = a.find_candidate_pair(b, c);
    assert_equal(d[0], Star(0, 0, 0), "Candidate0OutOfFOV", d[0].str() + "," + Star(0, 0, 0).str());
    return 0 * assert_equal(d[1], Star(0, 0, 0), "Candidate1OutOfFOV", d[1].str() + "," + Star(0, 0, 0).str());
}

/// Check that the zero-length stars are returned when no matching theta is found.
///
/// @return 0 when finished.
int TestAngle::test_candidate_none_query () {
    Chomp ch;
    std::random_device seed;
    Angle a(Benchmark(ch, seed, 10), Angle::Parameters());
    
    std::array<Star, 2> b = a.find_candidate_pair(Star(1, 1, 1), Star(1.1, 1, 1));
    assert_equal(b[0], Star(0, 0, 0), "Candidate0NoMatchingPair", b[0].str() + "," + Star(0, 0, 0).str());
    return 0 * assert_equal(b[1], Star(0, 0, 0), "Candidate1NoMatchingPair", b[1].str() + "," + Star(0, 0, 0).str());
}

/// Check that the correct stars are returned from the candidate pair query.
///
/// @return 0 when finished.
int TestAngle::test_candidate_results_query () {
    Chomp ch;
    std::random_device seed;
    Benchmark input(ch, seed, 15);
    Angle b(input, Angle::Parameters());
    
    std::array<Star, 2> c = b.find_candidate_pair(input.stars[0], input.stars[1]);
    assert_inside(c[0].get_label(), {input.stars[0].get_label(), input.stars[1].get_label()}, "CandidateMatchingStar0",
                  std::to_string(c[0].get_label()) + "," + std::to_string(input.stars[0].get_label()) + ","
                  + std::to_string(input.stars[1].get_label()));
    assert_inside(c[1].get_label(), {input.stars[0].get_label(), input.stars[1].get_label()}, "CandidateMatchingStar1",
                  std::to_string(c[1].get_label()) + "," + std::to_string(input.stars[0].get_label()) + ","
                  + std::to_string(input.stars[1].get_label()));
    return 0;
}

/// Check that the rotating match method marks the all stars as matched.
///
/// @return 0 when finished.
int TestAngle::test_rotating_match_correct_input () {
    Chomp ch;
    std::random_device seed;
    Star a = Star::chance(seed), b = Star::chance(seed);
    Rotation c = Rotation::chance(seed);
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(ch, seed, Star::chance(seed), c, 8);
    std::vector<Star> rev_input;
    Angle g(input, Angle::Parameters());
    
    // Reverse all input by inverse rotation matrix.
    rev_input.reserve(input.stars.size());
    for (Star rotated : input.stars) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    std::vector<Star> h = g.find_matches(rev_input, c);
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
int TestAngle::test_rotating_match_error_input () {
    Chomp ch;
    std::random_device seed;
    Star a = Star::chance(seed), b = Star::chance(seed);
    Rotation c = Rotation::chance(seed);
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(ch, seed, Star::chance(seed), c, 8);
    std::vector<Star> rev_input;
    Angle g(input, Angle::Parameters());
    
    // Reverse all input by inverse rotation matrix.
    rev_input.reserve(input.stars.size());
    for (Star rotated : input.stars) {
        rev_input.push_back(Rotation::rotate(rotated, f));
    }
    
    // Append focus as error.
    rev_input.push_back(input.focus);
    
    std::vector<Star> h = g.find_matches(rev_input, c);
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
int TestAngle::test_rotating_match_duplicate_input () {
    Chomp ch;
    std::random_device seed;
    Star a = Star::chance(seed), b = Star::chance(seed);
    Rotation c = Rotation::chance(seed);
    Star d = Rotation::rotate(a, c), e = Rotation::rotate(b, c);
    Rotation f = Rotation::rotation_across_frames({a, b}, {d, e});
    Benchmark input(ch, seed, Star::chance(seed), c, 8);
    std::vector<Star> rev_input;
    Angle g(input, Angle::Parameters());
    
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
int TestAngle::test_identify_clean_input () {
    Chomp ch;
    std::random_device seed;
    Benchmark input(ch, seed, 8);
    Angle::Parameters a;
    
    // We define a match as 66% here.
    a.match_minimum = (unsigned int) (input.stars.size() * (2.0 / 3.0));
    
    std::vector<Star> c = Angle::identify(input, a);
    assert_greater_than(c.size(), input.stars.size() * (2.0 / 3.0), "IdentificationFoundAllSize");
    
    std::string all_input = "";
    for (const Star &s : input.stars) {
        all_input += !(s == input.stars[input.stars.size() - 1]) ? s.str() + "," : s.str();
    }
    
    for (unsigned int q = 0; q < c.size() - 1; q++) {
        auto match = [&c, q] (const Star &b) -> bool {
            return b.get_label() == c[q].get_label();
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
int TestAngle::test_identify_error_input () {
    Chomp ch;
    std::random_device seed;
    Benchmark input(ch, seed, 9);
    Angle::Parameters a;
    input.add_extra_light(1);
    
    // We define a match as 66% here.
    a.match_minimum = (unsigned int) ((input.stars.size() - 1) * (2.0 / 3.0));
    
    std::vector<Star> c = Angle::identify(input, a);
    assert_greater_than(c.size(), (input.stars.size() - 1) * (2.0 / 3.0), "IdentificationFoundWithErrorSize");
    
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
    
    return 0;
}

/// Check that a match minimum higher than the input size will default to just the size of the input itself.
/// 
/// @return 0 when finished. 
int TestAngle::test_saturation_match () {
    Chomp ch;
    std::random_device seed;
    Benchmark input(ch, seed, 15);
    Angle::Parameters p;
    
    // Some ridiculous number...
    p.match_minimum = 10000;
    
    std::vector<Star> a = Angle::identify(input, p);
    return 0 * assert_not_equal(a.size(), 0, "StarsFoundNotEmpty");
}

/// Enumerate all tests in TestAngle.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestAngle::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_pair_query();
        case 1: return test_pair_multiple_choice_query();
        case 2: return test_candidate_fov_query();
        case 3: return test_candidate_none_query();
        case 4: return test_candidate_results_query();
        case 5: return test_rotating_match_correct_input();
        case 6: return test_rotating_match_error_input();
        case 7: return test_rotating_match_duplicate_input();
        case 8: return test_identify_clean_input();
        case 9: return test_identify_error_input();
        case 10: return test_saturation_match();
        default: return -1;
    }
}

/// Run the tests in TestAngle. Currently set to log all results. 
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    //    Angle::generate_sep_table(20, "SEP20");
    return TestAngle().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
