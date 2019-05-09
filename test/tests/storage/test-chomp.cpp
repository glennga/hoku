/// @file test-chomp.cpp
/// @author Glenn Galvizo
///
/// Source file for all Chomp class unit tests.

#define ENABLE_TESTING_ACCESS

#define _USE_MATH_DEFINES

#include "gmock/gmock.h"
#include <fstream>
#include <libgen.h>

#include "storage/chomp.h"


using testing::PrintToString;

MATCHER_P2(IsBetweenChomp, a, b,
           std::string(negation ? "isn't" : "is") + " between " + PrintToString(a) + " and " + PrintToString(b)) {
    return a <= arg && arg <= b;
}

TEST(Chomp, ConstructorNoArguments) {
    Nibble nb("/tmp/nibble.db");
    (*nb.conn).exec("DROP TABLE IF EXISTS HIP");
    (*nb.conn).exec("DROP TABLE IF EXISTS HIP_BRIGHT");

    // These tables should not exist now.
    EXPECT_FALSE(nb.does_table_exist("HIP"));
    EXPECT_FALSE(nb.does_table_exist("HIP_BRIGHT"));

    // Ensure the fields are set correctly.
    Chomp ch = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
            .limited_by_magnitude(4.5)
            .using_current_time("01-2018")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .build();

    // Tables should exist in nibble.db
    EXPECT_TRUE(ch.does_table_exist("HIP"));
    EXPECT_TRUE(ch.does_table_exist("HIP_BRIGHT"));
}

TEST(Chomp, StarTableCorrectAngleBetweenStars) {
    Chomp ch = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .build();

    Star a = ch.query_hip(31700), b = ch.query_hip(32349); // NU.03 Canis Majoris and Sirius.
    EXPECT_NEAR(2.3011, Vector3::Angle(a.get_vector(), b.get_vector()) * (180.0 / M_PI), 0.005);
}

TEST(Chomp, QueryQuery31700) {
    Chomp ch = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .build();
    Nibble::tuples_d a = ch.search_table("*", "label = 31700", 1); // NU.03 Canis Majoris.

    EXPECT_NEAR(a[0][0], 99.6708, 0.5);
    EXPECT_NEAR(a[0][1], -18.2592, 0.5);
    EXPECT_FLOAT_EQ(a[0][5], 4.5975);
}

TEST(Chomp, QueryQuery32349) {
    Chomp ch = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .build();
    Nibble::tuples_d a = ch.search_table("*", "label = 32349", 1); // Sirius.

    EXPECT_NEAR(a[0][0], 101.4875, 0.5);
    EXPECT_NEAR(a[0][1], -16.7439, 0.5);
    EXPECT_FLOAT_EQ(a[0][5], -1.0876);
}

TEST(Chomp, QueryHip) {
    Star a = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .build()
            .query_hip(3);

    EXPECT_DOUBLE_EQ(a[0], 0.778689441368632);
    EXPECT_DOUBLE_EQ(a[1], 6.84644278384085e-05);
    EXPECT_DOUBLE_EQ(a[2], 0.627409554608177);
}

TEST(Chomp, NearbyBrightStars) {
    Chomp ch = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .build();
    Star focus = Star::chance();
    std::vector<Star> nearby = ch.nearby_bright_stars(focus, 7.5, 30);

    for (int q = 0; q < nearby.size(); q++) {
        EXPECT_TRUE(Star::within_angle(nearby[q], focus, 7.5));
    }
}

TEST(Chomp, NearbyHipStars) {
    Chomp ch = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .build();
    Star focus = Star::chance();
    std::vector<Star> nearby = ch.nearby_hip_stars(focus, 5, 100);
    for (int q = 0; q < 10; q++) {
        EXPECT_TRUE(Star::within_angle(nearby[q], focus, 5));
    }
}