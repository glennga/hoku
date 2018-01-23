/// @file test-lumberjack.cpp
/// @author Glenn Galvizo
///
/// Source file for all Lumberjack class unit tests and the test runner.

#define ENABLE_TESTING_ACCESS

#include <chrono>
#include "experiment/lumberjack.h"
#include "experiment/experiment.h"
#include "gmock/gmock.h"

/// Ensure that the correct fields are selected. Uses the Query experiment table.
TEST(LumberjackConstruction, Constructor) {
    std::ostringstream l;
    using clock = std::chrono::high_resolution_clock;
    l << clock::to_time_t(clock::now() - std::chrono::hours(24));
    
    Lumberjack lu("QUERY", "Angle", l.str());
    EXPECT_EQ(lu.table, "QUERY");
    EXPECT_EQ(lu.identifier_name, "Angle");
    EXPECT_EQ(lu.timestamp, l.str());
    auto b = std::string(Experiment::Query::SCHEMA);
    EXPECT_EQ(lu.expected_result_size, (std::count(b.begin(), b.end(), ',') - 2 + 1));
}

/// Ensure that the buffer is flushed when the Lumberjack is flushed. Uses the Query experiment table.
TEST(LumberjackConstruction, Destructor) {
    std::ostringstream l;
    using clock = std::chrono::high_resolution_clock;
    l << clock::to_time_t(clock::now() - std::chrono::hours(24));
    
    Lumberjack lu("QUERY", "Angle", l.str());
    lu.log_trial(Nibble::tuple_d {-1, -1, -1, -1});
    lu.~Lumberjack();
    
    Lumberjack lu2("QUERY", "Angle", l.str());
    Nibble::tuples_d a = lu2.search_table("SigmaQuery", "SigmaQuery = -1", 1, 10);
    EXPECT_EQ(a.size(), 1);
    (*lu2.conn).exec("DELETE FROM QUERY WHERE SigmaQuery = -1");
}

/// Ensure that the log function works past the buffer limit.
TEST(LumberjackLog, LogFunction) {
    std::ostringstream l;
    using clock = std::chrono::high_resolution_clock;
    l << clock::to_time_t(clock::now() - std::chrono::hours(24));
    
    Lumberjack lu("QUERY", "Angle", l.str());
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