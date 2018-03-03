/// @file test-planar-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for all PlanarTriangle class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "gmock/gmock.h"

#include "identification/planar-triangle.h"
#include "math/trio.h"

// Import several matchers from Google Mock.
using testing::UnorderedElementsAre;
using testing::Each;
using testing::Contains;

/// Check that the constructor correctly sets the object's attributes.
TEST(PlanarTriangleConstructor, Constructor) {
    Chomp ch;
    Benchmark input(ch, 20);
    Plane::Parameters p = {0.01, 10, false, true, 0.1, 10, std::make_shared<unsigned int>(0), Rotation::quest, "H"};
    Plane a(input, p);
    
    EXPECT_EQ(a.fov, 20);
    EXPECT_EQ(a.ch.table, "H");
    EXPECT_EQ(a.parameters.sigma_query, p.sigma_query);
    EXPECT_EQ(a.parameters.sql_limit, p.sql_limit);
    EXPECT_EQ(a.parameters.no_reduction, p.no_reduction);
    EXPECT_EQ(a.parameters.favor_bright_stars, p.favor_bright_stars);
    EXPECT_EQ(a.parameters.nu_max, p.nu_max);
    EXPECT_EQ(a.parameters.nu, p.nu);
    EXPECT_EQ(a.parameters.f, p.f);
    EXPECT_EQ(a.parameters.table_name, p.table_name);
}

/// Check the existence and the structure of the PlanarTriangle table.
TEST(PlanarTriangleTable, ExistenceStructure) {
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
    Plane::generate_table(cf);
    Nibble nb;
    
    SQLite::Statement q(*nb.conn, "SELECT 1 FROM " + cf.Get("table-names", "plane", "") + " LIMIT 1");
    EXPECT_TRUE(q.executeStep());
    EXPECT_NO_THROW(nb.select_table(cf.Get("table-names", "plane", ""), true););
    
    std::string schema, fields;
    nb.find_attributes(schema, fields);
    EXPECT_EQ(schema, "label_a INT, label_b INT, label_c INT, a FLOAT, i FLOAT");
    EXPECT_EQ(fields, "label_a, label_b, label_c, a, i");
}

/// Check that the entries in the PlanarTriangle table are correct.
TEST(PlanarTriangleTable, CorrectEntries) {
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
    Plane::generate_table(cf);
    Chomp ch;
    ch.select_table(cf.Get("table-names", "plane", ""));
    
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532)};
    double a = Trio::planar_area(b[0], b[1], b[2]);
    double i = Trio::planar_area(b[0], b[1], b[2]);
    
    Nibble::tuples_d t = ch.search_table("a, i", "label_a = " + std::to_string(b[0].get_label()) + " AND label_b = "
                                                 + std::to_string(b[1].get_label()) + " AND label_c = "
                                                 + std::to_string(b[2].get_label()), 1);
    ASSERT_EQ(t.size(), 1);
    ASSERT_EQ(t[0].size(), 2);
    EXPECT_FLOAT_EQ(a, t[0][0]);
    EXPECT_FLOAT_EQ(i, t[0][1]);
}

/// Check that the query_for_trios method returns the correct result.
TEST(PlanarTriangleTriosQuery, CleanInput) {

}

/// Check that a clean input returns the expected query result.
TEST(PlanarTriangleTrial, CleanQuery) {

}

/// Check that a clean input returns the correct stars from a set of candidates.
TEST(PlanarTriangleTrial, CleanReduction) {

}

/// Check that a clean input returns the expected identification of stars.
TEST(PlanarTriangleTrial, CleanIdentify) {

}
/// Check that the nu_max is respected in identification.
TEST(PlanarTriangleTrial, ExceededNu) {

}

/// Check that the correct result is returned when no map is found.
TEST(PlanarTriangleTrial, NoMapFound) {

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