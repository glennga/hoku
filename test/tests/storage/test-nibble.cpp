/// @file test-nibble.cpp
/// @author Glenn Galvizo
///
/// Source file for all Nibble class unit tests. This assumes that the bright star table generator in Chomp works.

#define ENABLE_TESTING_ACCESS

#include <fstream>
#include <libgen.h>
#include "gtest/gtest.h"

#include "storage/chomp.h"

TEST(Nibble, FileExistence) {
    std::remove("/tmp/nibble.db");
    Nibble nb("/tmp/nibble.db");
    std::ifstream nibble("/tmp/nibble.db");
    ASSERT_TRUE(nibble.good());
}

TEST(Nibble, SearchConstrained) {
    std::remove("/tmp/nibble.db");
    Chomp ch = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
            .limited_by_magnitude(4.5)
            .using_current_time("01-2018")
            .build();
    Nibble nb("/tmp/nibble.db");
    nb.select_table("HIP");

    // Check the method with a constraint, given an expected amount and not.
    Nibble::tuples_d a = nb.search_table("i, j, k", "label = 88", 3);
    Nibble::tuples_d b = nb.search_table("i, j, k", "label = 88 or label = 107 ORDER BY label", 6);

    EXPECT_FLOAT_EQ(a[0][0], 0.658552173330720);
    EXPECT_FLOAT_EQ(a[0][1], 0.003092250084512);
    EXPECT_FLOAT_EQ(a[0][2], -0.752528719047187);
    EXPECT_FLOAT_EQ(b[0][0], 0.658552173330720);
    EXPECT_FLOAT_EQ(b[0][1], 0.003092250084512);
    EXPECT_FLOAT_EQ(b[0][2], -0.752528719047187);
    EXPECT_FLOAT_EQ(b[1][0], 0.638255709461383);
    EXPECT_FLOAT_EQ(b[1][1], 0.003719091180710);
    EXPECT_FLOAT_EQ(b[1][2], -0.769815443921941);
}

TEST(Nibble, SearchNotConstrained) {
    std::remove("/tmp/nibble.db");
    Chomp ch = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
            .limited_by_magnitude(4.5)
            .using_current_time("01-2018")
            .build();
    Nibble nb("/tmp/nibble.db");

    nb.select_table("HIP");
    Nibble::tuples_d a = nb.search_table("i", 10000);

    Nibble::tuples_d c = nb.search_table("i, j, k", 3);
    EXPECT_GT(c.size(), 3);
}

TEST(Nibble, SearchSingle) {
    std::remove("/tmp/nibble.db");
    Chomp ch = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
            .limited_by_magnitude(6.0)
            .using_current_time("01-2018")
            .build();

    Nibble nb("/tmp/nibble.db");
    nb.select_table("HIP_BRIGHT");

    EXPECT_NEAR(nb.search_single("i", "label = 88").result, 0.658552173330720, 0.001);
    EXPECT_NEAR(nb.search_single("j", "label = 88").result, 0.003092250084512, 0.001);
    EXPECT_NEAR(nb.search_single("k", "label = 88").result, -0.752528719047187, 0.001);
    EXPECT_EQ(nb.search_single("k", "label = -1").error, Nibble::NO_RESULT_FOUND_EITHER);

    EXPECT_EQ(nb.search_single("COUNT(*)").result, ch.bright_as_list().size());
}

TEST(Nibble, TableCreation) {
    // Clean up our mess (if it exists).
    Nibble nb2("/tmp/nibble.db");
    (*nb2.conn).exec("DROP TABLE IF EXISTS MYTABLE");

    // Nibble gets destroyed when this is done.
    std::unique_ptr<Nibble> nb_p = std::make_unique<Nibble>("/tmp/nibble.db");
    EXPECT_EQ (0, (*nb_p).create_table("MYTABLE", "a int"));
    nb_p.reset(nullptr);

    // Attempting to create a table again should return an error.
    Nibble nb("/tmp/nibble.db");
    EXPECT_EQ(Nibble::TABLE_NOT_CREATED_RET, nb.create_table("MYTABLE", "a int"));

    // Clean up our mess.
    (*nb.conn).exec("DROP TABLE IF EXISTS MYTABLE");
}

TEST(Nibble, TableAttributeRetrieval) {
    std::remove("/tmp/nibble.db");
    Chomp ch = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
            .limited_by_magnitude(4.5)
            .using_current_time("01-2018")
            .build();

    Nibble nb("/tmp/nibble.db");
    EXPECT_EQ (0, nb.create_table("MYTABLE", "a int, b int"));

    std::string schema, fields;
    nb.select_table("MYTABLE");
    nb.find_attributes(schema, fields);

    EXPECT_EQ("a, b", fields);
    EXPECT_EQ("a int, b int", schema);

    // Clean up our mess.
    (*nb.conn).exec("DROP TABLE IF EXISTS MYTABLE");
}

TEST(Nibble, TablePolishIndex) {
    std::remove("/tmp/nibble.db");
    Chomp ch = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
            .limited_by_magnitude(4.5)
            .using_current_time("01-2018")
            .build();

    Nibble nb("/tmp/nibble.db");

    nb.select_table("HIP_BRIGHT");
    nb.sort_and_index("alpha");

    EXPECT_ANY_THROW((*nb.conn).exec("CREATE INDEX HIP_BRIGHT_IDX on HIP_BRIGHT (alpha)"););

    // Delete new table and index.
    (*nb.conn).exec("DROP INDEX HIP_BRIGHT_IDX");
    (*nb.conn).exec("DROP TABLE HIP_BRIGHT");
}

TEST(Nibble, TablePolishDualIndex) {
    std::remove("/tmp/nibble.db");
    Chomp ch = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
            .limited_by_magnitude(6.0)
            .using_current_time("01-2018")
            .build();

    Nibble nb("/tmp/nibble.db");

    nb.select_table("HIP_BRIGHT");
    nb.sort_and_index("alpha, delta");

    EXPECT_ANY_THROW((*nb.conn).exec("CREATE INDEX HIP_BRIGHT_IDX on HIP_BRIGHT (alpha, delta)"););

    // Delete new table and index.
    (*nb.conn).exec("DROP INDEX HIP_BRIGHT_IDX");
    (*nb.conn).exec("DROP TABLE HIP_BRIGHT");
}

TEST(Nibble, TablePolishSort) {
    std::remove("/tmp/nibble.db");
    Chomp ch = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
            .limited_by_magnitude(6.0)
            .using_current_time("01-2018")
            .build();

    Nibble nb("/tmp/nibble.db");
    nb.select_table("HIP_BRIGHT");
    nb.sort_and_index("delta");
    double a = nb.search_single("label", "rowid = 1").result;
    EXPECT_EQ(a, 104382);

    // Delete new table and index. Rerun original BSC5 table generation.
    try {
        (*nb.conn).exec("DROP INDEX HIP_BRIGHT_IDX");
        (*nb.conn).exec("DROP TABLE HIP_BRIGHT");
        Chomp::Builder()
                .with_database_name("/tmp/nibble.db")
                .with_bright_name("HIP_BRIGHT")
                .with_hip_name("HIP")
                .build();
    }
    catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

TEST(Nibble, TableInsertion) {
    std::remove("/tmp/nibble.db");
    Chomp ch = Chomp::Builder()
            .with_database_name("/tmp/nibble.db")
            .with_bright_name("HIP_BRIGHT")
            .with_hip_name("HIP")
            .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
            .limited_by_magnitude(4.5)
            .using_current_time("01-2018")
            .build();

    Nibble nb("/tmp/nibble.db");
    std::vector<double> a{0, 0, 0, 0, 0, 0, 10000000}, b;
    SQLite::Transaction transaction(*nb.conn);

    nb.select_table("HIP_BRIGHT");
    nb.insert_into_table("alpha, delta, i, j, k, m, label", a);
    SQLite::Statement query(*nb.conn, "SELECT alpha, delta FROM HIP_BRIGHT WHERE label = 10000000");
    while (query.executeStep()) {
        b.push_back(query.getColumn(0).getDouble());
        b.push_back(query.getColumn(1).getDouble());
    }
    EXPECT_EQ(b[0], 0);
    EXPECT_EQ(b[1], 0);

    try {
        (*nb.conn).exec("DELETE FROM HIP WHERE label = 10000000");
        transaction.commit();
    }
    catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}
