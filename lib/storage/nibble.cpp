/// @file nibble.cpp
/// @author Glenn Galvizo
///
/// Source file for Nibble class, which facilitate the retrieval and storage of various lookup tables.

#include <algorithm>
#include "storage/nibble.h"

/// Returned when a table creation is not successful.
const int Nibble::TABLE_NOT_CREATED = -1;

/// Used when a limit constraint should not be specified.
const int Nibble::NO_LIMIT = -1;

/// Returned when the result of a search returns no tuples.
const int Nibble::NO_RESULT_FOUND_EITHER = 0;

/// Constructor. This dynamically allocates a database connection object to nibble.db. If the database does not exist,
/// it is created.
Nibble::Nibble () {
    const int FLAGS = SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE;
    this->conn = std::make_unique<SQLite::Database>(std::string(std::getenv("HOKU_PROJECT_PATH")) +
                                                    "/data/nibble.db", FLAGS);
}

/// Overloaded constructor. If a table name is specified, we load this table into memory. Note that ONLY this table
/// will reside in memory upon creation. No other tables in Nibble will exist with this connection. Polish table if a
/// focus attribute is specified.
///
/// @param table_name Name of the table to load into memory.
/// @param focus Name of the focus column to polish table with.
Nibble::Nibble (const std::string &table_name, const std::string &focus) {
    const int FLAGS = SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE;

    // We have two connections: one in memory, and one to our Nibble database on disk.
    this->conn = std::make_unique<SQLite::Database>(":memory:", FLAGS);
    Nibble nb;

    // Check if the requested table exists before proceeding.
    if (!nb.does_table_exist(table_name)) {
        throw std::runtime_error(std::string("Table " + table_name + " does not exist."));
    }

    // Copy the entire table to RAM.
    nb.select_table(table_name);
    const unsigned int CARDINALITY = (*nb.conn).execAndGet(
            std::string("SELECT MAX(rowid) FROM ") + table_name).getUInt();
    tuples_d table = nb.search_table("*", CARDINALITY);

    // Determine the schema and fields for insertion. Create the table.
    std::string schema, fields;
    nb.find_attributes(schema, fields);
    if (this->create_table(table_name, schema) == TABLE_NOT_CREATED) {
        throw std::runtime_error(std::string("Unable to create specified table"));
    }

    // Copy the table from our search table to our in-memory database.
    this->select_table(table_name);
    std::for_each(table.begin(), table.end(), [this, &fields] (const tuple_d &t) -> void {
        this->insert_into_table(fields, t);
    });

    // If desired, then polish the table (index and sort).
    if (!focus.empty()) {
        this->polish_table(focus);
    }
}

/// Change the current working table that is being operated on.
///
/// @param table Table to be selected.
void Nibble::select_table (const std::string &table) {
    this->table = table;
}

/// Check if a given table exists using the current database cursor.
///
/// @param table Table to check the existence of.
/// @return True if the given table exists. False otherwise.
bool Nibble::does_table_exist (const std::string &table) {
    SQLite::Statement query(*conn, "SELECT 1 FROM sqlite_master WHERE type='table' AND name='" + table + "\'");

    return query.executeStep();
}

/// Search a table for the specified fields given a constraint. Limit results by a certain amount if desired.
///
/// @param fields The columns to search for in the current table.
/// @param constraint The SQL string to be used with the WHERE clause.
/// @param expected Expected number of results to be returned. Better to overshoot.
/// @param limit Limit the results searched for with this. Use NO_LIMIT to avoid this constraint.
/// @return List of results returned from query, ordered by tuple in table.
Nibble::tuples_d Nibble::search_table (const std::string &fields, const std::string &constraint,
                                       const unsigned int expected, const int limit) {
    tuples_d result;
    std::string sql;

    result.reserve(expected);
    sql = "SELECT " + fields + " FROM " + table + " WHERE " + constraint;

    // Do not use the limit constraint if limit is not specified.
    if (limit != NO_LIMIT) {
        sql += " LIMIT " + std::to_string(limit);
    }

    SQLite::Statement query(*conn, sql);
    while (query.executeStep()) {
        tuple_d tup;

        for (int i = 0; i < query.getColumnCount(); i++) {
            tup.push_back(query.getColumn(i).getDouble());
        }
        result.push_back(tup);
    }

    return result;
}

/// Return all specified fields from a table. Limit results by a certain amount if desired. This function is
/// overloaded to perform a search without a constraint.
///
/// @param fields The columns to search for in the current table.
/// @param expected Expected number of results to be returned. Better to overshoot.
/// @param limit Limit the results searched for with this. Use NO_LIMIT to avoid this constraint.
/// @return List of results returned from query, ordered by tuple in table.
Nibble::tuples_d Nibble::search_table (const std::string &fields, const unsigned int expected, const int limit) {
    tuples_d result;
    std::string sql;

    result.reserve(expected);
    sql = "SELECT " + fields + " FROM " + table;

    // Do not use limit constraint if limit is not specified.
    if (limit != NO_LIMIT) {
        sql += " LIMIT " + std::to_string(limit);
    }

    SQLite::Statement query(*conn, sql);
    while (query.executeStep()) {
        tuple_d tup;

        for (int i = 0; i < query.getColumnCount(); i++) {
            tup.push_back(query.getColumn(i).getDouble());
        }
        result.push_back(tup);
    }

    return result;
}

/// Query the selected table using the given constraint (if specified) for the given fields. We only return the first
/// result we find (if any).
///
/// @param fields The columns to search for in the current table.
/// @param constraint The SQL string to be used with the WHERE clause.
/// @return If there exists nothing returned from query, return NO_RESULT_FOUND. Otherwise,the first result returned
/// from query.
Nibble::either Nibble::search_single (const std::string &fields, const std::string &constraint) {
    std::string sql = "SELECT " + fields + " FROM " + table + (constraint.empty() ? "" : " WHERE " + constraint);

    SQLite::Statement query(*conn, sql);
    while (query.executeStep()) {
        // This should only execute once.
        return either {query.getColumn(0).getDouble(), 0};
    }
    return either {0, NO_RESULT_FOUND_EITHER};
}

/// Create a table in the Nibble database with the given schema.
///
/// @param table Name of the table to create.
/// @param schema Schema for the table.
/// @return TABLE_NOT_CREATED if a table already exists. 0 otherwise.
int Nibble::create_table (const std::string &table, const std::string &schema) {
    SQLite::Statement query(*conn, "SELECT name FROM sqlite_master WHERE type='table' AND name='" + table + "\'");

    select_table(table);
    while (query.executeStep()) {
        if (query.getColumnCount() > 0) {
            return TABLE_NOT_CREATED;
        }
    }

    (*conn).exec("CREATE TABLE " + table + "(" + schema + ")");
    return 0;
}

/// In the current table, get the fields and schema.
///
/// @param schema Reference to string that will hold schema.
/// @param fields Reference to string that will hold fields.
/// @return 0 when finished.
int Nibble::find_attributes (std::string &schema, std::string &fields) {
    fields.clear();
    schema.clear();

    SQLite::Statement query(*conn, "PRAGMA table_info (" + table + ")");
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

/// In the given table, sort using the selected column(s).
///
/// @param focus The new table will be sorted and index by this/these column/columns.
/// @return 0 when finished.
int Nibble::sort_table (const std::string &focus) {
    SQLite::Transaction transaction(*conn);
    std::string fields, schema;

    // Grab the fields and schema of the table.
    find_attributes(schema, fields);

    // Create temporary table to insert sorted data. Insert the sorted data by focus.
    (*conn).exec("CREATE TABLE " + table + "_SORTED (" + schema + ")");
    (*conn).exec("INSERT INTO " + table + "_SORTED (" + fields + ")" + " SELECT " + fields + " FROM " + table +
                 " ORDER BY " + focus);

    // Remove old table. Rename the table to original table name.
    (*conn).exec("DROP TABLE " + table);
    (*conn).exec("ALTER TABLE " + table + "_SORTED RENAME TO " + table);
    transaction.commit();

    return 0;
}

/// In the given table, sort using the selected column(s) and create an index using the same column(s).
///
/// @param focus The new table will be sorted and index by this column(s).
/// @return 0 when finished.
int Nibble::polish_table (const std::string &focus) {
    sort_table(focus);

    // Create the index name 'TABLE_IDX'.
    SQLite::Transaction transaction(*conn);
    (*conn).exec("CREATE INDEX " + table + "_IDX ON " + table + "(" + focus + ")");
    transaction.commit();

    return 0;
}
