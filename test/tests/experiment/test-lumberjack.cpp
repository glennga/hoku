/// @file test-lumberjack.cpp
/// @author Glenn Galvizo
///
/// Source file for all Lumberjack class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include <chrono>
#include "experiment/lumberjack.h"
#include "experiment/experiment.h"
#include "gmock/gmock.h"

/// Verify that all trial schemas and fields are correct.
TEST(LumberjackTables, ExistenceStructure) {
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
    std::remove((std::string(std::getenv("HOKU_PROJECT_PATH")) + "/data/lumberjack.db").c_str());
    
    EXPECT_NO_THROW(Lumberjack::create_table(cf.Get("query-experiment", "lu", ""), Experiment::Query::SCHEMA););
    EXPECT_NO_THROW(Lumberjack::create_table(cf.Get("reduction-experiment", "lu", ""), Experiment::Reduction::SCHEMA););
    EXPECT_NO_THROW(Lumberjack::create_table(cf.Get("identification-experiment", "lu", ""), Experiment::Map::SCHEMA););
    
    Nibble nb;
    std::string schema, fields;
    nb.conn = std::make_unique<SQLite::Database>(std::getenv("HOKU_PROJECT_PATH") + std::string("/data/lumberjack.db"),
                                                 SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    
    EXPECT_NO_THROW(nb.select_table(cf.Get("query-experiment", "lu", "")););
    nb.find_attributes(schema, fields);
    EXPECT_EQ(schema, Experiment::Query::SCHEMA);
    EXPECT_EQ(fields, "IdentificationMethod, Timestamp, SigmaQuery, ShiftDeviation, CandidateSetSize, SExistence");
    
    EXPECT_NO_THROW(nb.select_table(cf.Get("reduction-experiment", "lu", "")););
    nb.find_attributes(schema, fields);
    EXPECT_EQ(schema, Experiment::Reduction::SCHEMA);
    EXPECT_EQ(fields, "IdentificationMethod, Timestamp, SigmaQuery, SigmaOverlay, ShiftDeviation, CameraSensitivity, "
        "ResultMatchesInput");
    
    EXPECT_NO_THROW(nb.select_table(cf.Get("identification-experiment", "lu", "")););
    nb.find_attributes(schema, fields);
    EXPECT_EQ(schema, Experiment::Map::SCHEMA);
    EXPECT_EQ(fields, "IdentificationMethod, Timestamp, SigmaQuery, SigmaOverlay, ShiftDeviation, CameraSensitivity, "
        "FalseStars, ComparisonCount, PercentageCorrect");
}

/// Ensure that the correct fields are selected.
TEST(LumberjackConstruction, Constructor) {
    std::ostringstream l;
    using clock = std::chrono::high_resolution_clock;
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
TEST(LumberjackConstruction, Destructor) {
    std::ostringstream l;
    using clock = std::chrono::high_resolution_clock;
    l << clock::to_time_t(clock::now() - std::chrono::hours(24));
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
    
    // Lu gets destroyed when exiting.
    std::unique_ptr<Lumberjack> lu_p = std::make_unique<Lumberjack>(cf.Get("query-experiment", "lu", ""), "Angle",
                                                                    l.str());
    (*lu_p).log_trial(Nibble::tuple_d {-1, -1, -1, -1});
    lu_p.reset(nullptr);
    
    Lumberjack lu2(cf.Get("query-experiment", "lu", ""), "Angle", l.str());
    Nibble::tuples_d a = lu2.search_table("SigmaQuery", "SigmaQuery = -1", 1, 10);
    EXPECT_EQ(a.size(), 1);
    (*lu2.conn).exec("DELETE FROM QUERY WHERE SigmaQuery = -1");
}

/// Ensure that the log function works as intended.
TEST(LumberjackLog, LogFunction) {
    std::ostringstream l;
    using clock = std::chrono::high_resolution_clock;
    l << clock::to_time_t(clock::now() - std::chrono::hours(24));
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
    
    Lumberjack lu(cf.Get("query-experiment", "lu", ""), "Angle", l.str());
    lu.log_trial(Nibble::tuple_d{-1, -1, -1, -1});
    Nibble::tuples_d a = lu.search_table("SigmaQuery", "SigmaQuery = -1", 1, 10);
    EXPECT_EQ(a.size(), 0);
    
    Nibble::tuples_d b = lu.search_table("SigmaQuery", "SigmaQuery = -1", 1, 1);
    EXPECT_EQ(b.size(), 0);
    
    lu.flush_buffer();
    Nibble::tuples_d b_1 = lu.search_table("SigmaQuery", "SigmaQuery = -1", 1, 1);
    EXPECT_EQ(b_1.size(), 1);
    
    (*lu.conn).exec("DELETE FROM QUERY WHERE SigmaQuery = -1");
}

/// Ensure that the log function works past the buffer limit.
TEST(LumberjackLog, LogFunctionFlush) {
    std::ostringstream l;
    using clock = std::chrono::high_resolution_clock;
    l << clock::to_time_t(clock::now() - std::chrono::hours(24));
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
    
    Lumberjack lu(cf.Get("query-experiment", "lu", ""), "Angle", l.str());
    lu.log_trial(Nibble::tuple_d{-1, -1, -1, -1});
    Nibble::tuples_d a = lu.search_table("SigmaQuery", "SigmaQuery = -1", 1, 10);
    EXPECT_EQ(a.size(), 0);
    
    for (int i = 0; i < Lumberjack::MAXIMUM_BUFFER_SIZE - 2; i++) {
        lu.log_trial(Nibble::tuple_d{-1, -1, -1, -1});
    }
    EXPECT_EQ(lu.result_buffer.size(), Lumberjack::MAXIMUM_BUFFER_SIZE - 1);
    lu.log_trial(Nibble::tuple_d{-1, -1, -1, -1});
    EXPECT_EQ(lu.result_buffer.size(), 0);
    
    lu.log_trial(Nibble::tuple_d{-1, -1, -1, -1});
    EXPECT_EQ(lu.result_buffer.size(), 1);
    Nibble::tuples_d b = lu.search_table("SigmaQuery", "SigmaQuery = -1", Lumberjack::MAXIMUM_BUFFER_SIZE,
                                         Lumberjack::MAXIMUM_BUFFER_SIZE + 1);
    EXPECT_EQ(b.size(), Lumberjack::MAXIMUM_BUFFER_SIZE);
    
    lu.flush_buffer();
    (*lu.conn).exec("DELETE FROM QUERY WHERE SigmaQuery = -1");
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