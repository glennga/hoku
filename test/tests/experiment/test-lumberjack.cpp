/// @file test-lumberjack.cpp
/// @author Glenn Galvizo
///
/// Source file for all Lumberjack class unit tests.

#define ENABLE_TESTING_ACCESS

#include <chrono>
#include "experiment/lumberjack.h"
#include "experiment/experiment.h"
#include "gmock/gmock.h"

/// Verify that all trial schemas and fields are correct.
TEST(Lumberjack, TablesExistenceStructure) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));

    EXPECT_NO_THROW(Lumberjack::create_table( // NOLINT(cppcoreguidelines-avoid-goto)
            cf.Get("query-experiment", "lu", ""), Experiment::Query::SCHEMA););
    EXPECT_NO_THROW(Lumberjack::create_table(  // NOLINT(cppcoreguidelines-avoid-goto)
            cf.Get("reduction-experiment", "lu", ""), Experiment::Reduction::SCHEMA););
    EXPECT_NO_THROW(Lumberjack::create_table(  // NOLINT(cppcoreguidelines-avoid-goto)
            cf.Get("identification-experiment", "lu", ""), Experiment::Map::SCHEMA););
    EXPECT_NO_THROW(Lumberjack::create_table( // NOLINT(cppcoreguidelines-avoid-goto)
            cf.Get("overlay-experiment", "lu", ""), Experiment::Overlay::SCHEMA););

    Nibble nb;
    std::string schema, fields;
    nb.conn = std::make_unique<SQLite::Database>(std::getenv("HOKU_PROJECT_PATH") + std::string("/data/lumberjack.db"),
                                                 SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

    EXPECT_NO_THROW(nb.select_table(cf.Get("query-experiment", "lu", ""));); // NOLINT(cppcoreguidelines-avoid-goto)
    nb.find_attributes(schema, fields);
    EXPECT_EQ(schema, Experiment::Query::SCHEMA);
    EXPECT_EQ(fields,
              "IdentificationMethod, Timestamp, Sigma1, Sigma2, Sigma3, ShiftDeviation, CandidateSetSize, RunningTime, "
              "SExistence");

    EXPECT_NO_THROW(nb.select_table(cf.Get("reduction-experiment", "lu", ""));); // NOLINT(cppcoreguidelines-avoid-goto)
    nb.find_attributes(schema, fields);
    EXPECT_EQ(schema, Experiment::Reduction::SCHEMA);
    EXPECT_EQ(fields,
              "IdentificationMethod, Timestamp, Sigma1, Sigma2, Sigma3, ShiftDeviation, FalseStars, QueryCount, "
              "TimeToResult, PercentageCorrect");

    EXPECT_NO_THROW(nb.select_table(  // NOLINT(cppcoreguidelines-avoid-goto)
            cf.Get("identification-experiment", "lu", "")););
    nb.find_attributes(schema, fields);
    EXPECT_EQ(schema, Experiment::Map::SCHEMA);
    EXPECT_EQ(fields, "IdentificationMethod, Timestamp, Sigma1, Sigma2, Sigma3, Sigma4, ShiftDeviation, FalseStars, "
                      "QueryCount, TimeToResult, PercentageCorrect, IsErrorOut");

    EXPECT_NO_THROW(nb.select_table(cf.Get("overlay-experiment", "lu", ""));); // NOLINT(cppcoreguidelines-avoid-goto)
    nb.find_attributes(schema, fields);
    EXPECT_EQ(schema, Experiment::Overlay::SCHEMA);
    EXPECT_EQ(fields, "IdentificationMethod, Timestamp, Sigma4, ShiftDeviation, FalseStars, TruePositive, "
                      "FalsePositive, TrueNegative, FalseNegative, N");
}

/// Ensure that the correct fields are selected.
TEST(Lumberjack, ConstructionConstructor) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    std::ostringstream l;
    using clock = std::chrono::system_clock;
    l << clock::to_time_t(clock::now() - std::chrono::hours(24));
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));

    Lumberjack lu(cf.Get("query-experiment", "lu", ""), "Angle", l.str());
    EXPECT_EQ(lu.table, cf.Get("query-experiment", "lu", ""));
    EXPECT_EQ(lu.identifier_name, "Angle");
    EXPECT_EQ(lu.timestamp, l.str());
    auto b = std::string(Experiment::Query::SCHEMA);
    EXPECT_EQ(lu.expected_result_size, (std::count(b.begin(), b.end(), ',') - 2 + 1));
}

/// Ensure that the buffer is flushed when the Lumberjack is flushed.
TEST(Lumberjack, ConstructionDestructor) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    std::ostringstream l;
    using clock = std::chrono::system_clock;
    l << clock::to_time_t(clock::now() - std::chrono::hours(24));
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));

    // Lu gets destroyed when exiting.
    std::unique_ptr<Lumberjack> lu_p = std::make_unique<Lumberjack>(cf.Get("query-experiment", "lu", ""), "Angle",
                                                                    l.str());
    (*lu_p).log_trial(Nibble::tuple_d{-1, -1, -1, -1, -1, -1, -1});
    lu_p.reset(nullptr);

    Lumberjack lu2(cf.Get("query-experiment", "lu", ""), "Angle", l.str());
    Nibble::tuples_d a = lu2.search_table("Sigma1", "Sigma1 = -1", 1, 10);
    EXPECT_EQ(a.size(), 1);

    SQLite::Transaction transaction(*lu2.conn);
    (*lu2.conn).exec("DELETE FROM QUERY WHERE Sigma1 = -1");
    transaction.commit();
}

/// Ensure that the log function works as intended.
TEST(Lumberjack, LogFunction) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    std::ostringstream l;
    using clock = std::chrono::system_clock;
    l << clock::to_time_t(clock::now() - std::chrono::hours(24));
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));

    Lumberjack lu(cf.Get("query-experiment", "lu", ""), "Angle", l.str());
    lu.log_trial(Nibble::tuple_d{-1, -1, -1, -1, -1, -1, -1});
    Nibble::tuples_d a = lu.search_table("Sigma1", "Sigma1 = -1", 1, 10);
    EXPECT_EQ(a.size(), 0);

    Nibble::tuples_d b = lu.search_table("Sigma1", "Sigma1 = -1", 1, 1);
    EXPECT_EQ(b.size(), 0);

    lu.flush_buffer();
    Nibble::tuples_d b_1 = lu.search_table("Sigma1", "Sigma1 = -1", 1, 1);
    EXPECT_EQ(b_1.size(), 1);

    SQLite::Transaction transaction(*lu.conn);
    (*lu.conn).exec("DELETE FROM QUERY WHERE Sigma1 = -1");
    transaction.commit();
}

/// Ensure that the log function works past the buffer limit.
TEST(Lumberjack, LogFunctionFlush) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    std::ostringstream l;
    using clock = std::chrono::system_clock;
    l << clock::to_time_t(clock::now() - std::chrono::hours(24));
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));

    Lumberjack lu(cf.Get("query-experiment", "lu", ""), "Angle", l.str());
    lu.log_trial(Nibble::tuple_d{-1, -1, -1, -1, -1, -1, -1});
    Nibble::tuples_d a = lu.search_table("Sigma1", "Sigma1 = -1", 1, 10);
    EXPECT_EQ(a.size(), 0);

    for (int i = 0; i < static_cast<int>(Lumberjack::MAXIMUM_BUFFER_SIZE) - 2; i++) {
        lu.log_trial(Nibble::tuple_d{-1, -1, -1, -1, -1, -1, -1});
    }
    EXPECT_EQ(lu.result_buffer.size(), Lumberjack::MAXIMUM_BUFFER_SIZE - 1);
    lu.log_trial(Nibble::tuple_d{-1, -1, -1, -1, -1, -1, -1});
    EXPECT_EQ(lu.result_buffer.size(), 0);

    lu.log_trial(Nibble::tuple_d{-1, -1, -1, -1, -1, -1, -1});
    EXPECT_EQ(lu.result_buffer.size(), 1);
    Nibble::tuples_d b = lu.search_table("Sigma1", "Sigma1 = -1",
                                         static_cast<unsigned int>(Lumberjack::MAXIMUM_BUFFER_SIZE),
                                         static_cast<unsigned int>(Lumberjack::MAXIMUM_BUFFER_SIZE + 1));
    EXPECT_EQ(b.size(), Lumberjack::MAXIMUM_BUFFER_SIZE);

    lu.flush_buffer();

    SQLite::Transaction transaction(*lu.conn);
    (*lu.conn).exec("DELETE FROM QUERY WHERE Sigma1 = -1");
    transaction.commit();
}

/// Check that when two lumberjacks are trying to perform an insert at the same time, both insertions succeed.
TEST(Lumberjack, LogDualLumberjack) { // NOLINT(cert-err58-cpp,modernize-use-equals-delete)
    // TODO: Determine a test for this.
}