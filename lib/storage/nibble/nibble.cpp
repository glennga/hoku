/*
 * @file: nibble.cpp
 *
 * @brief: Source file for Nibble namespace, which contains functions that facilitate the
 * retrieval and storage of various lookup tables.
 */

#include "nibble.h"

/*
 * Parse the star catalog data and compute the <i, j, k> components given a line from the ASCII
 * catalog.
 *
 * @param entry String containing line from ASCII catalog.
 * @return Array of components in order <alpha, delta, i, j, k, magnitude>.
 */
std::array<double, 6> Nibble::components_from_line(const std::string &entry) {
    std::array<double, 6> components;

    try {
        // read right ascension, convert hr + min + sec -> deg
        double alpha = 15 * stoi(entry.substr(75, 2), NULL) +
                       0.25 * stoi(entry.substr(77, 2), NULL) +
                       (1 / 240.0) * strtof(entry.substr(79, 4).c_str(), NULL);

        // read declination, no conversion necessary
        double delta = stoi(entry.substr(84, 2), NULL) +
                       (1 / 60.0) * stoi(entry.substr(86, 2), NULL) +
                       (1 / 3600.0) * stoi(entry.substr(88, 2), NULL);
        delta *= (entry.at(83) == '-') ? -1 : 1;

        // convert to cartesian w/ r=1, reduce to unit vector
        Star star_entry(1.0 * cos((M_PI / 180.0) * delta) * cos((M_PI / 180.0) * alpha),
                        1.0 * cos((M_PI / 180.0) * delta) * sin(M_PI / 180.0 * alpha),
                        1.0 * sin((M_PI / 180.0) * delta), 0, true);

        // only insert if magnitude < 6.0 (visible light)
        double magnitude = strtof(entry.substr(102, 5).c_str(), NULL);
        components = {alpha, delta, star_entry[0], star_entry[1], star_entry[2], magnitude};
    }
    catch (std::exception &e) {
#if NIBBLE_DISPLAY_EXCEPTION_MESSAGES == 1
        std::cout << "Exception: " << e.what() << std::endl;
        std::cout << "Entry is not a star." << std::endl;
        // ignore entries w/o recorded alpha or delta
#endif
    }

    return components;
}

/*
 * Helper function for generate_bsc5_table. Inserts into the BSC5 table given database and the
 * filestream for the catalog.
 *
 * @param db Database object used to create BSC5 table.
 * @param catalog Input file-stream used to open catalog.
 */
void Nibble::parse_catalog(SQLite::Database &db, std::ifstream &catalog) {
    int bsc_id = 1;

    for (std::string entry; getline(catalog, entry);) {
        std::array<double, 6> components = Nibble::components_from_line(entry);

        // only insert if magnitude < 6.0 (visible light)
        if (components[5] < 6.0) {
            Nibble::insert_into_table(db, "BSC5", "alpha, delta, i, j, k, magnitude, number",
                                      {components[0], components[1], components[2],
                                       components[3], components[4], components[5]});
        }
        bsc_id++;
    }
}

/*
 * Parse the right ascension, declination, visual magnitude, and BSC ID for each star. The i, j,
 * and k components are converted from the star's alpha, delta, and an assumed parallax = 1.
 * **This should be the first function run to generate all other tables.**
 *
 * @return -2 if the catalog file cannot be opened. -1 if an exception is thrown. 0 otherwise.
 */
int Nibble::generate_bsc5_table() {
    try {
        SQLite::Database db(Nibble::database_location,
                            SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        std::ifstream catalog(Nibble::catalog_location);
        if (!catalog.is_open()) { return -2; }

        SQLite::Transaction transaction(db);
        db.exec("CREATE TABLE BSC5 (alpha FLOAT, delta FLOAT, i FLOAT, j FLOAT, k FLOAT, "
                        "magnitude FLOAT, number INT)");

        Nibble::parse_catalog(db, catalog);
        transaction.commit();
    }
    catch (std::exception &e) {
#if NIBBLE_DISPLAY_EXCEPTION_MESSAGES == 1
        std::cout << "Exception: " << e.what() << std::endl;
#endif
        return -1;
    }

    // polish table, sort by bsc_id
    return Nibble::polish_table("BSC5", "alpha, delta, i, j, k, magnitude, number",
                                "alpha FLOAT, delta FLOAT, i FLOAT, j FLOAT, k FLOAT, "
                                        "magnitude FLOAT, number INT", "number");
}

/*
 * Search the BSC5 table for the star with the matching bsc_id. Return a star with the
 * vector components and bsc_id of this search.
 *
 * @param db Database object currently active.
 * @param bsc_id BSC ID of the star to return.
 * @return Star with the components of the matching bsc_id entry.
 */
Star Nibble::query_bsc5(SQLite::Database &db, const int bsc_id) {
    std::array<double, 3> components = {0, 0, 0};
    int limit_one = 0;

    SQLite::Statement query(db, "SELECT i, j, k FROM BSC5 WHERE number = ? LIMIT 1");
    query.bind(1, bsc_id);

    // this should only execute once
    while (query.executeStep() && limit_one++ == 0) {
        for (int a = 0; a < 3; a++) {
            components[a] = query.getColumn(a);
        }
    }

    return Star(components[0], components[1], components[2], bsc_id);
}

/*
 * Search the BSC5 table for the star with the matching bsc_id. Return a star with the vector
 * components and bsc_id of this search. This function is overloaded to use a separate database
 * object.
 *
 * @param bsc_id BSC ID of the star to return.
 * @return Star with the components of the matching bsc_id entry.
 */
Star Nibble::query_bsc5(const int bsc_id) {
    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    return Nibble::query_bsc5(db, bsc_id);
}

/*
 * Return all entries in BSC5 table as stars.
 *
 * @return Array with all stars in BSC5 table.
 */
std::array<Star, 5029> Nibble::all_bsc5_stars() {
    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    std::array<Star, 5029> star_list;
    int current_position = 0;

    // select all stars, prevent array access past 5028
    SQLite::Statement query(db, "SELECT i, j, k, number FROM BSC5");
    while (query.executeStep() && current_position < 5029) {
        star_list[current_position++] = Star(query.getColumn(0).getDouble(),
                                             query.getColumn(1).getDouble(),
                                             query.getColumn(2).getDouble(),
                                             query.getColumn(3).getInt());
    }

    return star_list;
}

/*
 * Given a focus star and a field of view limit, find all stars around the focus.
 *
 * @param Database object currently open.
 * @param focus Star to search around.
 * @param fov Limit a star must be separated from the focus by.
 * @param expected Expected number of stars around the focus. Better to overshoot.
 * @return Array with all nearby stars.
 */
std::vector<Star> Nibble::nearby_stars(SQLite::Database &db, const Star &focus, const double fov,
                                       const unsigned int expected) {
    std::vector<Star> nearby;
    nearby.reserve(expected);

    for (Star candidate : Nibble::all_bsc5_stars()) {
        if (Star::within_angle(focus, candidate, fov)) {
            nearby.push_back(candidate);
        }
    }

    return nearby;
}

/*
 * Given a focus star and a field of view limit, find all stars around the focus. Overloaded to
 * open a database connection and close it here.
 *
 * @param focus Star to search around.
 * @param fov Limit a star must be separated from the focus by.
 * @param expected Expected number of stars around the focus. Better to overshoot.
 * @return Array with all nearby stars.
 */
std::vector<Star> Nibble::nearby_stars(const Star &focus, const double fov,
                                       const unsigned int expected) {
    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    return Nibble::nearby_stars(db, focus, fov, expected);
}

/*
 * Search a table for the specified fields given a constraint. Limit results by a certain amount
 * if desired. The results returned are a 1D array that holds a fixed number of items in
 * succession.
 *
 * @param table Name of the table to query.
 * @param constraint The SQL string to be used with the WHERE clause.
 * @param fields The columns to search for in the given table.
 * @param expected Expected number of results * columns to be returned. Better to overshoot.
 * @param limit Limit the results searched for with this.
 * @return 1D list of chained results.
 */
std::vector<double> Nibble::search_table(const std::string &table, const std::string &constraint,
                                         const std::string &fields, const unsigned int expected,
                                         const int limit) {
    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    std::vector<double> result;
    std::ostringstream sql;

    result.reserve(expected);
    sql << "SELECT " << fields << " FROM " << table << " WHERE " << constraint;
    if (limit > 0) {
        // do not use limit constraint if limit is not specified
        sql << " LIMIT " << limit;
    }

    SQLite::Statement query(db, sql.str());
    while (query.executeStep()) {
        for (int a = 0; a < query.getColumnCount(); a++) {
            result.push_back(query.getColumn(a).getDouble());
        }
    }

    return result;
}

/*
 * Given a vector returned by Nibble::search_table, return a single entry. We determine this
 * knowing the index we want and the length of a single result.
 *
 * @param searched Result of a Nibble::search_table call.
 * @param column_length Number of fields queried for in search_result.
 * @param index Index of result to search for. **zero-based**
 * @param Vector of size 'result_size' containing the specified result.
 */
std::vector<double> Nibble::table_results_at(const std::vector<double> &searched,
                                             const unsigned int column_length, const int index) {
    std::vector<double> result;
    result.reserve(column_length);
    for (unsigned int a = 0; a < column_length; a++) {
        result.push_back(searched[(column_length * index) + a]);
    }

    return result;
}

/*
 * Given a table, insert the set of values in order of the fields given. Requires an open
 * database so we don't keep changing connection.
 *
 * @param db Database object containing the table you want to modify.
 * @param table Name of the table to insert to.
 * @param fields The fields corresponding to vector of in_values.
 * @param in_values Vector of values to insert to table.
 * @return 0 when finished.
 */
int Nibble::insert_into_table(SQLite::Database &db, const std::string &table,
                              const std::string &fields, 
                              const std::vector<std::string> &in_values) {
    std::string sql = "INSERT INTO " + table + " (" + fields + ") VALUES (";
    for (unsigned int a = 0; a < in_values.size() - 1; a++) {
        sql.append("?, ");
    }
    sql.append("?)");

    // bind all the fields to the in values
    SQLite::Statement query(db, sql);
    for (unsigned int a = 0; a < in_values.size(); a++) {
        query.bind(a + 1, in_values[a]);
    }
    query.exec();

    return 0;
}

/*
 * Given a table, insert the set of values in order of the fields given. Requires an open
 * database so we don't keep changing connection. This function is overloaded to accept a vector
 * of doubles instead of strings.
 *
 * @param db Database object containing the table you want to modify.
 * @param table Name of the table to insert to.
 * @param fields The fields corresponding to vector of in_values.
 * @param in_values Vector of values to insert to table.
 * @return 0 when finished.
 */
int Nibble::insert_into_table(SQLite::Database &db, const std::string &table,
                              const std::string &fields,
                              const std::vector<double> &in_values) {
    std::string sql = "INSERT INTO " + table + " (" + fields + ") VALUES (";
    for (unsigned int a = 0; a < in_values.size() - 1; a++) {
        sql.append("?, ");
    }
    sql.append("?)");

    // bind all the fields to the in values
    SQLite::Statement query(db, sql);
    for (unsigned int a = 0; a < in_values.size(); a++) {
        query.bind(a + 1, in_values[a]);
    }
    query.exec();

    return 0;
}

/*
 * In the given table, sort using the selected column.
 *
 * @param table Name of the table to query.
 * @param fields The fields of the given table.
 * @param focus_column The new table will be sorted and index by this column.
 * @return 0 when finished.
 */
int Nibble::sort_table(const std::string &table, const std::string &fields,
                       const std::string &schema, const std::string &focus_column) {
    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    SQLite::Transaction transaction(db);
    std::ostringstream sql;

    // create temporary table to insert sorted data
    sql << "CREATE TABLE " << table << "_SORTED (" << schema << ")";
    db.exec(sql.str());
    sql.str("");

    // insert sorted data by focus_column
    sql << "INSERT INTO " << table << "_SORTED (" << fields << ")" << " SELECT " << fields
        << " FROM " << table << " ORDER BY " << focus_column;
    db.exec(sql.str());
    sql.str("");

    // remove old table
    sql << "DROP TABLE " << table;
    db.exec(sql.str());
    sql.str("");

    // rename table to original table name
    sql << "ALTER TABLE " << table << "_SORTED" << " RENAME TO " << table;
    db.exec(sql.str());
    sql.str("");
    transaction.commit();

    return 0;
}

/*
 * In the given table, sort using the selected column and create an index using the same column.
 *
 * @param table Name of the table to query.
 * @param fields The fields of the given table.
 * @param focus_column The new table will be sorted and index by this column.
 * @return 0 when finished.
 */
int Nibble::polish_table(const std::string &table, const std::string &fields,
                         const std::string &schema, const std::string &focus_column) {
    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    Nibble::sort_table(table, fields, schema, focus_column);

    // create the index named 'TABLE_FOCUSCOLUMN'
    std::ostringstream sql;
    SQLite::Transaction transaction(db);
    sql << "CREATE INDEX " << table << "_" << focus_column << " ON " << table << "("
        << focus_column << ")";
    db.exec(sql.str());
    transaction.commit();

    return 0;
}
