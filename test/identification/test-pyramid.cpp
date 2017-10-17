/// @file test-pyramid.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestPyramid class, as well as the main function to run the tests.

#include "identification/test-pyramid.h"

/// Check that reference_find method returns the correct star.
///
/// @return 0 when finished.
int TestPyramid::test_reference_find () {
    Pyramid::label_list_pair ei = {Pyramid::label_pair {3, 100}, Pyramid::label_pair{3, 413}, Pyramid::label_pair {7, 87}};
    Pyramid::label_list_pair ej = {Pyramid::label_pair {3, 2}, Pyramid::label_pair{3, 5}, Pyramid::label_pair {13, 87}};
    Pyramid::label_list_pair ek = {Pyramid::label_pair {90, 12345}, Pyramid::label_pair{3, 7352},
        Pyramid::label_pair {9874, 512}};
    Pyramid a(Benchmark(20, Star::chance(), Rotation::chance()), Pyramid::Parameters());
    Star b = a.find_reference(ei, ej, ek);
    
    return 0 * assert_equal(b, a.ch.query_hip(3), "ReferenceStarCorrectlyFound", b.str());
}

/// Check that find_candidate_quad method returns the correct quad.
///
/// @return 0 when finished.
int TestPyramid::test_candidate_quad_find () {
    Benchmark input(20, Star::chance(), Rotation::chance());
    Pyramid a(input, Pyramid::Parameters());
    Pyramid::hr_quad b = a.find_candidate_quad({0, 1, 2, 3});
    
    assert_equal(input.stars[0].get_label(), b[0], "Star0MatchesInputHR");
    assert_equal(input.stars[1].get_label(), b[1], "Star1MatchesInputHR");
    assert_equal(input.stars[2].get_label(), b[2], "Star2MatchesInputHR");
    return 0 * assert_equal(input.stars[3].get_label(), b[3], "Star3MatchesInputHR");
}

/// Check that correct result is returned with a clean input.
///
/// @return 0 when finished.
int TestPyramid::test_identify_clean_input () {
    Benchmark input(20, Star::chance(), Rotation::chance());
    std::vector<Star> c = Pyramid::identify(input, Pyramid::Parameters());
    assert_equal(c.size(), input.stars.size(), "IdentificationFoundAllSize");
    
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
int TestPyramid::test_identify_error_input () {
    Benchmark input(9, Star::chance(), Rotation::chance());
    input.add_extra_light(1);
    
    std::vector<Star> c = Pyramid::identify(input, Pyramid::Parameters());
    assert_equal(c.size(), input.stars.size() - 1, "IdentificationFoundWithErrorSize");
    
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

/// Enumerate all tests in TestPyramid.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestPyramid::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_reference_find();
        case 1: return test_candidate_quad_find();
        case 2: return test_identify_clean_input();
        case 3: return test_identify_error_input();
        default: return -1;
    }
}

/// Run the tests in TestPyramid. Currently set to log all results. 
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    //    Pyramid::generate_sep_table(20, "SEP20");
    return TestPyramid().execute_tests(BaseTest::FULL_PRINT_LOG_ON);
}
