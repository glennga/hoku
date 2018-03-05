/// @file test-pyramid.cpp
/// @author Glenn Galvizo
///
/// Source file for all Pyramid class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include "identification/pyramid.h"
#include "gmock/gmock.h"

// Import several matchers from Google Mock.
using testing::UnorderedElementsAre;
using testing::Each;
using testing::Contains;
using testing::Not;

/// Check that the constructor correctly sets the object's attributes.
TEST(PyramidConstructor, Constructor) {
    Chomp ch;
    Benchmark input(ch, 20);
    Pyramid::Parameters p = {0.01, 10, false, true, 0.1, 10, std::make_shared<unsigned int>(0), Rotation::svd, "H"};
    Pyramid a(input, p);
    
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

/// Check the existence and the structure of the Pyramid table.
TEST(Pyramid, ExistenceStructure) {
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
    Pyramid::generate_table(cf);
    Nibble nb;
    
    SQLite::Statement q(*nb.conn, "SELECT 1 FROM " + cf.Get("table-names", "pyramid", "") + " LIMIT 1");
    EXPECT_TRUE(q.executeStep());
    EXPECT_NO_THROW(nb.select_table(cf.Get("table-names", "pyramid", ""), true););
    
    std::string schema, fields;
    nb.find_attributes(schema, fields);
    EXPECT_EQ(schema, "label_a INT, label_b INT, theta FLOAT");
    EXPECT_EQ(fields, "label_a, label_b, theta");
}

/// Check that the entries in the Pyramid table are correct.
TEST(PyramidTable, CorrectEntries) {
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
    Pyramid::generate_table(cf);
    Chomp ch;
    ch.select_table(cf.Get("table-names", "pyramid", ""));
    
    Star::list b = ch.bright_as_list(); // This list is ordered by label, do not need to worry about reversed case.
    double theta = (180.0 / M_PI) * Vector3::Angle(b[0], b[1]);
    double theta_2 = ch.search_single("theta", "label_a = " + std::to_string(b[0].get_label()) + " AND label_b = "
                                               + std::to_string(b[1].get_label()));
    EXPECT_EQ(theta, theta_2);
}

/// Check that the query_for_pairs method returns an entry with the correct result.
TEST(PyramidQuery, Pairs) {
    Chomp ch;
    Benchmark input(ch, 15);
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    p.sigma_query = 0.00000000001;
    
    // It is known that the angle between b_0 and b_1 here is < 20.
    double a = (180.0 / M_PI) * Vector3::Angle(input.b[0], input.b[1]);
    Pyramid::labels_list_list b = Pyramid(input, p).query_for_pairs(a);
    Pyramid::labels_list c = {input.b[0].get_label(), input.b[1].get_label()};
    EXPECT_EQ(b.size(), 1);
    EXPECT_THAT(c, Contains(b[0][0]));
    EXPECT_THAT(c, Contains(b[0][1]));
}

/// Check that the common method returns the correct common stars, and filters out not correct common stars.
TEST(PyramidDualCommon, CleanInput) {
    Chomp ch;
    
    Pyramid::labels_list_list ei = {{3, 100}, {3, 413}, {7, 87}};
    Pyramid::labels_list_list ej = {{3, 2}, {3, 5}, {13, 87}};
    Pyramid::labels_list_list ek = {{100, 5}, {3, 7352}, {987, 512}};
    Pyramid a(Benchmark(ch, 20), Pyramid::DEFAULT_PARAMETERS);
    
    Star::list b = a.common(ei, ej, {});
    EXPECT_THAT(b, Contains(ch.query_hip(3)));
    EXPECT_THAT(b, Contains(ch.query_hip(87)));
    
    Star::list c = a.common(ej, ek, b);
    EXPECT_THAT(c, Not(Contains(ch.query_hip(3))));
    EXPECT_THAT(c, Contains(ch.query_hip(5)));
    
    Star::list d = a.common(ei, ek, b);
    EXPECT_THAT(d, Not(Contains(ch.query_hip(3))));
    EXPECT_THAT(d, Contains(ch.query_hip(100)));
    
    Star::list e = a.common(ei, ej, b);
    EXPECT_THAT(e, Contains(Pyramid::NO_COMMON_FOUND[0]));
}

/// Check that the common method returns the correct common stars, and filters out not correct common stars.
TEST(PyramidThreeCommon, CleanInput) {
    Chomp ch;
    
    Pyramid::labels_list_list ei = {{3, 100}, {3, 413}, {7, 87}};
    Pyramid::labels_list_list ej = {{3, 2}, {3, 5}, {13, 87}};
    Pyramid::labels_list_list ek = {{100, 5}, {3, 7352}, {987, 512}};
    Pyramid a(Benchmark(ch, 20), Pyramid::DEFAULT_PARAMETERS);
    
    Star::list b = a.common(ei, ej, ek, {});
    EXPECT_EQ(b.size(), 1);
    EXPECT_THAT(b, Contains(ch.query_hip(3)));
}

/// Check that the verification works as intended with clean input.
TEST(PyramidVerify, CleanInput) {
    Chomp ch;
    Benchmark input(ch, 20);
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    Rotation q = Rotation::chance();
    p.sigma_query = 0.00000000001;
    
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
        ch.query_hip(101909)};
    Star::list c = {Rotation::rotate(b[0], q), Rotation::rotate(b[1], q), Rotation::rotate(b[2], q),
        Rotation::rotate(b[3], q), Rotation::rotate(b[4], q)};
    input.b = c, input.center = c[0];
    
    EXPECT_TRUE(Pyramid(input, p).verification(Star::trio {b[0], b[1], b[2]}, Star::trio {c[0], c[1], c[2]}));
    EXPECT_FALSE(
        Pyramid(input, p).verification(Star::trio {b[0], b[1], ch.query_hip(3)}, Star::trio {c[0], c[1], c[2]}));
}

/// Check that the catalog star finder determines the correct stars.
TEST(PyramidFind, CatalogStars) {
    Chomp ch;
    Benchmark input(ch, 20);
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    Rotation q = Rotation::chance();
    p.sigma_query = 0.0000000001;
    
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
        ch.query_hip(101909)};
    Star::list c = {Rotation::rotate(b[0], q), Rotation::rotate(b[1], q), Rotation::rotate(b[2], q),
        Rotation::rotate(b[3], q), Rotation::rotate(b[4], q)};
    input.b = c, input.center = c[0];
    
    Star::trio k = Pyramid(input, p).find_catalog_stars(Star::trio {c[0], c[1], c[2]});
    EXPECT_EQ(k[0], b[0]);
    EXPECT_EQ(k[1], b[1]);
    EXPECT_EQ(k[2], b[2]);
}

/// Check that the reduction step flag is upheld when not applied with the catalog star finder.
TEST(PyramidFind, NoReduction) {
    Chomp ch;
    Benchmark input(ch, 20);
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    Rotation q = Rotation::chance();
    p.sigma_query = 0.01, p.no_reduction = true;
    
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
        ch.query_hip(101909)};
    Star::list c = {Rotation::rotate(b[0], q), Rotation::rotate(b[1], q), Rotation::rotate(b[2], q),
        Rotation::rotate(b[3], q), Rotation::rotate(b[4], q)};
    input.b = c, input.center = c[0];
    Star::trio k = Pyramid(input, p).find_catalog_stars(Star::trio {c[0], c[1], c[2]});
    EXPECT_NE(k[0], b[0]);
    EXPECT_NE(k[1], b[1]);
    EXPECT_NE(k[2], b[2]);
}

/// Check that the brightest set is returned if desired.
TEST(PyramidFind, SortBrighteness) {
    Chomp ch;
    Benchmark input(ch, 20);
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS, p2 = Pyramid::DEFAULT_PARAMETERS;
    p.sigma_query = 0.0001, p.no_reduction = true, p.favor_bright_stars = true, p2.sigma_query = 0.000000000001;
    
    Star::trio k = Pyramid(input, p).find_catalog_stars(Star::trio {input.b[0], input.b[1], input.b[2]});
    Star::trio m = Pyramid(input, p2).find_catalog_stars(Star::trio {input.b[0], input.b[1], input.b[2]});
    
    EXPECT_LT(k[0].get_magnitude() + k[1].get_magnitude() + k[2].get_magnitude(),
              m[0].get_magnitude() + m[1].get_magnitude() + m[2].get_magnitude());
}

/// Check that the find method fails when expected.
TEST(PyramidFind, ExpectedFailure) {
    Chomp ch;
    Benchmark input(ch, 20);
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    p.sigma_query = 0.1;
    
    // Unsure how to test passing the |R| = 1 restriction but not the FBR. Only testing former.
    Star::trio k = Pyramid(input, p).find_catalog_stars(Star::trio {input.b[0], input.b[1], input.b[2]});
    
    EXPECT_EQ(k[0], Pyramid::NO_CONFIDENT_R_FOUND[0]);
    EXPECT_EQ(k[1], Pyramid::NO_CONFIDENT_R_FOUND[1]);
    EXPECT_EQ(k[2], Pyramid::NO_CONFIDENT_R_FOUND[2]);
}

/// Check that the identification method returns a non-confident map when appropriate.
TEST(PyramidIdentify, ExpectedFailure) {
    Chomp ch;
    Benchmark input(ch, 20);
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    p.sigma_query = std::numeric_limits<double>::epsilon();
    
    Star::list k = Pyramid(input, p).identify_as_list({input.b[0], input.b[1], input.b[2]});
    EXPECT_EQ(k.size(), 1);
    EXPECT_EQ(k[0], Pyramid::NO_CONFIDENT_A[0]);
    EXPECT_EQ(k[0].get_label(), Pyramid::NO_CONFIDENT_A[0].get_label());
}

/// Check that the identification method returns the correct stars.
TEST(PyramidIdentify, CleanInput) {
    Chomp ch;
    Benchmark input(ch, 20);
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    Rotation q = Rotation::chance();
    p.sigma_query = 0.0000000001;
    
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
        ch.query_hip(101909)};
    Star::list c = {Star::reset_label(Rotation::rotate(b[0], q)), Star::reset_label(Rotation::rotate(b[1], q)),
        Star::reset_label(Rotation::rotate(b[2], q)), Star::reset_label(Rotation::rotate(b[3], q)),
        Star::reset_label(Rotation::rotate(b[4], q))};
    input.b = c, input.center = c[0];
    
    Star::list k = Pyramid(input, p).identify_as_list(c);
    EXPECT_EQ(k[0], Rotation::rotate(b[0], q));
    EXPECT_EQ(k[1], Rotation::rotate(b[1], q));
    EXPECT_EQ(k[2], Rotation::rotate(b[2], q));
    EXPECT_EQ(k[0].get_label(), b[0].get_label());
    EXPECT_EQ(k[1].get_label(), b[1].get_label());
    EXPECT_EQ(k[2].get_label(), b[2].get_label());
}

/// Check that a clean input returns the expected query result.
TEST(PyramidTrial, CleanQuery) {
    Chomp ch;
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    p.sigma_query = 0.00000000001;
    Pyramid a(Benchmark::black(), p);
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532)};
    
    std::vector<Identification::labels_list> d = a.query(b);
    EXPECT_THAT(d, Contains(Identification::labels_list {102531, 95498, 102532}));
}

/// Check that a clean input returns the correct stars from a set of candidates.
TEST(PyramidTrial, CleanReduction) {
    Chomp ch;
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    p.sigma_query = 10e-10, p.sql_limit = 1000000;
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
        ch.query_hip(101909)};
    
    Benchmark i(b, b[0], 20);
    Pyramid a(i, p);
    EXPECT_THAT(a.reduce(), UnorderedElementsAre(102531, 95498, 102532));
}

/// Check that a clean input returns the expected identification of stars.
TEST(PyramidTrial, CleanIdentify) {
    Chomp ch;
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    p.nu = std::make_shared<unsigned int>(0);
    p.sigma_query = 10e-9;
    p.sigma_overlay = 0.000001;
    
    Rotation q = Rotation::chance();
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
        ch.query_hip(101909)};
    Star::list c = {Rotation::rotate(b[0], q), Rotation::rotate(b[1], q), Rotation::rotate(b[2], q),
        Rotation::rotate(b[3], q), Rotation::rotate(b[4], q)};
    
    Pyramid a(Benchmark(c, c[0], 20), p);
    Star::list h = a.identify();
    EXPECT_THAT(h, Contains(Star::define_label(c[0], 102531)));
    EXPECT_THAT(h, Contains(Star::define_label(c[1], 95498)));
    EXPECT_THAT(h, Contains(Star::define_label(c[2], 102532)));
}

/// Check that the nu_max is respected in identification.
TEST(PyramidTrial, ExceededNu) {
    Chomp ch;
    Benchmark input(ch, 15);
    input.shift_light(static_cast<unsigned int> (input.b.size()), 0.001);
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    p.nu = std::make_shared<unsigned int>(0), p.nu_max = 10;
    p.sigma_query = std::numeric_limits<double>::epsilon();
    p.sigma_overlay = std::numeric_limits<double>::epsilon();
    Pyramid a(input, p);
    
    EXPECT_EQ(a.identify()[0], Pyramid::EXCEEDED_NU_MAX[0]);
    EXPECT_EQ(*p.nu, p.nu_max + 1);
}

/// Check that the correct result is returned when no map is found.
TEST(PyramidTrial, NoMapFound) {
    Chomp ch;
    Benchmark input(ch, 7);
    input.shift_light(static_cast<unsigned int> (input.b.size()), 0.001);
    Pyramid::Parameters p = Pyramid::DEFAULT_PARAMETERS;
    p.nu = std::make_shared<unsigned int>(0), p.nu_max = std::numeric_limits<unsigned int>::max();
    p.sigma_query = std::numeric_limits<double>::epsilon();
    p.sigma_overlay = std::numeric_limits<double>::epsilon();
    Pyramid a(input, p);
    
    EXPECT_EQ(a.identify()[0], Pyramid::NO_CONFIDENT_A[0]);
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