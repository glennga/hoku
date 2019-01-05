/// @file test-nibble.cpp
/// @author Glenn Galvizo
///
/// Source file for all Nibble class unit tests. This assumes that the bright star table
/// generator in Chomp works AND is used for CONFIG.ini time = 01-2018.

#define ENABLE_TESTING_ACCESS

#include <fstream>
#include "gtest/gtest.h"

#include "storage/chomp.h"

/// Check that Nibble database is present after creating a Nibble instance.
TEST(Nibble, FileExistence) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    std::ifstream nibble(std::string(std::getenv("HOKU_PROJECT_PATH")) + "/data/nibble.db");
    ASSERT_TRUE(nibble.good());
}

/// Check that the in-memory connection of Nibble works identically to the disk connection.
TEST(Nibble, ConnectionInMemoryInstance) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;

    // Check when we create an instance with a focus, and without.
    Nibble nb(ch.bright_table, "label"), nb3(ch.bright_table);
    Nibble::tuples_d a = nb.search_table("i, j, k", "label = 88", 3);
    Nibble::tuples_d b = nb3.search_table("i, j, k", "label = 88", 3);
    nb.select_table(ch.bright_table);

    // Ensure that the focus instance is the same as the non-focus instance. Below checked with Matlab's sph2cart.
    EXPECT_EQ(nb.search_table("i, j, k", "label = 1", 1).size(), 0);
    EXPECT_EQ(nb.search_table("i, j, k", "label = 88", 1).size(), 1);
    EXPECT_FLOAT_EQ(a[0][0], 0.658552173330720);
    EXPECT_FLOAT_EQ(a[0][1], 0.003092250084512);
    EXPECT_FLOAT_EQ(a[0][2], -0.752528719047187);
    EXPECT_FLOAT_EQ(a[0][0], b[0][0]);
    EXPECT_FLOAT_EQ(a[0][1], b[0][1]);
    EXPECT_FLOAT_EQ(a[0][2], b[0][2]);

    // Ensure that we aren't creating random connections below.
    EXPECT_ANY_THROW(Nibble nb2("SomeTableThatDoesntExist", "asd");); // NOLINT(cppcoreguidelines-avoid-goto)
}

/// Check that the bright stars table can be queried using the general search method with a constraint.
TEST(Nibble, SearchConstrained) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Nibble nb;
    nb.select_table(ch.bright_table);

    // Check the method with a constraint, given an expected amount and not.
    Nibble::tuples_d a = nb.search_table("i, j, k", "label = 88", 3);
    Nibble::tuples_d b = nb.search_table("i, j, k", "label = 88 or label = 107 ORDER BY label", 6, 2);

    EXPECT_FLOAT_EQ(a[0][0], 0.658552173330720);
    EXPECT_FLOAT_EQ(a[0][1], 0.003092250084512);
    EXPECT_FLOAT_EQ(a[0][2], -0.752528719047187);
    EXPECT_FLOAT_EQ(b[0][0], 0.658552173330720);
    EXPECT_FLOAT_EQ(b[0][1], 0.003092250084512);
    EXPECT_FLOAT_EQ(b[0][2], -0.752528719047187);
    EXPECT_FLOAT_EQ(b[1][0], 0.638255709461383);
    EXPECT_FLOAT_EQ(b[1][1], 0.003719091180710);
    EXPECT_FLOAT_EQ(b[1][2], -0.769815443921941);

    Nibble::tuples_d c = nb.search_table("i, j, k", "label > 88", 3);
    Nibble::tuples_d d = nb.search_table("i, j, k", "label > 88", 6, 10);
    Nibble::tuples_d e = nb.search_table("i, j, k", "label > 88", 6, Nibble::NO_LIMIT);
    EXPECT_EQ(c.size(), 4558);
    EXPECT_EQ(d.size(), 10);
    EXPECT_EQ(e.size(), 4558);
}

/// Check that the bright stars table can be queried using the general search method without a constraint.
TEST(Nibble, SearchNotConstrained) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Nibble nb;
    nb.select_table(ch.bright_table);
    Nibble::tuples_d a = nb.search_table("i", Chomp::BRIGHT_TABLE_LENGTH, Nibble::NO_LIMIT);
    EXPECT_EQ(a.size(), Chomp::BRIGHT_TABLE_LENGTH);

    Nibble::tuples_d c = nb.search_table("i, j, k", 3);
    Nibble::tuples_d d = nb.search_table("i, j, k", 6, 10);
    Nibble::tuples_d e = nb.search_table("i, j, k", 6, Nibble::NO_LIMIT);
    EXPECT_EQ(c.size(), 4559);
    EXPECT_EQ(d.size(), 10);
    EXPECT_EQ(e.size(), 4559);
}

/// Check that the single search method works as intended with a constraint and without.
TEST(Nibble, SearchSingle) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Nibble nb;
    nb.select_table(ch.bright_table);

    EXPECT_FLOAT_EQ(nb.search_single("i", "label = 88"), 0.658552173330720);
    EXPECT_FLOAT_EQ(nb.search_single("j", "label = 88"), 0.003092250084512);
    EXPECT_FLOAT_EQ(nb.search_single("k", "label = 88"), -0.752528719047187);

    EXPECT_EQ(nb.search_single("COUNT(*)"), ch.bright_as_list().size());
}

/// Check that the table creation method works as intended (the table persists after closing connection.
TEST(Nibble, TableCreation) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    // Clean up our mess (if it exists).
    Nibble nb2;
    (*nb2.conn).exec("DROP TABLE IF EXISTS MYTABLE");

    // Nibble gets destroyed when this is done.
    std::unique_ptr<Nibble> nb_p = std::make_unique<Nibble>();
    EXPECT_EQ (0, (*nb_p).create_table("MYTABLE", "a int"));
    nb_p.reset(nullptr);

    // Attempting to create a table again should return an error.
    Nibble nb;
    EXPECT_EQ(Nibble::TABLE_NOT_CREATED, nb.create_table("MYTABLE", "a int"));

    // Clean up our mess.
    (*nb.conn).exec("DROP TABLE IF EXISTS MYTABLE");
}

/// Check that retrieved attributes and schema are correct for a given table.
TEST(Nibble, TableAttributeRetrieval) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Nibble nb;
    EXPECT_EQ (0, nb.create_table("MYTABLE", "a int, b int"));

    std::string schema, fields;
    nb.select_table("MYTABLE");
    nb.find_attributes(schema, fields);

    EXPECT_EQ("a, b", fields);
    EXPECT_EQ("a int, b int", schema);

    // Clean up our mess.
    (*nb.conn).exec("DROP TABLE IF EXISTS MYTABLE");
}

/// Check that the bright stars table has an index created.
TEST(Nibble, TablePolishIndex) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Nibble nb;
    nb.select_table(ch.bright_table);
    nb.polish_table("alpha");

    EXPECT_ANY_THROW((*nb.conn).exec("CREATE INDEX HIP_BRIGHT_IDX on " + // NOLINT(cppcoreguidelines-avoid-goto)
                                     ch.bright_table + "(alpha)"););

    // Delete new table and index. Rerun original bright table generation.
    (*nb.conn).exec("DROP INDEX HIP_BRIGHT_IDX");
    (*nb.conn).exec("DROP TABLE " + ch.bright_table);
    Chomp();
}

/// Check that the bright stars table has an dual-index created.
TEST(Nibble, TablePolishDualIndex) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Nibble nb;
    nb.select_table(ch.bright_table);
    nb.polish_table("alpha, delta");

    EXPECT_ANY_THROW((*nb.conn).exec("CREATE INDEX HIP_BRIGHT_IDX on " + // NOLINT(cppcoreguidelines-avoid-goto)
                                     ch.bright_table + "(alpha, delta)"););

    // Delete new table and index. Rerun original bright table generation.
    (*nb.conn).exec("DROP INDEX HIP_BRIGHT_IDX");
    (*nb.conn).exec("DROP TABLE " + ch.bright_table);
    Chomp();
}

/// Check that the bright stars table has an index created. 'sort' is called with 'polish', this is tested too.
TEST(Nibble, TablePolishSort) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Chomp ch;
    Nibble nb;
    nb.select_table(ch.bright_table);
    nb.polish_table("delta");
    double a = nb.search_single("label", "rowid = 1");
    EXPECT_EQ(a, 104382);

    // Delete new table and index. Rerun original BSC5 table generation.
    try {
        (*nb.conn).exec("DROP INDEX HIP_BRIGHT_IDX");
        (*nb.conn).exec("DROP TABLE " + ch.bright_table);
        Chomp();
    }
    catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

/// Test if the insertion of an entry was made.
TEST(Nibble, TableInsertion) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    Nibble nb;
    Chomp ch;
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
        (*nb.conn).exec("DELETE FROM " + ch.bright_table + " WHERE label = 10000000");
        transaction.commit();
    }
    catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}