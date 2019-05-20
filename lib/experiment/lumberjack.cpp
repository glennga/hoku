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

const unsigned long Lumberjack::MAXIMUM_BUFFER_SIZE = 50;

Lumberjack::Lumberjack (const std::string &database_name, const std::string &trial_table, const std::string &prefix,
                        const std::string &timestamp) : Nibble(database_name) {
    result_buffer.reserve(MAXIMUM_BUFFER_SIZE); // NOTE: We assume that Lumberjack has already been created.

    // We won't be changing our table from here. Ensure it exists before proceeding.
    if (!does_table_exist(trial_table)) {
        throw std::runtime_error(std::string("Table " + trial_table + " does not exist."));
    }
    select_table(trial_table);
    this->identifier_name = prefix, this->timestamp = timestamp;

    // Tuple length = number of commas + 1, subtract 2 as we don't expect the identifier name and the timestamp.
    std::string schema;
    find_attributes(schema, trial_fields);
    expected_result_size = static_cast<unsigned int> (std::count(trial_fields.begin(), trial_fields.end(), ',')) - 1;
}
Lumberjack::~Lumberjack () { flush_buffer(); }

int Lumberjack::create_table (const std::string &database_path, const std::string &table_name,
                              const std::string &schema) {
    return Nibble(database_path).create_table(table_name, schema);
}

int Lumberjack::log_trial (const tuple_d &result) {
    if (result.size() != expected_result_size) {
        throw std::runtime_error(std::string("Result is not of size: " + std::to_string(expected_result_size) + "."));
    }
    result_buffer.emplace_back(result);

    // If we have reached our storage max, flush the buffer.
    return (result_buffer.size() >= MAXIMUM_BUFFER_SIZE) ? flush_buffer() : 0;
}

int Lumberjack::flush_buffer () {
    // Do not proceed if the buffer is empty.
    if (result_buffer.empty()) return 0;

    std::string sql = "INSERT INTO " + current_table + std::string(" (") + trial_fields + ") VALUES ";
    for (unsigned int i = 0; i < result_buffer.size(); i++) {
        sql.append("(?, ");
        for (unsigned int a = 0; a < expected_result_size; a++) sql.append("?, ");
        sql.append("?),");
    }
    sql.erase(sql.size() - 1);

    SQLite::Statement insert(*conn, sql);
    for (unsigned int i = 0; i < result_buffer.size(); i++) {

        // Fields here are 1-indexed.
        insert.bind((i * (expected_result_size + 2)) + 1, this->identifier_name);
        insert.bind((i * (expected_result_size + 2)) + 2, this->timestamp);
        for (unsigned int j = 0; j < expected_result_size; j++) {
            insert.bind((i * (expected_result_size + 2)) + j + 3, result_buffer[i][j]);
        }
    }

    while (true) {
        try {
            SQLite::Transaction t(*conn);
            if (insert.exec() != static_cast<signed> (result_buffer.size()))
                throw std::runtime_error(std::string("All results were not recorded."));
            else {
                result_buffer.clear(), result_buffer.reserve(MAXIMUM_BUFFER_SIZE), t.commit();
                return 0;
            }
        }
        catch (SQLite::Exception &) {
            // Handle collisions by waiting a random amount of time.
            std::this_thread::sleep_for(std::chrono::milliseconds(RandomDraw::draw_integer(0, 1000)));
        }
    }
}
