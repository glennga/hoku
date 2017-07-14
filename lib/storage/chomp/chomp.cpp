/*
 * @file: chomp.cpp
 *
 * @brief: Source file for Nibble namespace, which exists to supplement the Nibble namespace and
 * provide more specific functions that facilitate the retrieval and storage of various lookup
 * tables.
 */

#include "chomp.h"

int Chomp::build_k_vector_table(SQLite::Database &db, const std::string &table,
                                const std::string &focus_column, const double m, const double q) {
    SQLite::Transaction transaction(db);
    int s_l = db.execAndGet(std::string("SELECT MAX(rowid) FROM ") + table).getInt();
    std::vector<double> s_vector;
    std::string fields, schema;

    // load all of table into RAM
    SQLite::Statement query(db, "SELECT * FROM " + table);
    s_vector.reserve((unsigned) s_l * query.getColumnCount());
    while (query.executeStep()) {
        for (int a = 0; a < query.getColumnCount(); a++) {
            s_vector.push_back(query.getColumn(a).getDouble());
        }
    }

    // load K-Vector into table, K(i) = j where s(j) <= z(i) < s(j + 1)
    Nibble::find_schema_fields(db, table, schema, fields);
    for (int a = 0; a < 100; a++) {
        std::vector<double> k_entry;
        k_entry.reserve((unsigned) query.getColumnCount() + 1);
        k_entry = Nibble::table_results_at(s_vector, (unsigned) query.getColumnCount(), a - 1);
        k_entry.push_back(0);

        auto c = (m * a) + q;
        for (int b = 0; b < s_l; b++) {
            k_entry[query.getColumnCount()] += (s_vector[b] < ((m * a) + q)) ? 1 : 0;
        }
        Nibble::insert_into_table(db, table + "_KVEC", fields + ", k_value", k_entry);
        std::cout << "\r" << "Current *A* Entry: " << a;
    }

    transaction.commit();
    return 0;
}

/*
 * Create the K-Vector for the given
 * Determine the equation for the K-Vector given the table and the focus column. Store them
 * inside the Nibble database.
 *
 * @param table Name of the table to build K-Vector for.
 * @param focus_column Name of the column to construct K-Vector with.
 * @return 0 when finished.
 */
int Chomp::create_k_vector(const std::string &table, const std::string &focus_column) {
    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    double focus_0, focus_n, m, q;
    std::string sql, fields, schema;

    // search for last element of sorted table
    std::string sql_for_max_id = std::string("SELECT MAX(rowid) FROM ") + table;
    sql = "SELECT " + focus_column + " FROM " + table + " WHERE rowid = " +
          "(" + sql_for_max_id + ")";
    focus_n = db.execAndGet(sql).getDouble();

    // search for first element of sorted table
    sql = "SELECT " + focus_column + " FROM " + table + " WHERE rowid = 1";
    focus_0 = db.execAndGet(sql).getDouble();

    // determine Z equation, this creates slightly steeper line
    m = (focus_n - focus_0 + (2.0 * DOUBLE_EPSILON)) / (db.execAndGet(sql_for_max_id).getInt());
    q = focus_0 - m - DOUBLE_EPSILON;

    // duplicate sorted table with original schema, append k_vec column
    Nibble::sort_table(table, focus_column);
    Nibble::find_schema_fields(db, table, schema, fields);
    db.exec("CREATE TABLE " + table + "_KVEC AS SELECT * FROM " + table + " WHERE 0");
    db.exec("ALTER TABLE " + table + "_KVEC ADD COLUMN k_value INT");

    return build_k_vector_table(db, table, focus_column, m, q);
}

/*
 * Search a table for the specified fields given a focus column using the K-Vector method. Searches
 * for all results between y_a and y_b. The results are a 1D array that holds a fixed number of
 * items in succession.
 *
 * **REQUIRES Chomp::create_k_vector TO HAVE BEEN RUN BEFOREHAND**
 *
 * @param db Database object containing the table you want to modify.
 * @param table Name of the table to query. Must have a kvec counterpart.
 * @param focus Table must be sorted and built upon this column. This is the search column.
 * @param fields The columns to search for in the given table.
 * @param y_a Lower bound of the focus to search for.
 * @param y_b Upper bound of the focus to search for.
 * @param expected Expected number of results * columns to be returned. Better to overshoot.
 * @return 1D list of chained results.
 */
std::vector<double> Chomp::k_vector_query(SQLite::Database &db, const std::string &table,
                                          const std::string &focus, const std::string &fields,
                                          const double y_a, const double y_b,
                                          const unsigned int expected) {
    std::string sql;
    std::vector<double> s_entries;
    double focus_0, focus_n, m, q, j_b, j_t;

    std::string sql_for_max_id = std::string("SELECT MAX(rowid) FROM ") + table;
    sql = "SELECT " + focus + " FROM " + table + " WHERE rowid = " +
          "(" + sql_for_max_id + ")";
    focus_n = db.execAndGet(sql).getDouble();

    // search for first element of sorted table
    sql = "SELECT " + focus + " FROM " + table + " WHERE rowid = 1";
    focus_0 = db.execAndGet(sql).getDouble();

    // determine Z equation
    m = (focus_n - focus_0 + (2.0 * DOUBLE_EPSILON)) / (db.execAndGet(sql_for_max_id).getInt());
    q = focus_0 - m - DOUBLE_EPSILON;

    // determine indices of search, and perform search
    j_b = floor((y_a - q) / m);
    j_t = ceil((y_b - q) / m);

    // get index to s-vector (original table)
    s_entries = Nibble::search_table(db, table + "_KVEC",
                                     "rowid BETWEEN " + std::to_string((int) j_b) + " AND " +
                                     std::to_string((int) j_t), "k_value", expected / 2);

    return Nibble::search_table(db, table, "rowid BETWEEN " +
                                           std::to_string(s_entries[0]) + " AND " +
                                           std::to_string(s_entries[s_entries.size() - 1]),
                                fields, expected);
}

///*
// * Given a vector returned by Nibble::search_table, return a single entry. We determine this
// * knowing the index we want and the length of a single result.
// *
// * @param searched Result of a Nibble::search_table call.
// * @param column_length Number of fields queried for in search_result.
// * @param index Index of result to search for. **zero-based**
// * @param Vector of size 'result_size' containing the specified result.
// */
//std::vector<double> Nibble::table_results_at(const std::vector<double> &searched,
//                                             const unsigned int column_length,
//                                             const int index) {
//    std::vector<double> result;
//    result.reserve(column_length);
//    for (unsigned int a = 0; a < column_length; a++) {
//        result.push_back(searched[(column_length * index) + a]);
//    }
//
//    return result;
//}
//
///*
// * Given a table, insert the set of values in order of the fields given. Requires an open
// * database so we don't keep changing connection.
// *
// * @param db Database object containing the table you want to modify.
// * @param table Name of the table to insert to.
// * @param fields The fields corresponding to vector of in_values.
// * @param in_values Vector of values to insert to table.
// * @return 0 when finished.
// */
//int Nibble::insert_into_table(SQLite::Database &db, const std::string &table,
//                              const std::string &fields, const std::vector<double> &in_values) {
//    std::string sql = "INSERT INTO " + table + " (" + fields + ") VALUES (";
//    for (unsigned int a = 0; a < in_values.size() - 1; a++) {
//        sql.append("?, ");
//    }
//    sql.append("?)");
//
//    // bind all the fields to the in values
//    SQLite::Statement query(db, sql);
//    for (unsigned int a = 0; a < in_values.size(); a++) {
//        query.bind(a + 1, in_values[a]);
//    }
//    query.exec();
//
//    return 0;
//}
//
//
///*
// * In the given table, sort using the selected column and create an index using the same column.
// *
// * @param table Name of the table to query.
// * @param fields The fields of the given table.
// * @param focus_column The new table will be sorted and index by this column.
// * @return 0 when finished.
// */
//int Nibble::polish_table(const std::string &table, const std::string &fields,
//                         const std::string &schema, const std::string &focus_column) {
//    SQLite::Database db(Nibble::database_location,
//                        SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
//    SQLite::Transaction transaction(db);
//    std::ostringstream sql;
//
//    // create temporary table to insert sorted data
//    sql << "CREATE TABLE " << table << "_SORTED (" << schema << ")";
//    db.exec(sql.str());
//    sql.str("");
//
//    // insert sorted data by focus_column
//    sql << "INSERT INTO " << table << "_SORTED (" << fields << ")" << " SELECT " << fields
//        << " FROM " << table << " ORDER BY " << focus_column;
//    db.exec(sql.str());
//    sql.str("");
//
//    // remove old table
//    sql << "DROP TABLE " << table;
//    db.exec(sql.str());
//    sql.str("");
//
//    // rename table to original table name
//    sql << "ALTER TABLE " << table << "_SORTED" << " RENAME TO " << table;
//    db.exec(sql.str());
//    sql.str("");
//
//    // create the index named 'TABLE_FOCUSCOLUMN'
//    sql << "CREATE INDEX " << table << "_" << focus_column << " ON " << table << "("
//        << focus_column << ")";
//    db.exec(sql.str());
//    transaction.commit();
//
//    return 0;
//}
