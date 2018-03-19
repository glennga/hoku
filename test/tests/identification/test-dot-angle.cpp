/// @file test-dot-angle.cpp
/// @author Glenn Galvizo
///
/// Source file for all DotAngle class unit tests.

#define ENABLE_TESTING_ACCESS

#define _USE_MATH_DEFINES
#include <cmath>
#include <identification/angle.h>
#include "gmock/gmock.h"

#include "math/trio.h"
#include "identification/dot-angle.h"

// Import several matchers from Google Mock.
using testing::UnorderedElementsAre;
using testing::Contains;
using testing::Not;

/// Check that the constructor correctly sets the object's attributes.
TEST(DotAngle, Constructor) {
    Chomp ch;
    Benchmark input(ch, 20);
    Dot::Parameters p = {0.01, 0.000001, 0.00000000001, 0.1, 10, false, true, 10, std::make_shared<unsigned int>(0),
        Rotation::svd, "H"};
    Dot a(input, p);
    
    EXPECT_EQ(a.fov, 20);
    EXPECT_EQ(a.ch.table, "H");
    EXPECT_EQ(a.parameters.sigma_1, p.sigma_1);
    EXPECT_EQ(a.parameters.sigma_2, p.sigma_2);
    EXPECT_EQ(a.parameters.sigma_3, p.sigma_3);
    EXPECT_EQ(a.parameters.sigma_4, p.sigma_4);
    EXPECT_EQ(a.parameters.sql_limit, p.sql_limit);
    EXPECT_EQ(a.parameters.no_reduction, p.no_reduction);
    EXPECT_EQ(a.parameters.favor_bright_stars, p.favor_bright_stars);
    EXPECT_EQ(a.parameters.nu_max, p.nu_max);
    EXPECT_EQ(a.parameters.nu, p.nu);
    EXPECT_EQ(a.parameters.f, p.f);
    EXPECT_EQ(a.parameters.table_name, p.table_name);
}

/// Check the existence and the structure of the DotAngle table.
TEST(DotAngle, TableExistenceStructure) {
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
    Dot::generate_table(cf);
    Nibble nb;
    
    SQLite::Statement q(*nb.conn, "SELECT 1 FROM " + cf.Get("table-names", "dot", "") + " LIMIT 1");
    EXPECT_TRUE(q.executeStep());
    EXPECT_NO_THROW(nb.select_table(cf.Get("table-names", "dot", ""), true););
    
    std::string schema, fields;
    nb.find_attributes(schema, fields);
    EXPECT_EQ(schema, "label_a INT, label_b INT, label_c INT, theta_1 FLOAT, theta_2 FLOAT, phi FLOAT");
    EXPECT_EQ(fields, "label_a, label_b, label_c, theta_1, theta_2, phi");
}

/// Check that the entries in the DotAngle table are correct.
TEST(DotAngle, TableCorrectEntries) {
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
    Dot::generate_table(cf);
    Chomp ch;
    ch.select_table(cf.Get("table-names", "dot", ""));
    
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532)};
    double theta_1 = (180.0 / M_PI) * Vector3::Angle(b[0], b[2]);
    double theta_2 = (180.0 / M_PI) * Vector3::Angle(b[1], b[2]);
    double phi = Trio::dot_angle(b[0], b[1], b[2]);
    
    Nibble::tuples_d t = ch.search_table("theta_1, theta_2, phi",
                                         "label_a = " + std::to_string(b[0].get_label()) + " AND label_b = "
                                         + std::to_string(b[1].get_label()) + " AND label_c = "
                                         + std::to_string(b[2].get_label()), 1);
    ASSERT_EQ(t.size(), 1);
    ASSERT_EQ(t[0].size(), 3);
    EXPECT_FLOAT_EQ(theta_1, t[0][0]);
    EXPECT_FLOAT_EQ(theta_2, t[0][1]);
    EXPECT_FLOAT_EQ(phi, t[0][2]);
}

///// No test is performed here. This is just to see how long the entire table will load into memory.
//TEST(DotAngle, ChompInMemory) {
//    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
//    Chomp ch(cf.Get("table-names", "dot", ""), cf.Get("table-focus", "dot", ""));
//}

/// Check that query_for_trio method returns the catalog ID of the correct stars, and actually returns stars.
TEST(DotAngle, QueryTrio) {
    Chomp ch;
    Benchmark input(ch, 15);
    Dot::Parameters p = Dot::DEFAULT_PARAMETERS, p2 = Dot::DEFAULT_PARAMETERS;
    p.sigma_1 = p.sigma_2 = p.sigma_3 = 0.000001;
    p2.sigma_1 = p2.sigma_2 = p2.sigma_3 = 0.1;
    
    Star::list b = {ch.query_hip(102531), ch.query_hip(109240), ch.query_hip(102532)};
    double theta_1 = (180.0 / M_PI) * Vector3::Angle(b[0], b[2]);
    double theta_2 = (180.0 / M_PI) * Vector3::Angle(b[1], b[2]);
    double phi = Trio::dot_angle(b[0], b[1], b[2]);
    
    Identification::labels_list c = Dot(input, p).query_for_trio(theta_1, theta_2, phi);
    Identification::labels_list d = {102531, 109240, 102532};
    ASSERT_EQ(c.size(), 3);
    EXPECT_EQ(d[0], c[0]);
    EXPECT_EQ(d[1], c[1]);
    EXPECT_EQ(d[2], c[2]);
    
    p2.no_reduction = true;
    Identification::labels_list e = Dot(input, p2).query_for_trio(theta_1, theta_2, phi);
    EXPECT_THAT(Dot::NO_CANDIDATES_FOUND, Not(Contains(e[0])));
    EXPECT_THAT(Dot::NO_CANDIDATES_FOUND, Not(Contains(e[1])));
}

/// Check that the query_for_trio method fails when expected.
TEST(DotAngle, QueryExpectedFailure) {
    Chomp ch;
    Benchmark input(ch, 15), input2(ch, 15);
    input.shift_light(static_cast<unsigned int> (input.b.size()), 0.001);
    Dot::Parameters p = Dot::DEFAULT_PARAMETERS;
    p.sigma_1 = p.sigma_2 = p.sigma_3 = 1.0e-19;
    
    double theta_1 = (180.0 / M_PI) * Vector3::Angle(input.b[0], input.b[2]);
    double theta_2 = (180.0 / M_PI) * Vector3::Angle(input.b[1], input.b[2]);
    if (theta_1 > theta_2) {
        theta_1 = (180.0 / M_PI) * Vector3::Angle(input.b[1], input.b[2]);
        theta_2 = (180.0 / M_PI) * Vector3::Angle(input.b[0], input.b[2]);
    }
    double phi = Trio::dot_angle(input.b[0], input.b[1], input.b[2]);
    
    Identification::labels_list b = Dot(input, p).query_for_trio(theta_1, theta_2, phi);
    EXPECT_THAT(Dot::NO_CANDIDATES_FOUND, Contains(b[0]));
    EXPECT_THAT(Dot::NO_CANDIDATES_FOUND, Contains(b[1]));
    
    // |R| restriction should prevent an answer from being displayed.
    double theta_1_b = (180.0 / M_PI) * Vector3::Angle(input2.b[0], input2.b[2]);
    double theta_2_b = (180.0 / M_PI) * Vector3::Angle(input2.b[1], input2.b[2]);
    if (theta_1_b > theta_2_b) {
        theta_1_b = (180.0 / M_PI) * Vector3::Angle(input2.b[1], input2.b[2]);
        theta_2_b = (180.0 / M_PI) * Vector3::Angle(input2.b[0], input2.b[2]);
    }
    double phi_2 = Trio::dot_angle(input2.b[0], input2.b[1], input2.b[2]);
    
    p.sigma_1 = p.sigma_2 = p.sigma_3 = 10;
    Identification::labels_list c = Dot(input, p).query_for_trio(theta_1_b, theta_2_b, phi_2);
    EXPECT_THAT(Dot::NO_CANDIDATES_FOUND, Contains(c[0]));
    EXPECT_THAT(Dot::NO_CANDIDATES_FOUND, Contains(c[1]));
}

/// Check that the brightest pair is selected, using the fbs flag.
TEST(DotAngle, QueryFavorBrightStarsFlag) {
    Chomp ch;
    Benchmark input(ch, 15);
    Dot::Parameters p = Dot::DEFAULT_PARAMETERS, p2 = Dot::DEFAULT_PARAMETERS;
    p.sigma_1 = p.sigma_2 = p.sigma_3 = 0.1, p2.sigma_1 = p2.sigma_2 = p2.sigma_3 = 0.1, p.favor_bright_stars = true;
    p.no_reduction = true, p2.no_reduction = true, p.sql_limit = 100000, p2.sql_limit = 100000;
    
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532)};
    double theta_1 = (180.0 / M_PI) * Vector3::Angle(b[0], b[2]);
    double theta_2 = (180.0 / M_PI) * Vector3::Angle(b[1], b[2]);
    double phi = Trio::dot_angle(b[0], b[1], b[2]);
    
    Identification::labels_list c = Dot(input, p).query_for_trio(theta_1, theta_2, phi);
    Identification::labels_list d = Dot(input, p2).query_for_trio(theta_1, theta_2, phi);
    ASSERT_EQ(b.size() + c.size(), 6);
    EXPECT_LT(
        ch.query_hip(c[0]).get_magnitude() + ch.query_hip(c[1]).get_magnitude() + ch.query_hip(c[2]).get_magnitude(),
        ch.query_hip(d[0]).get_magnitude() + ch.query_hip(d[1]).get_magnitude() + ch.query_hip(d[2]).get_magnitude());
}

/// Check that the zero-length stars are returned given that theta is greater than the current fov.
TEST(DotAngle, CandidateTrioFOV) {
    Chomp ch;
    Dot a(Benchmark(ch, 10), Dot::DEFAULT_PARAMETERS);
    Star b(0.998078771188383, -0.0350062881876723, 0.0511207031486225);
    Star c(0.998078771188383, -0.0350062881876723, 0.0511207);
    Star d(0.928454687492219, 0.132930961972911, 0.346844709665121);
    
    Star::trio e = a.find_candidate_trio(b, c, d);
    EXPECT_THAT(Dot::NO_CANDIDATE_TRIO_FOUND, Contains(e[0]));
    EXPECT_THAT(Dot::NO_CANDIDATE_TRIO_FOUND, Contains(e[1]));
    EXPECT_THAT(Dot::NO_CANDIDATE_TRIO_FOUND, Contains(e[2]));
}

/// Check that the stars that do not meet condition D are returned with an error set.
TEST(DotAngle, CandidateTrioCondition6D) {
    Chomp ch;
    Dot a(Benchmark(ch, 30), Dot::DEFAULT_PARAMETERS);
    Star b(0.998078771188383, -0.0350062881876723, 0.0511207031486225);
    Star c(0.998078771188383, -0.0350062881876723, 0.0511207);
    Star d(0.928454687492219, 0.132930961972911, 0.346844709665121);
    
    Star::trio e = a.find_candidate_trio(c, b, d);
    EXPECT_THAT(Dot::NO_CANDIDATE_TRIO_FOUND, Contains(e[0]));
    EXPECT_THAT(Dot::NO_CANDIDATE_TRIO_FOUND, Contains(e[1]));
    EXPECT_THAT(Dot::NO_CANDIDATE_TRIO_FOUND, Contains(e[2]));
}

/// Check that the zero-length stars are returned when no matching theta is found.
TEST(DotAngle, CandidatePairNone) {
    Chomp ch;
    Dot a(Benchmark(ch, 10), Dot::DEFAULT_PARAMETERS);
    
    Star::trio b = a.find_candidate_trio(Star(1, 1, 1), Star(1.1, 1, 1), Star(1.11, 1, 1));
    EXPECT_THAT(Dot::NO_CANDIDATE_TRIO_FOUND, Contains(b[0]));
    EXPECT_THAT(Dot::NO_CANDIDATE_TRIO_FOUND, Contains(b[1]));
    EXPECT_THAT(Dot::NO_CANDIDATE_TRIO_FOUND, Contains(b[1]));
}

/// Check that the correct stars are returned from the candidate trio query.
TEST(DotAngle, ResultsQuery) {
    Chomp ch;
    Benchmark input(ch, 15);
    Dot::Parameters p = Dot::DEFAULT_PARAMETERS;
    p.sigma_1 = p.sigma_2 = p.sigma_3 = 0.001, p.sql_limit = 100000;
    Dot b(input, p);
    
    Star::trio c = b.find_candidate_trio(input.b[0], input.b[1], input.b[2]);
    if (std::equal(c.begin(), c.end(), Dot::NO_CANDIDATE_TRIO_FOUND.begin())) {
        c = b.find_candidate_trio(input.b[1], input.b[0], input.b[2]);
    }
    Identification::labels_list d = {input.b[0].get_label(), input.b[1].get_label(), input.b[2].get_label()};
    EXPECT_THAT(d, Contains(c[0].get_label()));
    EXPECT_THAT(d, Contains(c[1].get_label()));
    EXPECT_THAT(d, Contains(c[2].get_label()));
}

/// Check that a clean input returns the expected query result.
TEST(DotAngle, TrialCleanQuery) {
    Chomp ch;
    Dot::Parameters p = Dot::DEFAULT_PARAMETERS;
    p.sigma_1 = p.sigma_2 = p.sigma_3 = 10e-7, p.no_reduction = false;
    Dot a(Benchmark::black(), p);
    Star::list b = {ch.query_hip(102531), ch.query_hip(109240), ch.query_hip(102532)};
    
    std::vector<Identification::labels_list> d = a.query(b);
    EXPECT_EQ(d[0][0], 102531);
    EXPECT_EQ(d[0][1], 109240);
    EXPECT_EQ(d[0][2], 102532);
}

/// Check that a clean input returns the correct stars from a set of candidates.
TEST(DotAngle, TrialCleanReduction) {
    Chomp ch;
    Dot::Parameters p = Dot::DEFAULT_PARAMETERS;
    p.nu = std::make_shared<unsigned int>(0), p.sigma_1 = p.sigma_2 = p.sigma_3 = 0.000000001;
    
    Star::list b = {ch.query_hip(102531), ch.query_hip(109240), ch.query_hip(102532)};
    Benchmark i(b, b[0], 20);
    Dot a(i, p);
    EXPECT_THAT(a.reduce(), UnorderedElementsAre(b[0], b[1], b[2]));
}

/// Check that a clean input returns the expected identification of stars.
TEST(DotAngle, TrialCleanIdentify) {
    Chomp ch;
    Dot::Parameters p = Dot::DEFAULT_PARAMETERS;
    p.nu = std::make_shared<unsigned int>(0);
    p.sigma_1 = p.sigma_2 = p.sigma_3 = 10e-9;
    p.sigma_4 = 0.000001;
    
    Rotation q = Rotation::chance();
    Star b = ch.query_hip(102531), c = ch.query_hip(109240), d = ch.query_hip(102532);
    Star e = Rotation::rotate(b, q), f = Rotation::rotate(c, q), g = Rotation::rotate(d, q);
    
    Dot a(Benchmark({e, f, g}, e, 20), p);
    Star::list h = a.identify();
    EXPECT_THAT(h, Contains(Star::define_label(e, 102531)));
    EXPECT_THAT(h, Contains(Star::define_label(f, 109240)));
    EXPECT_THAT(h, Contains(Star::define_label(g, 102532)));
}

/// Check that the nu_max is respected in identification.
TEST(DotAngle, TrialExceededNu) {
    Chomp ch;
    Benchmark input(ch, 15);
    input.shift_light(static_cast<unsigned int> (input.b.size()), 0.001);
    Dot::Parameters p = Dot::DEFAULT_PARAMETERS;
    p.nu = std::make_shared<unsigned int>(0), p.nu_max = 10;
    p.sigma_1 = p.sigma_2 = p.sigma_3 = 1.0e-19;
    p.sigma_4 = 1.0e-19;
    Dot a(input, p);
    
    EXPECT_EQ(a.identify()[0], Dot::EXCEEDED_NU_MAX[0]);
    EXPECT_EQ(*p.nu, p.nu_max + 1);
}

/// Check that the correct result is returned when no map is found.
TEST(DotAngle, TrialNoMapFound) {
    Chomp ch;
    Benchmark input(ch, 7);
    input.shift_light(static_cast<unsigned int> (input.b.size()), 0.001);
    Dot::Parameters p = Dot::DEFAULT_PARAMETERS;
    p.nu = std::make_shared<unsigned int>(0), p.nu_max = std::numeric_limits<unsigned int>::max();
    p.sigma_1 = p.sigma_2 = p.sigma_3 = 1.0e-19;
    p.sigma_4 = 1.0e-19;
    Dot a(input, p);
    
    EXPECT_EQ(a.identify()[0], Dot::NO_CONFIDENT_A[0]);
}