/// @file lumberjack.h
/// @author Glenn Galvizo
///
/// Header file for Lumberjack class, which contains functions that facilitate the storage of various results of
/// trials.

#ifndef HOKU_LUMBERJACK_H
#define HOKU_LUMBERJACK_H

#include "storage/nibble.h"

/// @brief Class used to log the results of all experiments.
class Lumberjack : public Nibble {
public:
    class Builder;
    ~Lumberjack ();

    static int create_table (const std::string &database_path, const std::string &table_name,
                             const std::string &schema);
    int log_trial (const tuple_d &result);

private:
    int flush_buffer ();
    static const unsigned long MAXIMUM_BUFFER_SIZE;
    unsigned int expected_result_size;
    std::vector<tuple_d> result_buffer;

    std::string trial_fields;
    std::string identifier_name;
    std::string timestamp;

    Lumberjack (const std::string &database_name, const std::string &trial_table, const std::string &prefix,
                const std::string &timestamp);
};

class Lumberjack::Builder {
public:
    Builder &with_database_name (const std::string &name) {
        this->database_name = name;
        return *this;
    }
    Builder &using_trial_table (const std::string &name) {
        this->trial_table = name;
        return *this;
    }
    Builder &with_prefix (const std::string &p) {
        this->prefix = p;
        return *this;
    }
    Builder &using_timestamp (const std::string &tim) {
        this->at_time = tim;
        return *this;
    }
    Lumberjack build () { return Lumberjack(database_name, trial_table, prefix, at_time); }

private:
    std::string database_name;
    std::string trial_table;
    std::string prefix;
    std::string at_time;
};


#endif /* HOKU_LUMBERJACK_H */
