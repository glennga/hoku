/// @file test-angle.cpp
/// @author Glenn Galvizo
///
/// Source file for all Angle class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "identification/angle.h"
#include "gmock/gmock.h"

// Import several matchers from Google Mock.
using testing::UnorderedElementsAre;
using testing::Contains;
using testing::Not;

/// Check that the constructor correctly sets the object's attributes.
TEST(AngleConstructor, Constructor) {
    Chomp ch;
    Benchmark input(ch, 20);
    Angle::Parameters p = {0.01, 10, false, true, 0.1, 10, std::make_shared<unsigned int>(0), Rotation::quest, "H"};
    Angle a(input, p);
    
    EXPECT_EQ(a.fov, 20);
    EXPECT_EQ(a.ch.table, "H");
    EXPECT_EQ(a.parameters.sigma_query, p.sigma_query);
    EXPECT_EQ(a.parameters.sql_limit, p.sql_limit);
    EXPECT_EQ(a.parameters.pass_r_set_cardinality, p.pass_r_set_cardinality);
    EXPECT_EQ(a.parameters.favor_bright_stars, p.favor_bright_stars);
    EXPECT_EQ(a.parameters.nu_max, p.nu_max);
    EXPECT_EQ(a.parameters.nu, p.nu);
    EXPECT_EQ(a.parameters.f, p.f);
    EXPECT_EQ(a.parameters.table_name, p.table_name);
}

/// Check the existence and the structure of the Angle table.
TEST(AngleTable, ExistenceStructure) {
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
    Angle::generate_table(cf, "angle");
    Nibble nb;
    
    SQLite::Statement q(*nb.conn, "SELECT 1 FROM " + cf.Get("table-names", "angle", "") + " LIMIT 1");
    EXPECT_TRUE(q.executeStep());
    EXPECT_NO_THROW(nb.select_table(cf.Get("table-names", "angle", ""), true););
    
    std::string schema, fields;
    nb.find_attributes(schema, fields);
    EXPECT_EQ(schema, "label_a INT, label_b INT, theta FLOAT");
    EXPECT_EQ(fields, "label_a, label_b, theta");
}

/// Check that the entries in the Angle table are correct.
TEST(AngleTable, CorrectEntries) {
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
    Angle::generate_table(cf, "angle");
    Chomp ch;
    ch.select_table(cf.Get("table-names", "angle", ""));
    
    Star::list b = ch.bright_as_list(); // This list is ordered by label, do not need to worry about reversed case.
    double theta = (180.0 / M_PI) * Vector3::Angle(b[0], b[1]);
    double theta_2 = ch.search_single("theta", "label_a = " + std::to_string(b[0].get_label()) + " AND label_b = "
                                               + std::to_string(b[1].get_label()));
    EXPECT_EQ(theta, theta_2);
}

/// Check that query_for_pair method returns the catalog ID of the correct stars, and actually returns stars.
TEST(AngleQuery, Pair) {
    Chomp ch;
    Benchmark input(ch, 15);
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS, p2 = Angle::DEFAULT_PARAMETERS;
    p.sigma_query = 0.00000000001, p2.sigma_query = 0.1;
    
    // It is known that the angle between b_0 and b_1 here is < 20.
    double a = (180.0 / M_PI) * Vector3::Angle(input.b[0], input.b[1]);
    Identification::labels_list b = Angle(input, p).query_for_pair(a);
    Identification::labels_list c = {input.b[0].get_label(), input.b[1].get_label()};
    EXPECT_THAT(c, Contains(b[0]));
    EXPECT_THAT(c, Contains(b[1]));
    
    p2.pass_r_set_cardinality = false;
    Identification::labels_list d = Angle(input, p2).query_for_pair(a);
    EXPECT_THAT(Angle::EMPTY_BIG_R_ELL, Not(Contains(d[0])));
    EXPECT_THAT(Angle::EMPTY_BIG_R_ELL, Not(Contains(d[1])));
}

/// Check that the query_for_pair method fails when expected.
TEST(AngleQuery, ExpectedFailure) {
    Chomp ch;
    Benchmark input(ch, 15), input2(ch, 15);
    input.shift_light(static_cast<unsigned int> (input.b.size()), 0.001);
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.sigma_query = std::numeric_limits<double>::epsilon();
    
    double a = (180.0 / M_PI) * Vector3::Angle(input.b[0], input.b[1]);
    Identification::labels_list b = Angle(input, p).query_for_pair(a);
    EXPECT_THAT(Angle::EMPTY_BIG_R_ELL, Contains(b[0]));
    EXPECT_THAT(Angle::EMPTY_BIG_R_ELL, Contains(b[1]));
    
    // |R| restriction should prevent an answer from being displayed.
    p.sigma_query = 0.01;
    double d = (180.0 / M_PI) * Vector3::Angle(input2.b[0], input2.b[1]);
    Identification::labels_list c = Angle(input2, p).query_for_pair(d);
    EXPECT_THAT(Angle::EMPTY_BIG_R_ELL, Contains(c[0]));
    EXPECT_THAT(Angle::EMPTY_BIG_R_ELL, Contains(c[1]));
}

/// Check that the brightest pair is selected, using the fbs flag.
TEST(AngleQuery, FavorBrightStarsFlag) {
    Chomp ch;
    Benchmark input(ch, 15);
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS, p2 = Angle::DEFAULT_PARAMETERS;
    p.sigma_query = 0.0001, p2.sigma_query = 0.000000001, p.favor_bright_stars = true;
    p.pass_r_set_cardinality = false, p2.pass_r_set_cardinality = false;
    
    double a = (180.0 / M_PI) * Vector3::Angle(input.b[0], input.b[1]);
    Identification::labels_list b = Angle(input, p).query_for_pair(a);
    Identification::labels_list c = Angle(input, p2).query_for_pair(a);
    EXPECT_LT(ch.query_hip(b[0]).get_magnitude() + ch.query_hip(b[1]).get_magnitude(),
              ch.query_hip(c[0]).get_magnitude() + ch.query_hip(c[1]).get_magnitude());
}

/// Check that the zero-length stars are returned given that theta is greater than the current fov.
TEST(AngleCandidatePair, FOV) {
    Chomp ch;
    Angle a(Benchmark(ch, 10), Angle::DEFAULT_PARAMETERS);
    Star b(0.928454687492219, 0.132930961972911, 0.346844709665121);
    Star c(0.998078771188383, -0.0350062881876723, 0.0511207031486225);
    
    Star::pair d = a.find_candidate_pair(b, c);
    EXPECT_THAT(Angle::NO_CANDIDATE_PAIR_FOUND, Contains(d[0]));
    EXPECT_THAT(Angle::NO_CANDIDATE_PAIR_FOUND, Contains(d[1]));
}

/// Check that the zero-length stars are returned when no matching theta is found.
TEST(AngleCandidatePair, None) {
    Chomp ch;
    Angle a(Benchmark(ch, 10), Angle::DEFAULT_PARAMETERS);
    
    Star::pair b = a.find_candidate_pair(Star(1, 1, 1), Star(1.1, 1, 1));
    EXPECT_THAT(Angle::NO_CANDIDATE_PAIR_FOUND, Contains(b[0]));
    EXPECT_THAT(Angle::NO_CANDIDATE_PAIR_FOUND, Contains(b[1]));
}

/// Check that the direct match test returns the correct set.
TEST(AngleDMT, DirectMatchTest) {
    Chomp ch;
    Rotation q = Rotation::chance();
    Star::list n = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532)};
    Star::list n_q = {Rotation::rotate(n[0], q), Rotation::rotate(n[1], q), Rotation::rotate(n[2], q)};
    Benchmark input(n_q, n_q[0], 20);
    Identification::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.sigma_overlay = 0.0001;
    Angle b(input, p);
    
    Star::list a = b.direct_match_test(n, {n[0], n[1]}, {input.b[0], input.b[1]});
    Star::list d = b.direct_match_test(n, {n[0], n[1]}, {input.b[1], input.b[0]});
    ASSERT_EQ(a.size(), 2);
    ASSERT_EQ(d.size(), 2);
    
    Identification::labels_list f = {a[0].get_label(), a[1].get_label()};
    Identification::labels_list f_2 = {d[0].get_label(), d[1].get_label()};
    EXPECT_THAT(f, Contains(n_q[0].get_label()));
    EXPECT_THAT(f, Contains(n_q[1].get_label()));
    EXPECT_THAT(f_2, Contains(n_q[0].get_label()));
    EXPECT_THAT(f_2, Contains(n_q[1].get_label()));
}

/// Check that the correct stars are returned from the candidate pair query.
TEST(AngleResults, Query) {
    Chomp ch;
    Benchmark input(ch, 15);
    Angle b(input, Angle::DEFAULT_PARAMETERS);
    
    Star::pair c = b.find_candidate_pair(input.b[0], input.b[1]);
    Identification::labels_list d = {input.b[0].get_label(), input.b[1].get_label()};
    Identification::labels_list e = {input.b[0].get_label(), input.b[1].get_label()};
    EXPECT_THAT(d, Contains(c[0].get_label()));
    EXPECT_THAT(e, Contains(c[1].get_label()));
}

/// Check that a clean input returns the expected query result.
TEST(AngleTrial, CleanQuery) {
    Chomp ch;
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.sigma_query = 10e-7, p.pass_r_set_cardinality = false;
    Angle a(Benchmark::black(), p);
    Star b = ch.query_hip(22667), c = ch.query_hip(27913);
    
    std::vector<Identification::labels_list> d = a.query({b, c});
    EXPECT_THAT(d, Contains(Identification::labels_list {22667, 27913}));
}

/// Check that a clean input returns the correct stars from a set of candidates.
TEST(AngleTrial, CleanReduction) {
    Chomp ch;
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.sigma_overlay = 0.0001, p.sigma_query = 0.000000001;
    
    Benchmark i ({ch.query_hip(22667), ch.query_hip(27913)}, ch.query_hip(22667), 20);
    Angle a(i, p);
    EXPECT_THAT(a.reduce(), UnorderedElementsAre(22667, 27913));
}

/// Check that a clean input returns the expected identification of stars.
TEST(AngleTrial, CleanIdentify) {
    Chomp ch;
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.nu = std::make_shared<unsigned int>(0);
    p.sigma_query = 10e-9;
    p.sigma_overlay = 0.000001;
    
    Rotation q = Rotation::chance();
    Star b = ch.query_hip(22667), c = ch.query_hip(27913);
    Star d = Rotation::rotate(b, q), e = Rotation::rotate(c, q);
    
    Angle a(Benchmark({d, e}, d, 20), p);
    Star::list f = a.identify();
    EXPECT_THAT(f, Contains(Star::define_label(d, 22667)));
    EXPECT_THAT(f, Contains(Star::define_label(e, 27913)));
}

/// Check that the nu_max is respected in identification.
TEST(AngleTrial, ExceededNu) {
    Chomp ch;
    Benchmark input(ch, 15);
    input.shift_light(static_cast<unsigned int> (input.b.size()), 0.001);
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.nu = std::make_shared<unsigned int>(0), p.nu_max = 10;
    p.sigma_query = std::numeric_limits<double>::epsilon();
    p.sigma_overlay = std::numeric_limits<double>::epsilon();
    Angle a(input, p);
    
    EXPECT_EQ(a.identify()[0], Angle::EXCEEDED_NU_MAX[0]);
    EXPECT_EQ(*p.nu, p.nu_max + 1);
}

/// Check that the correct result is returned when no map is found.
TEST(AngleTrial, NoMapFound) {
    Chomp ch;
    Benchmark input(ch, 7);
    input.shift_light(static_cast<unsigned int> (input.b.size()), 0.001);
    Angle::Parameters p = Angle::DEFAULT_PARAMETERS;
    p.nu = std::make_shared<unsigned int>(0), p.nu_max = std::numeric_limits<unsigned int>::max();
    p.sigma_query = std::numeric_limits<double>::epsilon();
    p.sigma_overlay = std::numeric_limits<double>::epsilon();
    Angle a(input, p);
    
    EXPECT_EQ(a.identify()[0], Angle::NO_CONFIDENT_A[0]);
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