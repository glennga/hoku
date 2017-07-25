/*
 * @file: test-triangle-planar.cpp
 *
 * @brief: Source file for the TestTrianglePlanar class, as well as the main function to run the
 * tests.
 */

#include "test-planar-triangle.h"

/*
 * Check that query_for_trio method returns the BSC ID of the correct stars.
 */
void TestPlanarTriangle::test_trio_query() {
    Benchmark input(15, Star::chance(), Rotation::chance());
    Plane p(input);

    double kaph = Trio::planar_area(input.stars[0], input.stars[1], input.stars[2]);
    double yodh = Trio::planar_moment(input.stars[0], input.stars[1], input.stars[2]);
    std::vector<Plane::hr_trio> teth = p.query_for_trio(kaph, yodh);
    std::array<bool, 3> matched = {false, false, false};

    // check that original input trio exists in search
    for (const Plane::hr_trio &t : teth) {
        for (int i = 0; i < 3; i++) {
            if (input.stars[i].get_hr() == t[0] || input.stars[i].get_hr() == t[1] ||
                input.stars[i].get_hr() == t[2]) {
                matched[i] = true;
            }
        }
    }

    assert_true(matched[0], "QueryTrioInsideInputStar0");
    assert_true(matched[1], "QueryTrioInsideInputStar1");
    assert_true(matched[2], "QueryTrioInsideInputStar2");
}

/*
 * Check that the zero-length stars are returned when a theta between a pair of stars is greater
 * than the current fov.
 */
void TestPlanarTriangle::test_match_stars_fov() {
    Plane kaph(Benchmark(10, Star::chance(), Rotation::chance()));
    kaph.input[0].hr = 3;
    kaph.input[1].hr = 4;
    kaph.input[2].hr = 5;

    std::vector<Plane::star_trio> yodh = kaph.match_stars({0, 1, 2});
    assert_true(yodh[0][0] == Star() && yodh[0][1] == Star() && yodh[0][2] == Star(),
                "CandidateOutOfFOV");
}

/*
 * Check that the zero-length stars are returned when no matching trio is found.
 */
void TestPlanarTriangle::test_match_stars_none() {
    Plane kaph(Benchmark(10, Star::chance(), Rotation::chance()));
    kaph.input[0] = Star(1, 1, 1.1);
    kaph.input[1] = Star(1, 1, 1);
    kaph.input[2] = Star(1.1, 1, 1);

    std::vector<Plane::star_trio> yodh = kaph.match_stars({0, 1, 2});
    assert_true(yodh[0][0] == Star() && yodh[0][1] == Star() && yodh[0][2] == Star(),
                "CandidateNoMatchingPair");
}

/*
 * Check that the correct stars are returned from the candidate trio query.
 */
void TestPlanarTriangle::test_match_stars_results() {
    Plane kaph(Benchmark(15, Star::chance(), Rotation::chance()));
    kaph.input[0] = kaph.ch.query_bsc5(3898);
    kaph.input[1] = kaph.ch.query_bsc5(4325);
    kaph.input[2] = kaph.ch.query_bsc5(4502);

    std::vector<Plane::star_trio> yodh = kaph.match_stars({0, 1, 2});
    std::array<bool, 3> matched = {false, false, false};

    // check that original input trio exists in search
    for (const Plane::star_trio &t : yodh) {
        for (int i = 0; i < 3; i++) {
            if (kaph.input[i] == t[0] || kaph.input[i] == t[1] || kaph.input[i] == t[2]) {
                matched[i] = true;
            }
        }
    }

    assert_true(matched[0], "CandidateMatchingStar0");
    assert_true(matched[1], "CandidateMatchingStar1");
    assert_true(matched[2], "CandidateMatchingStar2");
}

/*
 * Check that the pivot query method returns the correct trio.
 */
void TestPlanarTriangle::test_pivot_query_results() {
    Plane kaph(Benchmark(15, Star::chance(), Rotation::chance()));
    kaph.input[0] = kaph.ch.query_bsc5(3898);
    kaph.input[1] = kaph.ch.query_bsc5(4325);
    kaph.input[2] = kaph.ch.query_bsc5(4502);

    std::vector<Plane::star_trio> yodh = kaph.match_stars({0, 1, 2});
    Plane::star_trio teth = kaph.pivot({0, 1, 2}, yodh);
    assert_true(teth[0] == kaph.input[0] || teth[0] == kaph.input[1] || teth[0] == kaph.input[2],
                "CandidateMatchingStarPivotQueryStar0");
    assert_true(teth[1] == kaph.input[0] || teth[1] == kaph.input[1] || teth[1] == kaph.input[2],
                "CandidateMatchingStarPivotQueryStar1");
    assert_true(teth[2] == kaph.input[0] || teth[2] == kaph.input[1] || teth[2] == kaph.input[2],
                "CandidateMatchingStarPivotQueryStar2");
}

/*
 * Check that the rotating match method marks the all stars as matched.
 */
void TestPlanarTriangle::test_rotating_match_correct_input() {
    Star kaph = Star::chance(), yodh = Star::chance();
    Rotation teth = Rotation::chance();
    Star heth = Rotation::rotate(kaph, teth);
    Star zayin = Rotation::rotate(yodh, teth);
    Rotation waw = Rotation::rotation_across_frames({kaph, yodh}, {heth, zayin});
    Benchmark input(8, Star::chance(), teth);
    std::vector<Star> rev_input;
    PlanarTriangle he(input);

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
void TestPlanarTriangle::test_rotating_match_error_input() {
    Star kaph = Star::chance(), yodh = Star::chance();
    Rotation teth = Rotation::chance();
    Star heth = Rotation::rotate(kaph, teth);
    Star zayin = Rotation::rotate(yodh, teth);
    Rotation waw = Rotation::rotation_across_frames({kaph, yodh}, {heth, zayin});
    Benchmark input(8, Star::chance(), teth);
    std::vector<Star> rev_input;
    PlanarTriangle he(input);

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
void TestPlanarTriangle::test_rotating_match_duplicate_input() {
    Star kaph = Star::chance(), yodh = Star::chance();
    Rotation teth = Rotation::chance();
    Star heth = Rotation::rotate(kaph, teth);
    Star zayin = Rotation::rotate(yodh, teth);
    Rotation waw = Rotation::rotation_across_frames({kaph, yodh}, {heth, zayin});
    Benchmark input(8, Star::chance(), teth);
    std::vector<Star> rev_input;
    PlanarTriangle he(input);

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
void TestPlanarTriangle::test_identify_clean_input() {
    Benchmark input(8, Star::chance(), Rotation::chance());
    PlanarTriangle::Parameters kaph;

    // we define a match as 66% here
    kaph.match_minimum = (unsigned int) (input.stars.size() / 3.0);

    std::vector<Star> teth = PlanarTriangle::identify(input, kaph);
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
void TestPlanarTriangle::test_identify_error_input() {
    Benchmark input(9, Star::chance(), Rotation::chance());
    PlanarTriangle::Parameters kaph;
    input.add_extra_light(1);

    // we define a match as 66% here
    kaph.match_minimum = (unsigned int) ((input.stars.size() - 1) / 3.0);

    std::vector<Star> teth = PlanarTriangle::identify(input, kaph);
    assert_true(teth.size() == input.stars.size() - 1, "IdentificationFoundWithErrorSize");

    for (unsigned int a = 0; a < teth.size(); a++) {
        auto match = [teth, a](const Star &b) -> bool { return b.get_hr() == teth[a].get_hr(); };
        auto is_found = std::find_if(input.stars.begin(), input.stars.end(), match);

        std::string test_name = "IdentificationErrorInputStar" + std::to_string(a + 1);
        assert_true(is_found != input.stars.end(), test_name);
    }
}

/*
 * Enumerate all tests in TestTrianglePlanar.
 *
 * @return -1 if the test case does not exist. 0 otherwise.
 */
int TestPlanarTriangle::enumerate_tests(int test_case) {
    switch (test_case) {
        case 0: test_trio_query();
            break;
        case 1: test_match_stars_fov();
            break;
        case 2: test_match_stars_none();
            break;
        case 3: test_match_stars_results();
            break;
        case 4: test_pivot_query_results();
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
 * Run the tests in TestPlanarTriangle.
 */
int main() {
//    PlanarTriangle::generate_triangle_table(20, "PLAN20");
//    Chomp::create_k_vector("PLAN20", "a");
//    Nibble::polish_table("PLAN20_KVEC", "k_value");
    return TestPlanarTriangle().execute_tests();
}
