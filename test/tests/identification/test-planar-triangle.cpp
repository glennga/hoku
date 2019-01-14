/// @file test-planar-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for all PlanarTriangle class unit tests.

#define ENABLE_TESTING_ACCESS

#include "gmock/gmock.h"

#include "identification/planar-triangle.h"
#include "math/trio.h"

// Import several matchers from Google Mock.
using testing::UnorderedElementsAre;
using testing::Each;
using testing::Contains;

/// Check that the constructor correctly sets the object's attributes.
TEST(PlanarTriangle, Constructor) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 20);
    Plane::Parameters p = {0.01, 0.0001, 0.000001, 0.1, 10, false, true, 10, std::make_shared<unsigned int>(0),
        Rotation::svd, "H"};
    Plane a(input, p);
    
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

/// Check the existence and the structure of the PlanarTriangle table.
TEST(PlanarTriangle, TableExistenceStructure) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
    Plane::generate_table(cf);
    Nibble nb;
    
    SQLite::Statement q(*nb.conn, "SELECT 1 FROM " + cf.Get("table-names", "plane", "") + " LIMIT 1");
    EXPECT_TRUE(q.executeStep());
    EXPECT_TRUE(nb.does_table_exist(cf.Get("table-names", "plane", "")));
    nb.select_table(cf.Get("table-names", "plane", ""));
    
    std::string schema, fields;
    nb.find_attributes(schema, fields);
    EXPECT_EQ(schema, "label_a INT, label_b INT, label_c INT, a FLOAT, i FLOAT");
    EXPECT_EQ(fields, "label_a, label_b, label_c, a, i");
}

/// Check that the entries in the PlanarTriangle table are correct.
TEST(PlanarTriangle, TableCorrectEntries) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
    Plane::generate_table(cf);
    Chomp ch;
    ch.select_table(cf.Get("table-names", "plane", ""));
    
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532)};
    std::sort(b.begin(), b.end(), [] (const Star &s_1, const Star &s_2) -> bool {
        return s_1.get_label() < s_2.get_label();
    });
    double a = Trio::planar_area(b[0], b[1], b[2]);
    double i = Trio::planar_moment(b[0], b[1], b[2]);
    
    Nibble::tuples_d t = ch.search_table("a, i", "label_a = " + std::to_string(b[0].get_label()) + " AND label_b = "
                                                 + std::to_string(b[1].get_label()) + " AND label_c = "
                                                 + std::to_string(b[2].get_label()), 1);
    ASSERT_EQ(t.size(), 1);
    ASSERT_EQ(t[0].size(), 2);
    EXPECT_FLOAT_EQ(a, t[0][0]);
    EXPECT_NEAR(i, t[0][1], 1.0e-8);
}

///// No test is performed here. This is just to see how long the entire table will load into memory.
//TEST(PlanarTriangle, ChompInMemory) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
//    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
//    Chomp ch(cf.Get("table-names", "plane", ""), cf.Get("table-focus", "plane", ""));
//}

/// Check that the query_for_trios method returns the correct result.
TEST(PlanarTriangle, TriosQueryCleanInput) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 15);
    Identification::Parameters p;
    p.sigma_1 = p.sigma_2 = 0.000000001, p.table_name = "PLANE_20";
    Plane a(input, p);
    BaseTriangle::trio_vector_either b = a.query_for_trios({0, 1, 2});

    ASSERT_EQ(b.error, 0);
    EXPECT_EQ(b.result.size(), 1);
    EXPECT_THAT(b.result[0], Contains(ch.query_hip((*input.b)[0].get_label())));
    EXPECT_THAT(b.result[0], Contains(ch.query_hip((*input.b)[1].get_label())));
    EXPECT_THAT(b.result[0], Contains(ch.query_hip((*input.b)[2].get_label())));
}

/// Check that a clean input returns the expected query result.
TEST(PlanarTriangle, TrialCleanQuery) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Plane::Parameters p;
    p.sigma_1 = p.sigma_2 = 10e-8, p.no_reduction = false, p.table_name = "PLANE_20";
    Plane a(Benchmark::black(), p);
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532)};
    
    std::vector<Identification::labels_list> d = a.query(b);
    EXPECT_THAT(d, Contains(Identification::labels_list {95498, 102531, 102532}));
}

/// Check that a clean input returns the correct stars from a set of candidates.
TEST(PlanarTriangle, TrialCleanReduction) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Plane::Parameters p;
    p.sigma_1 = p.sigma_2 = 10e-10, p.sql_limit = 1000000, p.nu = std::make_shared<unsigned int>(0);
    p.table_name = "PLANE_20";
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
        ch.query_hip(101909)};
    
    Benchmark i(b, b[0], 20);
    Plane a(i, p);
    EXPECT_THAT(a.reduce().result, UnorderedElementsAre(b[0], b[1], b[2]));
}

/// Check that a clean input returns the expected identification of stars.
TEST(PlanarTriangle, TrialCleanIdentify) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Plane::Parameters p;
    p.nu = std::make_shared<unsigned int>(0);
    p.sigma_1 = p.sigma_2 = 10e-9;
    p.sigma_4 = 0.000001, p.table_name = "PLANE_20";
    
    Rotation q = Rotation::chance();
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
        ch.query_hip(101909)};
    Star::list c = {Rotation::rotate(b[0], q), Rotation::rotate(b[1], q), Rotation::rotate(b[2], q),
        Rotation::rotate(b[3], q), Rotation::rotate(b[4], q)};
    
    Plane a(Benchmark(c, c[0], 20), p);
    Plane::stars_either h = a.identify();
    EXPECT_THAT(h.result, Contains(Star::define_label(c[0], 102531)));
    EXPECT_THAT(h.result, Contains(Star::define_label(c[1], 95498)));
    EXPECT_THAT(h.result, Contains(Star::define_label(c[2], 102532)));
}

/// Check that the nu_max is respected in identification.
TEST(PlanarTriangle, TrialExceededNu) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 20);
    input.shift_light(static_cast<unsigned int> (input.b->size()), 0.001);
    Plane::Parameters p;
    p.nu_max = 10, p.sigma_1 = p.sigma_2 = 1.0e-19;
    p.sigma_4 = 1.0e-19, p.table_name = "PLANE_20";
    Plane a(input, p);
    
    EXPECT_EQ(a.identify().error, Plane::EXCEEDED_NU_MAX_EITHER);
    EXPECT_GE(*(a.parameters->nu), p.nu_max + 1);
}

/// Check that the correct result is returned when no map is found.
TEST(PlanarTriangle, TrialNoMapFound) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 7);
    input.shift_light(static_cast<unsigned int> (input.b->size()), 0.001);
    input.b->resize(10);
    Plane::Parameters p;
    p.nu = std::make_shared<unsigned int>(0), p.nu_max = std::numeric_limits<unsigned int>::max();
    p.sigma_1 = p.sigma_2 = 1.0e-19;
    p.sigma_4 = 1.0e-19, p.table_name = "PLANE_20";
    Plane a(input, p);
    
    EXPECT_EQ(a.identify().error, Plane::NO_CONFIDENT_A_EITHER);
}