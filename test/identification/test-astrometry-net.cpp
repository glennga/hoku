/// @file test-astrometry-net.cpp
/// @author Glenn Galvizo
///
/// Source file for the TestAstrometryNet class, as well as the main function to run the tests.

#include "identification/test-astrometry-net.h"

/// Check that a star quad not within the field-of-view is not inserted, or if an asterism can not be generated, or
/// if a star has too high of a hash count.
///
/// @code{.cpp}
/// Notes to check for valid hash:
/// for (int i = 0; i < Nibble::BSC5_TABLE_LENGTH -3; i++) {
///     for (int j = i + 1; j < Nibble::BSC5_TABLE_LENGTH - 2; j++) {
///         for (int k = j + 1; k < Nibble::BSC5_TABLE_LENGTH - 1; k++) {
///             for (int m = k + 1; m < Nibble::BSC5_TABLE_LENGTH; m++) {
///                 auto b = Asterism::hash({nb.all_bsc5_stars()[i], nb.all_bsc5_stars()[j], nb.all_bsc5_stars()[k],
///                    nb.all_bsc5_stars()[m]});
///                    if (b[0] != 0 && b[1] != 0 && b[2] != 0 && b[3] != 0) {
///                        auto c = Asterism::hash({nb.all_bsc5_stars()[i], nb.all_bsc5_stars()[j],
///                                 nb.all_bsc5_stars()[k], nb.all_bsc5_stars()[m]});
///             }
///         }
///     }
/// }
/// @endcode
///
/// @return 0 when finished.
int TestAstrometryNet::test_astro_h_insertion () {
    std::vector<int> a_count;
    Nibble nb;
    
    // All of our asterism counts start at zero. Create the table.
    a_count.resize(Nibble::BSC5_MAX_HR);
    std::fill(a_count.begin(), a_count.end(), 0);
    nb.create_table("TestTable", "hr_0 INT, hr_1 INT, hr_2 INT, hr_3 INT, cx FLOAT, cy FLOAT, dx FLOAT, dy FLOAT");
    nb.select_table("TestTable");
    
    a_count[3] = a_count[4] = a_count[5] = a_count[6] = 1;
    assert_equal(-1, Astro::insert_astro_h(nb, a_count, 1, {3, 4, 5, 6}, 180), "FailsWithACount");
    assert_equal(-1, Astro::insert_astro_h(nb, a_count, 2, {3, 4, 5, 6}, 5), "FailsWithFov");
    assert_equal(-1, Astro::insert_astro_h(nb, a_count, 2, {3, 4, 5, 9110}, 180), "FailsWithInvalidHash");
    
    a_count[3] = a_count[4] = a_count[5] = a_count[6] = 0;
    assert_equal(0, Astro::insert_astro_h(nb, a_count, 2, {3, 4, 5, 8848}, 180), "SucceedsWithValidHash");
    for (const int &i : {3, 4, 5, 8848}) {
        assert_equal(a_count[i], 1, "ACountIncrementedProperly" + std::to_string(i));
    }
    
    return 0;
}

/// Check that an asterism can be correctly queried for.
///
/// @return 0 when finished.
int TestAstrometryNet::test_asterism_query () {
    Benchmark input(20, Nibble().query_bsc5(3), Rotation::chance());
    Astro a(input, Astro::Parameters());
    Nibble nb;
    
    a.input[0] = nb.query_bsc5(3), a.input[1] = nb.query_bsc5(4), a.input[2] = nb.query_bsc5(5);
    a.input[3] = nb.query_bsc5(9110), a.input[4] = nb.query_bsc5(8848);
    
    // Verify that an asterism that is incorrect returns {-1, -1, -1, -1}.
    assert_equal(-1, a.query_for_asterism({0, 1, 2, 3})[0], "NegativeHashReturned");
    
    // Verify that the HR values returned from the query are the input.
    assert_equal(3, a.query_for_asterism({0, 1, 2, 4})[0], "FirstHrIsCorrect");
    assert_equal(4, a.query_for_asterism({0, 1, 2, 4})[1], "SecondHrIsCorrect");
    assert_equal(5, a.query_for_asterism({0, 1, 2, 4})[2], "ThirdHrIsCorrect");
    return 0 * assert_equal(8848, a.query_for_asterism({0, 1, 2, 4})[3], "FourthHrIsCorrect");
}

/// Check that correct result is returned with a clean input.
///
/// @return 0 when finished.
int TestAstrometryNet::test_identify_clean_input () {
    Benchmark input(8, Star::chance(), Rotation::chance());
    Astro::Parameters a;
    
    std::vector<Star> c = Astro::identify(input, a);
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
int TestAstrometryNet::test_identify_error_input () {
    Benchmark input(9, Star::chance(), Rotation::chance());
    Astro::Parameters a;
    input.add_extra_light(1);
    
    std::vector<Star> c = Astro::identify(input, a);
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
/// Enumerate all tests in TestAstrometryNet.
///
/// @param test_case Number of the test case to run.
/// @return -1 if the test case does not exist. 0 otherwise.
int TestAstrometryNet::enumerate_tests (int test_case) {
    switch (test_case) {
        case 0: return test_astro_h_insertion();
        case 1: return test_asterism_query();
        case 2: return test_identify_clean_input();
        case 3: return test_identify_error_input();
        default: return -1;
    }
}

/// Run the tests in TestAstrometryNet. Currently set to log all results. 
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    //    AstromteryNet::generate_hash_table(20, 1000, 1000, "ASTRO_H20");
    //    AstrometryNet::generate_center_table("ASTRO_H20", "ASTRO_C20");
    //    Chomp::create_k_vector("ASTRO_H20", "cx");
    //    Chomp::create_k_vector("ASTRO_C20", "i");
    //    Nibble::polish_table("ASTRO_H20_KVEC", "k_value");
    //    Nibble::polish_table("ASTRO_C20_KVEC", "k_value");
    return TestAstrometryNet().execute_tests();
}
