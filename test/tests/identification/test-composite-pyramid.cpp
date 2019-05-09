/// @file test-composite-pyramid.cpp
/// @author Glenn Galvizo
///
/// Source file for all Composite class unit tests.

#define ENABLE_TESTING_ACCESS

#include <libgen.h>
#include "gmock/gmock.h"

#include "identification/composite-pyramid.h"
#include "math/trio.h"

// Import several matchers from Google Mock.
using testing::UnorderedElementsAre;
using testing::Each;
using testing::Contains;
using testing::Not;

/// Check that the constructor correctly sets the object's attributes.
TEST(CompositePyramid, Constructor) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 20);
    Composite::Parameters p = {0.01, 0.00001, 0.000001, 0.001, 10, false, true, 10, std::make_shared<unsigned int>(0),
                               Rotation::svd, "H"};
    Composite a(input, p);

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

/// Check the existence and the structure of the Composite table.
TEST(CompositePyramid, ExistenceStructure) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    INIReader cf(std::getenv("HOKU_CONFIG_INI") ? std::string(std::getenv("HOKU_CONFIG_INI")) :
                 std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../CONFIG.ini");
    Composite::generate_table(cf);
    Nibble nb;

    SQLite::Statement q(*nb.conn, "SELECT 1 FROM " + cf.Get("table-names", "composite", "") + " LIMIT 1");
    EXPECT_TRUE(q.executeStep());
    EXPECT_TRUE(nb.does_table_exist(cf.Get("table-names", "composite", "")));
    nb.select_table(cf.Get("table-names", "composite", ""));

    std::string schema, fields;
    nb.find_attributes(schema, fields);
    EXPECT_EQ(schema, "label_a INT, label_b INT, label_c INT, a FLOAT, i FLOAT");
    EXPECT_EQ(fields, "label_a, label_b, label_c, a, i");
}

/// Check that the entries in the Composite table are correct.
TEST(CompositePyramid, TableCorrectEntries) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    INIReader cf(std::getenv("HOKU_CONFIG_INI") ? std::string(std::getenv("HOKU_CONFIG_INI")) :
                 std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../CONFIG.ini");
    Composite::generate_table(cf);
    Chomp ch;
    ch.select_table(cf.Get("table-names", "composite", ""));

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
//TEST(CompositePyramid, ChompInMemory) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
//        INIReader cf(std::getenv("HOKU_CONFIG_INI") ? std::string(std::getenv("HOKU_CONFIG_INI")) :
//                 std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../CONFIG.ini");
//    Chomp ch(cf.Get("table-names", "composite", ""), cf.Get("table-focus", "composite", ""));
//}

/// Check that the query_for_trios method returns the brightest set.
TEST(CompositePyramid, TriosQueryBrightnessSort) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 15);
    Identification::Parameters p;
    p.sigma_1 = p.sigma_2 = 0.00000001, p.favor_bright_stars = true, p.table_name = "COMPOSITE_20";
    Composite a(input, p);

    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532)};
    std::sort(b.begin(), b.end(), [] (const Star &s_1, const Star &s_2) -> bool {
        return s_1.get_label() < s_2.get_label();
    });
    double a_j = Trio::planar_area(b[0], b[1], b[2]);
    double i = Trio::planar_moment(b[0], b[1], b[2]);
    Composite::labels_list_list f = a.query_for_trios(a_j, i);

    EXPECT_LT(ch.query_hip(f[0][0]).get_magnitude() + ch.query_hip(f[0][1]).get_magnitude()
              + ch.query_hip(f[0][2]).get_magnitude(),
              ch.query_hip(f[1][0]).get_magnitude() + ch.query_hip(f[1][1]).get_magnitude()
              + ch.query_hip(f[1][2]).get_magnitude());

    EXPECT_LT(ch.query_hip(f[1][0]).get_magnitude() + ch.query_hip(f[1][1]).get_magnitude()
              + ch.query_hip(f[1][2]).get_magnitude(),
              ch.query_hip(f[2][0]).get_magnitude() + ch.query_hip(f[2][1]).get_magnitude()
              + ch.query_hip(f[2][2]).get_magnitude());
}

/// Check that the query_for_trios method returns the correct result.
TEST(CompositePyramid, TriosQueryCleanInput) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 15);
    Identification::Parameters p;
    p.sigma_1 = p.sigma_2 = 0.000000001, p.table_name = "COMPOSITE_20";
    Composite a(input, p);

    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532)};
    std::sort(b.begin(), b.end(), [] (const Star &s_1, const Star &s_2) -> bool {
        return s_1.get_label() < s_2.get_label();
    });
    double a_j = Trio::planar_area(b[0], b[1], b[2]);
    double i = Trio::planar_moment(b[0], b[1], b[2]);

    Composite::labels_list_list f = a.query_for_trios(a_j, i);

    EXPECT_EQ(f.size(), 1);
    EXPECT_THAT(f[0], Contains(102531));
    EXPECT_THAT(f[0], Contains(95498));
    EXPECT_THAT(f[0], Contains(102532));
}

/// Check that the verification works as intended with clean input.
TEST(CompositePyramid, VerifyCleanInput) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 20);
    Composite::Parameters p;
    Rotation q = Rotation::chance();
    p.sigma_1 = p.sigma_2 = 0.00000000001, p.table_name = "COMPOSITE_20";

    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
                    ch.query_hip(101909)};
    Star::list c = {Rotation::rotate(b[0], q), Rotation::rotate(b[1], q), Rotation::rotate(b[2], q),
                    Rotation::rotate(b[3], q), Rotation::rotate(b[4], q)};
    input.b = std::make_shared<Star::list>(c), input.center = c[0];

    EXPECT_TRUE(Composite(input, p).verification(Star::trio{b[0], b[1], b[2]}, Star::trio{c[0], c[1], c[2]}));
    EXPECT_FALSE(
            Composite(input, p).verification(Star::trio{b[0], b[1], ch.query_hip(3)}, Star::trio{c[0], c[1], c[2]}));
}

/// Check that the catalog star finder determines the correct stars.
TEST(CompositePyramid, FindCatalogStars) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 20);
    Composite::Parameters p;
    Rotation q = Rotation::chance();
    p.sigma_1 = p.sigma_2 = 0.0000000001, p.sigma_4 = 0.000001, p.table_name = "COMPOSITE_20";

    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
                    ch.query_hip(101909)};
    Star::list c = {Rotation::rotate(b[0], q), Rotation::rotate(b[1], q), Rotation::rotate(b[2], q),
                    Rotation::rotate(b[3], q), Rotation::rotate(b[4], q)};
    input.b = std::make_shared<Star::list>(c), input.center = c[0];

    Composite::trios_either k = Composite(input, p).find_catalog_stars(Star::trio{c[0], c[1], c[2]});
    ASSERT_EQ(k.error, 0);
    EXPECT_EQ(k.result[0], b[0]);
    EXPECT_EQ(k.result[1], b[1]);
    EXPECT_EQ(k.result[2], b[2]);
}

/// Check that the reduction step flag is upheld when not applied with the catalog star finder.
TEST(CompositePyramid, FindNoReduction) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete) 
    Chomp ch;
    Benchmark input(ch, 20);
    Composite::Parameters p;
    Rotation q = Rotation::chance();
    p.sigma_1 = p.sigma_2 = 0.01, p.no_reduction = true, p.sigma_4 = 0.000001, p.table_name = "COMPOSITE_20";

    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
                    ch.query_hip(101909)};
    Star::list c = {Rotation::rotate(b[0], q), Rotation::rotate(b[1], q), Rotation::rotate(b[2], q),
                    Rotation::rotate(b[3], q), Rotation::rotate(b[4], q)};
    input.b = std::make_shared<Star::list>(c), input.center = c[0];
    Composite::trios_either k = Composite(input, p).find_catalog_stars(Star::trio{c[0], c[1], c[2]});
    ASSERT_EQ(k.error, 0);
    EXPECT_NE(k.result[0], b[0]);
    EXPECT_NE(k.result[1], b[1]);
    EXPECT_NE(k.result[2], b[2]);
}

/// Check that the brightest set is returned if desired.
TEST(CompositePyramid, FindSortBrightness) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 20);
    Composite::Parameters p, p2;
    p.sigma_1 = p.sigma_2 = 0.0001, p.no_reduction = true, p.favor_bright_stars = true;
    p2.sigma_1 = p2.sigma_2 = 0.000000000001;
    p.table_name = p2.table_name = "COMPOSITE_20";

    Composite::trios_either k = Composite(input, p).find_catalog_stars(
            Star::trio{(*input.b)[0], (*input.b)[1], (*input.b)[2]});
    Composite::trios_either m = Composite(input, p2).find_catalog_stars(
            Star::trio{(*input.b)[0], (*input.b)[1], (*input.b)[2]});

    EXPECT_LT(k.result[0].get_magnitude() + k.result[1].get_magnitude() + k.result[2].get_magnitude(),
              m.result[0].get_magnitude() + m.result[1].get_magnitude() + m.result[2].get_magnitude());
}

/// Check that the find method fails when expected.
TEST(CompositePyramid, FindExpectedFailure) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 20);
    Composite::Parameters p;
    p.sigma_1 = p.sigma_2 = 0.0001, p.table_name = "COMPOSITE_20";

    Composite::trios_either k = Composite(input, p).find_catalog_stars(
            Star::trio{(*input.b)[0], (*input.b)[1], (*input.b)[2]});
    EXPECT_EQ(k.error, Composite::NO_CONFIDENT_R_FOUND_EITHER);
}

/// Check that the identification method returns a non-confident map when appropriate.
TEST(CompositePyramid, IdentifyExpectedFailure) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 20);
    Composite::Parameters p;
    p.sigma_1 = p.sigma_2 = p.sigma_4 = 1.0e-19, p.table_name = "COMPOSITE_20";

    Composite::stars_either k = Composite(input, p).identify_as_list({(*input.b)[0], (*input.b)[1], (*input.b)[2]});
    EXPECT_EQ(k.error, Composite::NO_CONFIDENT_A_EITHER);
}

/// Check that the identification method returns the correct stars.
TEST(CompositePyramid, IdentifyCleanInput) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 20);
    Composite::Parameters p;
    Rotation q = Rotation::chance();
    p.sigma_1 = p.sigma_2 = 0.00000000001, p.table_name = "COMPOSITE_20";

    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
                    ch.query_hip(101909)};
    Star::list c = {Star::reset_label(Rotation::rotate(b[0], q)), Star::reset_label(Rotation::rotate(b[1], q)),
                    Star::reset_label(Rotation::rotate(b[2], q)), Star::reset_label(Rotation::rotate(b[3], q)),
                    Star::reset_label(Rotation::rotate(b[4], q))};
    input.b = std::make_shared<Star::list>(c), input.center = c[0];

    Composite::stars_either k = Composite(input, p).identify_as_list(c);
    EXPECT_EQ(k.result[0], Rotation::rotate(b[0], q));
    EXPECT_EQ(k.result[1], Rotation::rotate(b[1], q));
    EXPECT_EQ(k.result[2], Rotation::rotate(b[2], q));
    EXPECT_EQ(k.result[0].get_label(), b[0].get_label());
    EXPECT_EQ(k.result[1].get_label(), b[1].get_label());
    EXPECT_EQ(k.result[2].get_label(), b[2].get_label());
}

/// Check that a clean input returns the expected query result.
TEST(CompositePyramid, TrialCleanQuery) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Composite::Parameters p;
    p.sigma_1 = p.sigma_2 = 0.00000000001, p.table_name = "COMPOSITE_20";
    Composite a(Benchmark::black(), p);
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532)};

    std::vector<Identification::labels_list> d = a.query(b);
    EXPECT_THAT(d, Contains(Identification::labels_list{95498, 102531, 102532}));
}

/// Check that a clean input returns the correct stars from a set of candidates.
TEST(CompositePyramid, TrialCleanReduction) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Composite::Parameters p;
    p.sigma_1 = p.sigma_2 = 10e-10, p.sql_limit = 1000000, p.nu = std::make_shared<unsigned int>(0);
    p.table_name = "COMPOSITE_20";
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
                    ch.query_hip(101909)};

    Benchmark i(b, b[0], 20);
    Composite a(i, p);
    EXPECT_THAT(a.reduce().result, UnorderedElementsAre(b[0], b[1], b[2]));
}

/// Check that a clean input returns the expected identification of stars.
TEST(CompositePyramid, TrialCleanIdentify) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Composite::Parameters p;
    p.nu = std::make_shared<unsigned int>(0);
    p.sigma_1 = p.sigma_2 = 10e-10;
    p.sigma_4 = 0.000001, p.table_name = "COMPOSITE_20";

    Rotation q = Rotation::chance();
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
                    ch.query_hip(101909)};
    Star::list c = {Rotation::rotate(b[0], q), Rotation::rotate(b[1], q), Rotation::rotate(b[2], q),
                    Rotation::rotate(b[3], q), Rotation::rotate(b[4], q)};

    Composite a(Benchmark(c, c[0], 20), p);
    Composite::stars_either h = a.identify();
    ASSERT_EQ(h.error, 0);
    EXPECT_THAT(h.result, Contains(Star::define_label(c[0], 102531)));
    EXPECT_THAT(h.result, Contains(Star::define_label(c[1], 95498)));
    EXPECT_THAT(h.result, Contains(Star::define_label(c[2], 102532)));
}

/// Check that the nu_max is respected in identification.
TEST(CompositePyramid, TrialExceededNu) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 15);
    input.shift_light(static_cast<unsigned int> (input.b->size()), 0.001);
    Composite::Parameters p;
    p.nu_max = 10, p.sigma_1 = p.sigma_2 = p.sigma_4 = 1.0e-19, p.table_name = "COMPOSITE_20";
    Composite a(input, p);

    EXPECT_EQ(a.identify().error, Composite::EXCEEDED_NU_MAX_EITHER);
    EXPECT_GT(*(a.parameters->nu), p.nu_max);
}

/// Check that the correct result is returned when no map is found.
TEST(CompositePyramid, TrialNoMapFound) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Benchmark input(ch, 15);
    input.shift_light(static_cast<unsigned int> (input.b->size()), 0.001);
    Composite::Parameters p;
    p.nu = std::make_shared<unsigned int>(0), p.nu_max = std::numeric_limits<unsigned int>::max();
    p.sigma_1 = p.sigma_2 = 1.0e-19, p.sigma_4 = 1.0e-19, p.table_name = "COMPOSITE_20";
    Composite a(input, p);

    EXPECT_EQ(a.identify().error, Composite::NO_CONFIDENT_A_EITHER);
}
