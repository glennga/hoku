/// @file nibble.h
/// @author Glenn Galvizo
///
/// Header file for Nibble class, which contain functions that facilitate the retrieval and storage of various
/// lookup tables.

#ifndef HOKU_NIBBLE_H
#define HOKU_NIBBLE_H

#include <memory>
#include "third-party/sqlite-cpp/SQLiteCpp.h"

#include "math/star.h"

/// @brief Class for interacting with `nibble.db` through SQLite.
class Nibble {
public:
    using tuple_d = std::vector<double>;
    using tuples_d = std::vector<tuple_d>;
    using tuple_i = std::vector<int>;

    struct Either {
        double result;
        int error;
    };

    std::unique_ptr<SQLite::Database> conn; // This must be public to work with SQLiteCpp library.

public:
    explicit Nibble (const std::string &database_name);

    tuples_d search_table (const std::string &fields, unsigned int expected);
    tuples_d search_table (const std::string &fields, const std::string &constraint, unsigned int expected);
    Either search_single (const std::string &fields, const std::string &constraint = "");

    void select_table (const std::string &table);
    bool does_table_exist (const std::string &table);
    int create_table (const std::string &table, const std::string &schema);

    int find_attributes (std::string &schema, std::string &fields);
    int sort_and_index (const std::string &focus);

    static const int TABLE_NOT_CREATED_RET;
    static const int NO_RESULT_FOUND_EITHER;

public:
    /// @tparam T Type of input vector. Should be tuple_i or tuple_d.
    template<typename T>
    int insert_into_table (const std::string &fields, const T &in_values) {
        // Create bind statement with necessary amount of '?'.
        std::string sql = "INSERT INTO " + current_table + " (" + fields + ") VALUES (";
        for (unsigned int a = 0; a < in_values.size() - 1; a++) {
            sql.append("?, ");
        }
        sql.append("?)");

        // Bind all the fields to the in values.
        SQLite::Statement query(*conn, sql);
        for (unsigned int i = 0; i < in_values.size(); i++) {
            query.bind(i + 1, in_values[i]);
        }
        query.exec();

        return 0;
    }

protected:
    std::string current_table;
};

#endif /* HOKU_NIBBLE_H */
