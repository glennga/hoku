/// @file chomp.cpp
/// @author Glenn Galvizo
///
/// Source file for Chomp class, which is a child class of Nibble and provides more specific functions that facilitate
/// the retrieval and storage of various lookup tables.

#include "chomp.h"

/// A helper method for the create_k_vector function. Build the K-Vector table for the given table using the
/// specified focus column and Z-equation description. K(i) = j represents how many elements j in our s_vector (the
/// focus column) are below Z(i).
///
/// @param focus_column Name of column to act as S-Vector.
/// @param m Slope parameter of Z equation.
/// @param q Y-Intercept parameter of Z equation.
/// @return 0 when finished.
int Chomp::build_k_vector_table (const std::string &focus_column, const double m, const double q) {
    
    int s_l = (*db).execAndGet(std::string("SELECT MAX(rowid) FROM ") + table).getInt();
    SQLite::Transaction transaction(*db);
    sql_row s_vector;
    
    // Load the entire table into RAM.
    s_vector = search_table("*", (unsigned) s_l);
    
    // Load K-Vector into table, K(i) = j where s(j) <= z(i) < s(j + 1).
    for (int i = 0; i < s_l; i++) {
        double k_value = 0;
        for (int j = 0; j < s_l; j++) {
            k_value += (s_vector[j] < ((m * i) + q)) ? 1 : 0;
        }
        
        insert_into_table("k_value", sql_row {k_value});
        std::cout << "\r" << "Current *I* Entry: " << i;
    }
    
    // Index the K-Vector column.
    (*db).exec("CREATE INDEX " + table + "_" + focus_column + " ON " + table + "(" + focus_column + ")");
    
    transaction.commit();
    return 0;
}

/// Create the K-Vector for the given for the given table using the specified focus column.
///
/// @param focus Name of the column to construct K-Vector with.
/// @return 0 when finished.
int Chomp::create_k_vector (const std::string &focus) {
    sort_table(focus);
    SQLite::Transaction transaction(*db);
    double focus_0, focus_n, m, q, n;
    std::string sql, fields, schema;
    
    // search for last and first element of sorted table
    std::string sql_for_max_id = std::string("(SELECT MAX(rowid) FROM ") + table + ")";
    focus_n = search_table("rowid = " + sql_for_max_id, focus, 1, 1)[0];
    focus_0 = search_table("rowid = 1", focus, 1, 1)[0];
    
    // determine Z equation, this creates slightly steeper line
    n = search_table("MAX(rowid)", 1)[0];
    m = (focus_n - focus_0 + (2.0 * DOUBLE_EPSILON)) / (int) n;
    q = focus_0 - m - DOUBLE_EPSILON;
    
    // sorted table is s-vector, create k-vector and build the k-vector table
    create_table(table + "_KVEC", "k_value INT");
    transaction.commit();
    
    return build_k_vector_table(focus, m, q);
}

/// Search a table for the specified fields given a focus column using the K-Vector method. Searches for all results
/// between y_a and y_b. The results are a 1D array that holds a fixed number of items in succession.
///
/// *REQUIRES Chomp::create_k_vector TO HAVE BEEN RUN BEFOREHAND**
///
/// @param focus Table must be sorted and built upon this column. This is the search column.
/// @param fields The columns to search for in the given table.
/// @param y_a Lower bound of the focus to search for.
/// @param y_b Upper bound of the focus to search for.
/// @param expected Expected number of results * columns to be returned. Better to overshoot.
/// @return 1D list of chained results.
Nibble::sql_row Chomp::k_vector_query (const std::string &focus, const std::string &fields, const double y_a,
                                       const double y_b, const unsigned int expected) {
    double focus_0, focus_n, m, q, n, j_b, j_t;
    sql_row s_endpoints;
    std::string sql, s_table = table;
    
    // Search for last and first element of sorted table.
    std::string sql_for_max_id = std::string("(SELECT MAX(rowid) FROM ") + table + ")";
    focus_n = search_table("rowid = " + sql_for_max_id, focus, 1, 1)[0];
    focus_0 = search_table("rowid = 1", focus, 1, 1)[0];
    
    // Determine Z equation, this creates slightly steeper line.
    n = search_table("MAX(rowid)", 1)[0];
    m = (focus_n - focus_0 + (2.0 * DOUBLE_EPSILON)) / (int) n;
    q = focus_0 - m - DOUBLE_EPSILON;
    
    // Determine indices of search, and perform search.
    j_b = floor((y_a - q) / m);
    j_t = ceil((y_b - q) / m);
    
    // Get index to s-vector (original table).
    select_table(s_table + "_KVEC");
    sql = "rowid BETWEEN " + std::to_string((int) j_b) + " AND " + std::to_string((int) j_t);
    s_endpoints = search_table(sql, "k_value", expected / 2);
    
    select_table(s_table);
    sql = "rowid BETWEEN " + std::to_string(s_endpoints.front()) + " AND " + std::to_string(s_endpoints.back());
    return search_table(sql, fields, expected);
}