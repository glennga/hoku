/*
 * @file: chomp.cpp
 *
 * @brief: Source file for Chomp namespace, which exists to supplement the Nibble namespace and
 * provide more specific functions that facilitate the retrieval and storage of various lookup
 * tables.
 */

#include "chomp.h"

/*
 * A helper method for the create_k_vector function. Build the K-Vector table for the given table
 * using the specified focus column and Z-equation description. K(i) = j represents how many
 * elements j in our s_vector (the focus column) are below Z(i).
 *
 * @param table Name of table to build K-Vector for.
 * @param focus_column Name of column to act as S-Vector.
 * @param m Slope parameter of Z equation.
 * @param q Y-Intercept parameter of Z equation.
 * @return 0 when finished.
 */
int Chomp::build_k_vector_table(const std::string &table, const std::string &focus_column,
                                const double m, const double q) {
    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE);
    SQLite::Transaction transaction(db);
    int s_l = db.execAndGet(std::string("SELECT MAX(rowid) FROM ") + table).getInt();
    std::vector<double> s_vector;

    // load all of table into RAM
    SQLite::Statement query(db, "SELECT " + focus_column + " FROM " + table);
    s_vector.reserve((unsigned) s_l);
    while (query.executeStep()) {
        s_vector.emplace_back(query.getColumn(0).getDouble());
    }

    // load K-Vector into table, K(i) = j where s(j) <= z(i) < s(j + 1)
    for (int i = 0; i < s_l; i++) {
        double k_value = 0;
        for (int j = 0; j < s_l; j++) {
            k_value += (s_vector[j] < ((m * i) + q)) ? 1 : 0;
        }

        Nibble::insert_into_table(db, table + "_KVEC", "k_value", std::vector<double> {k_value});
        std::cout << "\r" << "Current *I* Entry: " << i;
    }

    // index the K-Vector column
    db.exec("CREATE INDEX " + table + "_" + focus_column + " ON " + table +
            "(" + focus_column + ")");

    transaction.commit();
    return 0;
}

/*
 * Create the K-Vector for the given for the given table using the specified focus column.
 *
 * @param table Name of the table to build K-Vector for.
 * @param focus_column Name of the column to construct K-Vector with.
 * @return 0 when finished.
 */
int Chomp::create_k_vector(const std::string &table, const std::string &focus_column) {
    Nibble::sort_table(table, focus_column);
    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    SQLite::Transaction transaction(db);
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

    // sorted table is s-vector, create k-vector and build the k-vector table
    db.exec("CREATE TABLE " + table + "_KVEC (k_value INT)");
    transaction.commit();

    return build_k_vector_table(table, focus_column, m, q);
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
Nibble::result_list Chomp::k_vector_query(SQLite::Database &db, const std::string &table,
                                          const std::string &focus, const std::string &fields,
                                          const double y_a, const double y_b,
                                          const unsigned int expected) {
    double focus_0, focus_n, m, q, j_b, j_t;
    Nibble::result_list s_endpoints;
    std::string sql;

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
    s_endpoints = Nibble::search_table(db, table + "_KVEC",
                                       "rowid BETWEEN " + std::to_string((int) j_b) + " AND " +
                                       std::to_string((int) j_t), "k_value", expected / 2);

    return Nibble::search_table(db, table, "rowid BETWEEN " +
                                           std::to_string(s_endpoints.front()) + " AND " +
                                           std::to_string(s_endpoints.back()), fields, expected);
}