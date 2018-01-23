/// @file lumberjack.h
/// @author Glenn Galvizo
///
/// Header file for Lumberjack class, which contains functions that facilitate the storage of various results of
/// trials.

#ifndef HOKU_LUMBERJACK_H
#define HOKU_LUMBERJACK_H

#include "storage/nibble.h"

// TODO: finish documentation with examples
/// The lumberjack class is used to log the results of all trials.
#if !defined ENABLE_TESTING_ACCESS
class Lumberjack : private Nibble {
#else
class Lumberjack : public Nibble {
#endif
  public:
    Lumberjack (const std::string &trial_table, const std::string &identifier_name, const std::string &timestamp);
    ~Lumberjack ();
    
    static int create_table (const std::string &table_name, const std::string &fields);
    int log_trial (const tuple_d &result);
    
    static const std::string PROJECT_LOCATION;
    static const std::string DATABASE_LOCATION;

#if !defined ENABLE_TESTING_ACCESS
    private:
#endif
    int flush_buffer ();

#if !defined ENABLE_TESTING_ACCESS
    private:
#endif
    /// The maximum size of our result buffer. When we go over this limit, we flush.
    static constexpr int MAXIMUM_BUFFER_SIZE = 100;
    
    /// Every trial log must be of this size.
    unsigned int expected_result_size;
    
    /// The log_trial function stores each result in this buffer.
    std::vector<tuple_d> result_buffer;
    
    /// Comma separated string of the fields for the current trial table.
    std::string trial_fields;
    
    /// Name of identification method used to produce the results of each trial.
    std::string identifier_name;
    
    /// Time associated with the beginning of each experiment (not trial).
    std::string timestamp;
};

#endif /* HOKU_LUMBERJACK_H */
