/// @file test-spherical-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for all SphericalTriangle class unit tests.

#define ENABLE_TESTING_ACCESS

#include <libgen.h>
#include "gmock/gmock.h"

#include "identification/spherical-triangle.h"
#include "math/trio.h"

// Import several matchers from Google Mock.
using testing::UnorderedElementsAre;
using testing::Each;
using testing::Contains;

/// Check that the constructor correctly sets the object's attributes.
TEST(SphericalTriangle, Constructor) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 20);
    Sphere::Parameters p = {0.01, 0.00001, 0.0000001, 0.1, 10, false, true, 10, std::make_shared<unsigned int>(0),
        Rotation::svd, "H"};
    Sphere a(input, p);
    
    EXPECT_EQ(a.fov, 20);
    EXPECT_EQ(a.ch.table, "H");
    EXPECT_EQ(a.parameters->sigma_1, p.sigma_1);
    EXPECT_EQ(a.parameters->sigma_2, p.sigma_2);
    EXPECT_EQ(a.parameters->sigma_3, p.sigma_3);
    EXPECT_EQ(a.parameters->sigma_4, p.sigma_4);
    EXPECT_EQ(a.parameters->sql_limit, p.sql_limit);
    EXPECT_EQ(a.parameters->no_reduction, p.no_reduction);
    EXPECT_EQ(a.parameters->favor_bright_stars, p.favor_bright_stars);
    EXPECT_EQ(a.parameters->nu_max, p.nu_max);
    EXPECT_EQ(a.parameters->f, p.f);
    EXPECT_EQ(a.parameters->table_name, p.table_name);
}

/// Check the existence and the structure of the SphericalTriangle table.
TEST(SphericalTriangle, TableExistenceStructure) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    INIReader cf(std::getenv("HOKU_CONFIG_INI") ? std::string(std::getenv("HOKU_CONFIG_INI")) :
                 std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../CONFIG.ini");
    Sphere::generate_table(cf);
    Nibble nb;
    
    SQLite::Statement q(*nb.conn, "SELECT 1 FROM " + cf.Get("table-names", "sphere", "") + " LIMIT 1");
    EXPECT_TRUE(q.executeStep());
    EXPECT_TRUE(nb.does_table_exist(cf.Get("table-names", "sphere", "")));
    nb.select_table(cf.Get("table-names", "sphere", ""));
    
    std::string schema, fields;
    nb.find_attributes(schema, fields);
    EXPECT_EQ(schema, "label_a INT, label_b INT, label_c INT, a FLOAT, i FLOAT");
    EXPECT_EQ(fields, "label_a, label_b, label_c, a, i");
}

/// Check that the entries in the SphericalTriangle table are correct.
TEST(SphericalTriangle, TableCorrectEntries) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    INIReader cf(std::getenv("HOKU_CONFIG_INI") ? std::string(std::getenv("HOKU_CONFIG_INI")) :
                 std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../CONFIG.ini");
    Sphere::generate_table(cf);
    Chomp ch;
    ch.select_table(cf.Get("table-names", "sphere", ""));
    
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532)};
    std::sort(b.begin(), b.end(), [] (const Star &s_1, const Star &s_2) -> bool {
        return s_1.get_label() < s_2.get_label();
    });
    double a = Trio::spherical_area(b[0], b[1], b[2]).result;
    double i = Trio::spherical_moment(b[0], b[1], b[2]).result;
    
    Nibble::tuples_d t = ch.search_table("a, i", "label_a = " + std::to_string(b[0].get_label()) + " AND label_b = "
                                                 + std::to_string(b[1].get_label()) + " AND label_c = "
                                                 + std::to_string(b[2].get_label()), 1);
    ASSERT_EQ(t.size(), 1);
    ASSERT_EQ(t[0].size(), 2);
    EXPECT_NEAR(i, t[0][1], 1.0e-8);
    EXPECT_NEAR(a, t[0][0], 1.0e-4);
}

///// No test is performed here. This is just to see how long the entire table will load into memory.
//TEST(SphericalTriangle, ChompInMemory) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
//        INIReader cf(std::getenv("HOKU_CONFIG_INI") ? std::string(std::getenv("HOKU_CONFIG_INI")) :
//                 std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../CONFIG.ini");
//    Chomp ch(cf.Get("table-names", "sphere", ""), cf.Get("table-focus", "sphere", ""));
//}

/// Check that the query_for_trios method returns the correct result.
TEST(SphericalTriangle, TriosQueryCleanInput) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 15);
    Identification::Parameters p;
    p.sigma_1 = p.sigma_2 = 0.000000001, p.table_name = "SPHERE_20";
    Sphere a(input, p);
    BaseTriangle::trio_vector_either b = a.query_for_trios({0, 1, 2});

    ASSERT_EQ(b.error, 0);
    EXPECT_EQ(b.result.size(), 1);
    EXPECT_THAT(b.result[0], Contains(ch.query_hip((*input.b)[0].get_label())));
    EXPECT_THAT(b.result[0], Contains(ch.query_hip((*input.b)[1].get_label())));
    EXPECT_THAT(b.result[0], Contains(ch.query_hip((*input.b)[2].get_label())));
}

/// Check that a clean input returns the expected query result.
TEST(SphericalTriangle, TrialCleanQuery) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Sphere::Parameters p;
    p.sigma_1 = p.sigma_2 = 10e-8, p.no_reduction = false, p.table_name = "SPHERE_20";
    Sphere a(Benchmark::black(), p);
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532)};
    
    std::vector<Identification::labels_list> d = a.query(b);
    EXPECT_THAT(d, Contains(Identification::labels_list {95498, 102531, 102532}));
}

/// Check that a clean input returns the correct stars from a set of candidates.
TEST(SphericalTriangle, TrialCleanReduction) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Sphere::Parameters p;
    p.sigma_1 = p.sigma_2 = 10e-10, p.sql_limit = 1000000, p.nu = std::make_shared<unsigned int> (0);
    p.table_name = "SPHERE_20";
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
        ch.query_hip(101909)};
    
    Benchmark i(b, b[0], 20);
    Sphere a(i, p);
    EXPECT_THAT(a.reduce().result, UnorderedElementsAre(b[0], b[1], b[2]));
}

/// Check that a clean input returns the expected identification of stars.
TEST(SphericalTriangle, TrialCleanIdentify) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Sphere::Parameters p;
    p.nu = std::make_shared<unsigned int>(0);
    p.sigma_1 = p.sigma_2 = 10e-9;
    p.sigma_4 = 0.000001, p.table_name = "SPHERE_20";
    
    Rotation q = Rotation::chance();
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
        ch.query_hip(101909)};
    Star::list c = {Rotation::rotate(b[0], q), Rotation::rotate(b[1], q), Rotation::rotate(b[2], q),
        Rotation::rotate(b[3], q), Rotation::rotate(b[4], q)};
    
    Sphere a(Benchmark(c, c[0], 20), p);
    Star::list h = a.identify().result;
    EXPECT_THAT(h, Contains(Star::define_label(c[0], 102531)));
    EXPECT_THAT(h, Contains(Star::define_label(c[1], 95498)));
    EXPECT_THAT(h, Contains(Star::define_label(c[2], 102532)));
}

/// Check that the nu_max is respected in identification.
TEST(SphericalTriangle, TrialExceededNu) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 20);
    input.shift_light(static_cast<unsigned int> (input.b->size()), 0.001);
    Sphere::Parameters p;
    p.nu = std::make_shared<unsigned int>(0), p.nu_max = 10;
    p.sigma_1 = p.sigma_2 = 1.0e-19;
    p.sigma_4 = 1.0e-19, p.table_name = "SPHERE_20";
    Sphere a(input, p);
    
    EXPECT_EQ(a.identify().error, Sphere::EXCEEDED_NU_MAX_EITHER);
    EXPECT_GE(*(a.parameters->nu), p.nu_max + 1);
}

/// Check that the correct result is returned when no map is found.
TEST(SphericalTriangle, TrialNoMapFound) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 7);
    input.shift_light(static_cast<unsigned int> (input.b->size()), 0.001);
    Sphere::Parameters p;
    p.nu = std::make_shared<unsigned int>(0), p.nu_max = std::numeric_limits<unsigned int>::max();
    p.sigma_1 = p.sigma_2 = p.sigma_4 = 1.0e-19, p.table_name = "SPHERE_20";
    Sphere a(input, p);
    
    EXPECT_EQ(a.identify().error, Sphere::NO_CONFIDENT_A_EITHER);
}