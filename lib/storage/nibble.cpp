/// @file nibble.cpp
/// @author Glenn Galvizo
///
/// Source file for Nibble class, which facilitate the retrieval and storage of various lookup tables.

#include <algorithm>
#include <libgen.h>

#include "storage/nibble.h"

const int Nibble::TABLE_NOT_CREATED_RET = -1;
const int Nibble::NO_RESULT_FOUND_EITHER = 0;

Nibble::Nibble (const std::string &database_name) {
    // Automatically create the database if it does not exist.
    const int FLAGS = SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE; // NOLINT(hicpp-signed-bitwise)
    this->conn = std::make_shared<SQLite::Database>(database_name, FLAGS);
}

void Nibble::select_table (const std::string &table) { this->current_table = table; }
bool Nibble::does_table_exist (const std::string &table) {
    SQLite::Statement query(
            *conn,
            "SELECT 1 "
            "FROM sqlite_master "
            "WHERE type='table' AND name='" + table + "\'"
    );
    return query.executeStep();
}

Nibble::tuples_d Nibble::search_table (const std::string &fields, const std::string &constraint,
                                       const unsigned int expected) {
    tuples_d result;
    std::string sql =
            "SELECT " + fields +
            " FROM " + current_table +
            " WHERE " + constraint;

    result.reserve(expected);
    SQLite::Statement query(*conn, sql);
    while (query.executeStep()) {
        tuple_d tup;

        for (int i = 0; i < query.getColumnCount(); i++) tup.push_back(query.getColumn(i).getDouble());
        result.push_back(tup);
    }

    return result;
}
Nibble::tuples_d Nibble::search_table (const std::string &fields, const unsigned int expected) {
    tuples_d result;
    std::string sql =
            "SELECT " + fields +
            " FROM " + current_table;

    result.reserve(expected);
    SQLite::Statement query(*conn, sql);
    while (query.executeStep()) {
        tuple_d tup;

        for (int i = 0; i < query.getColumnCount(); i++) tup.push_back(query.getColumn(i).getDouble());
        result.push_back(tup);
    }

    return result;
}
Nibble::Either Nibble::search_single (const std::string &fields, const std::string &constraint) {
    std::string sql =
            "SELECT " + fields +
            " FROM " + current_table +
            (constraint.empty() ? "" : " WHERE " + constraint);

    SQLite::Statement query(*conn, sql);
    while (query.executeStep()) return Either{query.getColumn(0).getDouble(), 0}; // This should only execute once.
    return Either{0, NO_RESULT_FOUND_EITHER};
}

int Nibble::create_table (const std::string &table, const std::string &schema) {
    SQLite::Statement query(
            *conn,
            "SELECT name "
            "FROM sqlite_master "
            "WHERE type='table' AND name='" + table + "\'"
    );

    select_table(table);
    while (query.executeStep()) {
        if (query.getColumnCount() > 0) return TABLE_NOT_CREATED_RET;
    }

    (*conn).exec("CREATE TABLE " + table + "(" + schema + ")");
    return 0;
}

int Nibble::find_attributes (std::string &schema, std::string &fields) {
    fields.clear();
    schema.clear();

    SQLite::Statement query(*conn, "PRAGMA table_info (" + current_table + ")");
    while (query.executeStep()) {
        fields += query.getColumn(1).getString() + ", ";
        schema += query.getColumn(1).getString() + " " + query.getColumn(2).getString() + ", ";
    }

    // Remove trailing commas from fields and schema.
    for (unsigned int i = 0; i < 2; i++) {
        fields.pop_back();
        schema.pop_back();
    }

    return 0;
}

int Nibble::sort_and_index (const std::string &focus) {
    SQLite::Transaction transaction(*conn);
    std::string fields, schema;

    // Grab the fields and schema of the table.
    find_attributes(schema, fields);

    // Create temporary table to insert sorted data. Insert the sorted data by focus.
    (*conn).exec("CREATE TABLE " + current_table + "_SORTED (" + schema + ")");
    (*conn).exec(
            "INSERT INTO " + current_table + "_SORTED (" + fields + ")" +
            " SELECT " + fields +
            " FROM " + current_table +
            " ORDER BY " + focus
    );

    // Remove old table. Rename the table to original table name.
    (*conn).exec("DROP TABLE " + current_table);
    (*conn).exec("ALTER TABLE " + current_table + "_SORTED RENAME TO " + current_table);

    // Create the index name 'TABLE_IDX'.
    (*conn).exec("CREATE INDEX " + current_table + "_IDX ON " + current_table + "(" + focus + ")");
    transaction.commit();

    return 0;
}
