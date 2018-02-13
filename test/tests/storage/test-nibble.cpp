/// @file test-nibble.cpp
/// @author Glenn Galvizo
///
/// Source file for all Nibble class unit tests and the test runner. This assumes that the bright star table
/// generator in Chomp works.

#define ENABLE_TESTING_ACCESS

#include <fstream>
#include "gtest/gtest.h"

#include "storage/chomp.h"

/// Check that Nibble database is present after creating a Nibble instance.
TEST(NibbleFile, Existence) {
    std::ifstream nibble(Nibble().DATABASE_LOCATION);
    ASSERT_TRUE(nibble.good());
}

/// Check that the bright stars table can be queried using the general search method.
TEST(NibbleTable, SearchResult) {
    Chomp ch;
    Nibble nb;
    (*ch.conn).exec("DROP TABLE IF EXISTS " + ch.bright_table);
    (*ch.conn).exec("DROP TABLE IF EXISTS " + ch.hip_table);
    ch.generate_bright_table(), ch.generate_hip_table();
    nb.select_table(ch.bright_table);
    Nibble::tuples_d a = nb.search_table("i, j, k", "label = 88", 3);
    Nibble::tuples_d b = nb.search_table("i, j, k", "label = 88 or label = 107", 6, 2);
    
    EXPECT_DOUBLE_EQ(a[0][0], 0.658552697278613);
    EXPECT_NEAR(a[0][1], 0.00309371805098928, 0.00000000000000001);
    EXPECT_DOUBLE_EQ(a[0][2], -0.75252825449659);
    EXPECT_DOUBLE_EQ(b[1][0], 0.638254649361992);
    EXPECT_DOUBLE_EQ(b[1][1], 0.00371847539671449);
    EXPECT_DOUBLE_EQ(b[1][2], -0.769816325826183);
}

/// Check that the bright stars table has an index created.
TEST(NibbleTable, PolishIndex) {
    Chomp ch;
    Nibble nb;
    ch.generate_bright_table();
    nb.select_table(ch.bright_table);
    nb.polish_table("alpha");
    bool assertion = false;
    
    try {
        SQLite::Statement(*nb.conn, "CREATE INDEX HIP_BRIGHT_alpha on " + ch.bright_table + "(alpha)").exec();
    }
    catch (std::exception &e) {
        // Exception thrown while creating index means that the index exists.
        std::cout << "Exception: " << e.what() << std::endl;
        assertion = true;
    }
    
    // Delete new table and index. Rerun original bright table generation.
    SQLite::Transaction transaction(*nb.conn);
    SQLite::Statement(*nb.conn, "DROP INDEX HIP_BRIGHT_alpha").exec();
    SQLite::Statement(*nb.conn, "DROP TABLE " + ch.bright_table).exec();
    transaction.commit();
    Chomp().generate_bright_table();
    EXPECT_TRUE(assertion);
}

/// Check that the bright stars table has an index created.
TEST(NibbleTable, PolishSort) {
    Chomp ch;
    Nibble nb;
    ch.generate_bright_table();
    nb.select_table(ch.bright_table);
    nb.polish_table("delta");
    double a = nb.search_single("label", "rowid = 1");
    EXPECT_EQ(a, 104023);
    
    // Delete new table and index. Rerun original BSC5 table generation.
    try {
        SQLite::Transaction transaction(*nb.conn);
        SQLite::Statement(*nb.conn, "DROP INDEX HIP_BRIGHT_delta").exec();
        SQLite::Statement(*nb.conn, "DROP TABLE " + ch.bright_table).exec();
        transaction.commit();
        ch.generate_bright_table();
    }
    catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

/// Test if the insertion of an entry was made.
TEST(NibbleTable, Insertion) {
    Nibble nb;
    Chomp ch;
    ch.generate_bright_table();
    std::vector<double> a{0, 0, 0, 0, 0, 0, 10000000}, b;
    SQLite::Transaction transaction(*nb.conn);
    
    nb.select_table(ch.bright_table);
    nb.insert_into_table("alpha, delta, i, j, k, m, label", a);
    SQLite::Statement query(*nb.conn, "SELECT alpha, delta FROM " + ch.bright_table + " WHERE label = 10000000");
    while (query.executeStep()) {
        b.push_back(query.getColumn(0).getDouble());
        b.push_back(query.getColumn(1).getDouble());
    }
    EXPECT_EQ(b[0], 0);
    EXPECT_EQ(b[1], 0);
    
    try {
        SQLite::Statement(*nb.conn, "DELETE FROM " + ch.bright_table + " WHERE label = 10000000").exec();
        transaction.commit();
    }
    catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

/// Check that the in-memory connection of Nibble works identically to the disk connection.
TEST(NibbleConnection, InMemoryInstance) {
    Chomp ch;
    ch.generate_bright_table();
    Nibble nb(ch.bright_table, "label");
    Nibble::tuples_d a = nb.search_table("i, j, k", "label = 88", 3);
    
    EXPECT_NEAR(a[0][0], 0.658552697278613, 0.000000000000001);
    EXPECT_NEAR(a[0][1], 0.00309371805098928, 0.000000000000001);
    EXPECT_NEAR(a[0][2], -0.75252825449659, 0.000000000000001);
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