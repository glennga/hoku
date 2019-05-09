/// @file lumberjack.cpp
/// @author Glenn Galvizo
///
/// Source file for Lumberjack class, which facilitates the storage of various results of trials.

#include <algorithm>
#include <chrono>
#include <thread>
#include <libgen.h>

#include "math/random-draw.h"
#include "experiment/lumberjack.h"

/// The maximum size of our result buffer (dependent on SQLITE_MAX_VARIABLE_NUMBER). Flush above this limit.
const unsigned long Lumberjack::MAXIMUM_BUFFER_SIZE = 50;

/// Constructor. We open a connection to the Lumberjack database here, and we determine our expected result length.
///
/// @param trial_table Name of the trial (table name in database) this lumberjack is attached to.
/// @param identifier_name Name of the identification method this lumberjack is attached to.
/// @param timestamp Time associated with the beginning of each experiment (not trial).
Lumberjack::Lumberjack (const std::string &trial_table, const std::string &identifier_name,
                        const std::string &timestamp) {
    // Parse the project path.
    std::string project_path = std::string(dirname(const_cast<char *>(__FILE__))) + "/../../";
    INIReader cf(std::getenv("HOKU_CONFIG_INI") ? std::string(std::getenv("HOKU_CONFIG_INI")) :
                 project_path + "CONFIG.ini");

    const int FLAGS = SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE;
    conn = std::make_unique<SQLite::Database>(project_path + cf.Get("database-names", "lumberjack", ""), FLAGS);
    result_buffer.reserve(MAXIMUM_BUFFER_SIZE);

    // We won't be changing our table from here. Ensure it exists before proceeding.
    if (!does_table_exist(trial_table)) {
        throw std::runtime_error(std::string("Table " + trial_table + " does not exist."));
    }
    select_table(trial_table);
    this->identifier_name = identifier_name, this->timestamp = timestamp;

    // Determine the length of every result into log_trial. The schema string will not be used.
    std::string schema;
    find_attributes(schema, trial_fields);

    // Tuple length = number of commas + 1, subtract 2 as we don't expect the identifier name and the timestamp.
    expected_result_size = static_cast<unsigned int> (std::count(trial_fields.begin(), trial_fields.end(), ',')) - 1;
}

/// Flush our buffer before destroying our lumberjack.
Lumberjack::~Lumberjack () {
    flush_buffer();
}

/// Create the appropriate table in the lumberjack database.
///
/// @param table_name Name of the table (i.e. trial name) to create table for.
/// @param fields Fields corresponding to the table to be created.
/// @return TABLE_NOT_CREATED if a table already exists. 0 otherwise.
int Lumberjack::create_table (const std::string &table_name, const std::string &fields) {
    const int FLAGS = SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE;
    std::string project_path = std::string(dirname(const_cast<char *>(__FILE__))) + "/../../";
    INIReader cf(std::getenv("HOKU_CONFIG_INI") ? std::string(std::getenv("HOKU_CONFIG_INI")) :
                 project_path + "CONFIG.ini");
    Nibble nb;

    nb.conn = std::make_unique<SQLite::Database>(project_path + cf.Get("database-names", "lumberjack", ""), FLAGS);
    return nb.create_table(table_name, fields);
}

/// Insert our result into the buffer. If necessary, flush the buffer and write to our database.
///
/// @param result A result tuple to insert into the selected trial table.
/// @param method_name Name of the method whose trial this corresponds to.
/// @return 0 when finished.
int Lumberjack::log_trial (const tuple_d &result) {
    if (result.size() != expected_result_size) {
        throw std::runtime_error(std::string("Result is not of size: " + std::to_string(expected_result_size) + "."));
    }
    result_buffer.emplace_back(result);

    // If we have reached our storage max, flush the buffer.
    return (result_buffer.size() >= MAXIMUM_BUFFER_SIZE) ? flush_buffer() : 0;
}

/// Write everything in our result buffer to appropriate table in the lumberjack database.
///
/// @return 0 when finished.
int Lumberjack::flush_buffer () {
    // Do not proceed if the buffer is empty.
    if (result_buffer.empty()) {
        return 0;
    }

    // Create bind statement with necessary amount of '?'.
    std::string sql = "INSERT INTO " + table + std::string(" (") + trial_fields + ") VALUES ";

    // For each result in our buffer, prepare a statement.
    for (unsigned int i = 0; i < result_buffer.size(); i++) {
        sql.append("(?, ");
        for (unsigned int a = 0; a < expected_result_size; a++) {
            sql.append("?, ");
        }
        sql.append("?),");
    }
    sql.erase(sql.size() - 1);

    // Bind all the fields to the results, and execute the insert.
    SQLite::Statement insert(*conn, sql);
    for (unsigned int i = 0; i < result_buffer.size(); i++) {

        // Fields here are 1-indexed.
        insert.bind((i * (expected_result_size + 2)) + 1, this->identifier_name);
        insert.bind((i * (expected_result_size + 2)) + 2, this->timestamp);
        for (unsigned int j = 0; j < expected_result_size; j++) {
            insert.bind((i * (expected_result_size + 2)) + j + 3, result_buffer[i][j]);
        }
    }

    // Keep trying to perform the insertion.
    while (true) {
        try {
            SQLite::Transaction t(*conn);
            if (insert.exec() != static_cast<signed> (result_buffer.size())) {
                throw std::runtime_error(std::string("All results were not recorded."));
            }
            else {
                result_buffer.clear(), result_buffer.reserve(MAXIMUM_BUFFER_SIZE), t.commit();
                return 0;
            }
        }
        catch (SQLite::Exception &) {
            // Another insertion is currently occurring. Wait until this is finished.
            std::this_thread::sleep_for(std::chrono::milliseconds(RandomDraw::draw_integer(0, 1000)));
        }
    }
}
