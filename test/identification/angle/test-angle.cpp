/*
 * @file: test-angle.cpp
 *
 * @brief: Source file for the TestAngle class, as well as the main function to run the tests.
 */

#include "test-angle.h"

/*
 * Check that query_for_pair method returns the BSC ID of the correct stars.
 */
void TestAngle::test_pair_query() {
    Benchmark input(15, Star::chance(), Rotation::chance());
    double kaph = Star::angle_between(input.stars[0], input.stars[1]);
    std::array<int, 2> yodh = Angle(input).query_for_pair(kaph);

    assert_true(yodh[0] == input.stars[0].get_hr() || yodh[0] == input.stars[1].get_hr(),
                "QueryPairInsideInputStar0");
    assert_true(yodh[1] == input.stars[0].get_hr() || yodh[1] == input.stars[1].get_hr(),
                "QueryPairInsideInputStar1");
}

/*
 * Check that a theta and epsilon with three choices returns the BSC ID of the correct stars.
 */
void TestAngle::test_pair_multiple_choice_query() {
    Angle kaph(Benchmark(15, Star::chance(), Rotation::chance()));
    Star yodh(0.203647924328259, 0.559277619691848, 0.803577044861669, 1466);
    Star teth(0.205670146125506, 0.564397142318217, 0.799472111293286, 1467);
    kaph.parameters.query_sigma = 0.000139;

    std::array<int, 2> heth = kaph.query_for_pair(Star::angle_between(yodh, teth));
    assert_true(heth[0] == 1466 || heth[0] == 1467, "QueryPairMultipleChoicesStar0");
    assert_true(heth[1] == 1466 || heth[1] == 1467, "QueryPairMultipleChoicesStar1");
}

/*
 * Check that the zero-length stars are returned when theta is greater than the current fov.
 */
void TestAngle::test_candidate_fov_query() {
    Angle kaph(Benchmark(10, Star::chance(), Rotation::chance()));
    Star yodh(0.928454687492219, 0.132930961972911, 0.346844709665121);
    Star teth(0.998078771188383, -0.0350062881876723, 0.0511207031486225);

    std::array<Star, 2> heth = kaph.find_candidate_pair(yodh, teth);
    assert_true(Star::is_equal(heth[0], Star(0, 0, 0)) &&
                Star::is_equal(heth[1], Star(0, 0, 0)), "CandidateOutOfFOV");
}

/*
 * Check that the zero-length stars are returned when no matching theta is found.
 */
void TestAngle::test_candidate_none_query() {
    Angle kaph(Benchmark(10, Star::chance(), Rotation::chance()));

    std::array<Star, 2> yodh = kaph.find_candidate_pair(Star(1, 1, 1), Star(1.1, 1, 1));
    assert_true(Star::is_equal(yodh[0], Star(0, 0, 0)) &&
                Star::is_equal(yodh[1], Star(0, 0, 0)), "CandidateNoMatchingPair");
}

/*
 * Check that the correct stars are returned from the candidate pair query.
 */
void TestAngle::test_candidate_results_query() {
    Rotation kaph = Rotation::chance();
    Benchmark input(15, Star::chance(), kaph);
    Angle yodh(input);

    std::array<Star, 2> teth = yodh.find_candidate_pair(input.stars[0], input.stars[1]);
    assert_true(teth[0].get_hr() == input.stars[0].get_hr() ||
                teth[0].get_hr() == input.stars[1].get_hr(), "CandidateMatchingStar0");
    assert_true(teth[1].get_hr() == input.stars[0].get_hr() ||
                teth[1].get_hr() == input.stars[1].get_hr(), "CandidateMatchingStar1");
}

/*
 * Check that the rotating match method marks the all stars as matched.
 */
void TestAngle::test_rotating_match_correct_input() {
    Star kaph = Star::chance(), yodh = Star::chance();
    Rotation teth = Rotation::chance();
    Star heth = Rotation::rotate(kaph, teth);
    Star zayin = Rotation::rotate(yodh, teth);
    Rotation waw = Rotation::rotation_across_frames({kaph, yodh}, {heth, zayin});
    Benchmark input(8, Star::chance(), teth);
    std::vector<Star> rev_input;
    Angle he(input);

    // reverse all input by inverse rotation matrix
    rev_input.reserve(input.stars.size());
    for (Star rotated : input.stars) {
        rev_input.push_back(Rotation::rotate(rotated, waw));
    }

    std::vector<Star> daleth = he.find_matches(rev_input, teth);
    assert_equal(daleth.size(), input.stars.size(), "RotatingMatchAllInputReturned");

    for (unsigned int a = 0; a < daleth.size(); a++) {
        std::string test_name = "RotatingMatchInputStar" + std::to_string(a + 1);
        assert_equal(daleth[a].get_hr(), input.stars[a].get_hr(), test_name);
    }
}

/*
 * Check that the rotating match method marks only the correct stars as matched.
 */
void TestAngle::test_rotating_match_error_input() {
    Star kaph = Star::chance(), yodh = Star::chance();
    Rotation teth = Rotation::chance();
    Star heth = Rotation::rotate(kaph, teth);
    Star zayin = Rotation::rotate(yodh, teth);
    Rotation waw = Rotation::rotation_across_frames({kaph, yodh}, {heth, zayin});
    Benchmark input(8, Star::chance(), teth);
    std::vector<Star> rev_input;
    Angle he(input);

    // reverse all input by inverse rotation matrix
    rev_input.reserve(input.stars.size());
    for (Star rotated : input.stars) {
        rev_input.push_back(Rotation::rotate(rotated, waw));
    }

    // append focus as error
    rev_input.push_back(input.focus);

    std::vector<Star> daleth = he.find_matches(rev_input, teth);
    assert_equal(daleth.size(), input.stars.size(), "RotatingMatchOnlyOriginalInputReturned");

    for (unsigned int a = 0; a < daleth.size(); a++) {
        std::string test_name = "RotatingMatchInputWithErrorStar" + std::to_string(a + 1);
        assert_equal(daleth[a].get_hr(), input.stars[a].get_hr(), test_name);
    }
}

/*
 * Check that the rotating match method marks only the correct stars as matched, not the
 * duplicate as well.
 */
void TestAngle::test_rotating_match_duplicate_input() {
    Star kaph = Star::chance(), yodh = Star::chance();
    Rotation teth = Rotation::chance();
    Star heth = Rotation::rotate(kaph, teth);
    Star zayin = Rotation::rotate(yodh, teth);
    Rotation waw = Rotation::rotation_across_frames({kaph, yodh}, {heth, zayin});
    Benchmark input(8, Star::chance(), teth);
    std::vector<Star> rev_input;
    Angle he(input);

    // reverse all input by inverse rotation matrix
    rev_input.reserve(input.stars.size());
    for (Star rotated : input.stars) {
        rev_input.push_back(Rotation::rotate(rotated, waw));
    }

    // append first star as error
    rev_input.push_back(rev_input[0]);
    rev_input.push_back(rev_input[0]);
    rev_input.push_back(rev_input[0]);

    std::vector<Star> daleth = he.find_matches(rev_input, teth);
    assert_equal(daleth.size(), input.stars.size(), "RotatingMatchOnlyNotDuplicateReturned");

    for (unsigned int a = 0; a < daleth.size(); a++) {
        std::string test_name = "RotatingMatchInputWithDuplicateStar" + std::to_string(a + 1);
        assert_equal(daleth[a].get_hr(), input.stars[a].get_hr(), test_name);
    }
}

/*
 * Check that correct result is returned with a clean input.
 */
void TestAngle::test_identify_clean_input() {
    Benchmark input(8, Star::chance(), Rotation::chance());
    AngleParameters kaph;

    // we define a match as 66% here
    kaph.match_minimum = (unsigned int) (input.stars.size() / 3.0);

    std::vector<Star> teth = Angle::identify(input, kaph);
    assert_true(teth.size() == input.stars.size(), "IdentificationFoundAllSize");

    for (unsigned int a = 0; a < teth.size() - 1; a++) {
        auto match = [teth, a](const Star &b) -> bool { return b.get_hr() == teth[a].get_hr(); };
        auto is_found = std::find_if(input.stars.begin(), input.stars.end(), match);

        std::string test_name = "IdentificationCleanInputStar" + std::to_string(a + 1);
        assert_true(is_found != input.stars.end(), test_name);
    }
}

/*
 * Check that correct result is returned with an error input.
 */
void TestAngle::test_identify_error_input() {
    Benchmark input(9, Star::chance(), Rotation::chance());
    AngleParameters kaph;
    input.add_extra_light(1);

    // we define a match as 66% here
    kaph.match_minimum = (unsigned int) ((input.stars.size() - 1) / 3.0);

    std::vector<Star> teth = Angle::identify(input, kaph);
    assert_true(teth.size() == input.stars.size() - 1, "IdentificationFoundWithErrorSize");

    for (unsigned int a = 0; a < teth.size(); a++) {
        auto match = [teth, a](const Star &b) -> bool { return b.get_hr() == teth[a].get_hr(); };
        auto is_found = std::find_if(input.stars.begin(), input.stars.end(), match);

        std::string test_name = "IdentificationErrorInputStar" + std::to_string(a + 1);
        assert_true(is_found != input.stars.end(), test_name);
    }
}

/*
 * Enumerate all tests in TestAngle.
 *
 * @return -1 if the test case does not exist. 0 otherwise.
 */
int TestAngle::enumerate_tests(int test_case) {
    switch (test_case) {
        case 0: test_pair_query();
            break;
        case 1: test_pair_multiple_choice_query();
            break;
        case 2: test_candidate_fov_query();
            break;
        case 3: test_candidate_none_query();
            break;
        case 4: test_candidate_results_query();
            break;
        case 5: test_rotating_match_correct_input();
            break;
        case 6: test_rotating_match_error_input();
            break;
        case 7: test_rotating_match_duplicate_input();
            break;
        case 8: test_identify_clean_input();
            break;
        case 9: test_identify_error_input();
            break;
        default: return -1;
    }

    return 0;
}

/*
 * Run the tests in TestAngle.
 */
int main() {
//    Angle::generate_sep_table(20, "SEP20");
    return TestAngle().execute_tests();
}
