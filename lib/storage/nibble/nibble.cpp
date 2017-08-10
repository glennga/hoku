/// @file nibble.cpp
/// @author Glenn Galvizo
///
/// Source file for Nibble namespace, which contains functions that facilitate the retrieval and storage of various
/// lookup tables.

#include "nibble.h"

/// Constructor. This dynamically allocates a database connection object to nibble.db. If the database does not exist,
/// it is created. Set the current table to BSC5, and load all stars to RAM.
Nibble::Nibble()
{
    const int FLAGS = SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE;
    this->db = std::unique_ptr<SQLite::Database>(new SQLite::Database(DATABASE_LOCATION, FLAGS));

    // Starting table is 'BSC5'.
    this->table = "BSC5";

    // Load all stars into instance's 'all_stars'.
    load_all_stars();
}

/// Change the current working table that is being operated on.
///
/// @param table Table to be selected.
void Nibble::select_table(const std::string &table)
{
    this->table = table;
}

/// Helper method for parse_catalog method. Read the star catalog data and compute the <i, j, k> components given a
/// line from the ASCII catalog.
///
/// @param entry String containing line from ASCII catalog.
/// @return Array of components in order <alpha, delta, i, j, k, m>.
std::array<double, 6> Nibble::components_from_line(const std::string &entry)
{
    std::array<double, 6> components;

    try
    {
        // Read right ascension. Convert hr + min + sec -> deg.
        double alpha = 15 * stoi(entry.substr(75, 2), NULL) + 0.25 * stoi(entry.substr(77, 2), NULL) +
            (1 / 240.0) * strtof(entry.substr(79, 4).c_str(), NULL);

        // Read declination. No conversion necessary.
        double delta = stoi(entry.substr(84, 2), NULL) + (1 / 60.0) * stoi(entry.substr(86, 2), NULL) +
            (1 / 3600.0) * stoi(entry.substr(88, 2), NULL);
        delta *= (entry.at(83) == '-') ? -1 : 1;

        // Convert to cartesian w/ r = 1. Reduce to unit vector.
        Star star_entry(1.0 * cos((M_PI / 180.0) * delta) * cos((M_PI / 180.0) * alpha),
                        1.0 * cos((M_PI / 180.0) * delta) * sin(M_PI / 180.0 * alpha),
                        1.0 * sin((M_PI / 180.0) * delta), 0, true);

        // Parse apparent magnitude.
        double m = strtof(entry.substr(102, 5).c_str(), NULL);
        components = {alpha, delta, star_entry[0], star_entry[1], star_entry[2], m};
    }
    catch (std::exception &e)
    {
        // Ignore entries without recorded alpha or delta.
    }

    return components;
}

/// Helper function for generate_bsc5_table. Inserts into the BSC5 table given database and the filestream for the
/// catalog.
///
/// @param catalog Input file-stream used to open catalog.
void Nibble::parse_catalog(std::ifstream &catalog)
{
    int hr = 1;

    for (std::string entry; getline(catalog, entry);)
    {
        std::array<double, 6> c = components_from_line(entry);

        // Only insert if magnitude < 6.0 (visible light).
        if (c[5] < 6.0)
        {
            insert_into_table("alpha, delta, i, j, k, m, hr", {c[0], c[1], c[2], c[3], c[4], c[5], (double) hr});
        }
        hr++;
    }
}

/// Parse the right ascension, declination, visual magnitude, and HR number for each star. The i, j, and k components
/// are converted from the star's alpha, delta, and an assumed parallax = 1. **This should be the first function run to
/// generate all other tables.**
///
/// @return -1 if the table cannot be created. -2 if the catalog cannot be found. 0 otherwise.
int Nibble::generate_bsc5_table()
{
    std::ifstream catalog(CATALOG_LOCATION);
    if (!catalog.is_open()) return -2;

    SQLite::Transaction transaction(*db);
    if (create_table("BSC5", "alpha FLOAT, delta FLOAT, i FLOAT, j FLOAT, k FLOAT, m FLOAT, hr INT") == -1)
    {
        return -1;
    }

    parse_catalog(catalog);
    transaction.commit();

    // Polish table. Sort by HR number.
    return polish_table("hr");
}

/// Search the BSC5 table for the star with the matching HR number. Return a star with the vector components and HR
/// number of this search.
///
/// @param hr HR number of the star to return.
/// @return Star with the components of the matching HR number entry.
Star Nibble::query_bsc5(const int hr)
{
    sql_row results;

    select_table("BSC5");
    results = search_table("hr = " + std::to_string(hr), "i, j, k", 1, 1);
    return Star(results[0], results[1], results[2], hr);
}

/// Accessor for all_stars.
///
/// @return Array with all stars in BSC5 table.
Star::list Nibble::all_bsc5_stars()
{
    return this->all_stars;
}

/// Given a focus star and a field of view limit, find all stars around the focus.
///
/// @param focus Star to search around.
/// @param fov Limit a star must be separated from the focus by.
/// @param expected Expected number of stars around the focus. Better to overshoot.
/// @return Array with all nearby stars.
Star::list Nibble::nearby_stars(const Star &focus, const double fov, const unsigned int expected)
{
    Star::list nearby;
    nearby.reserve(expected);

    for (const Star &candidate : all_stars)
    {
        if (Star::within_angle(focus, candidate, fov)) nearby.push_back(candidate);
    }

    return nearby;
}

/// Search a table for the specified fields given a constraint. Limit results by a certain amount if desired. The
/// results returned are a 1D array that holds a fixed number of items in succession.
///
/// @param constraint The SQL string to be used with the WHERE clause.
/// @param fields The columns to search for in the current table.
/// @param expected Expected number of results * columns to be returned. Better to overshoot.
/// @param limit Limit the results searched for with this.
/// @return 1D list of chained results.
Nibble::sql_row Nibble::search_table(const std::string &constraint, const std::string &fields,
                                     const unsigned int expected, const int limit)
{
    sql_row result;
    std::string sql;

    result.reserve(expected);
    sql = "SELECT " + fields + " FROM " + table + " WHERE " + constraint;

    // Do not use the limit constraint if limit is not specified.
    if (limit > 0) sql += " LIMIT " + std::to_string(limit);

    SQLite::Statement query(*db, sql);
    while (query.executeStep())
    {
        for (int i = 0; i < query.getColumnCount(); i++)
        {
            result.push_back(query.getColumn(i).getDouble());
        }
    }

    return result;
}

/// Search a table for the specified fields given a constraint. Limit results by a certain amount if desired.
/// The results returned are a 1D array that holds a fixed number of items in succession. This function is overloaded
/// to perform a search without a constraint.
///
/// @param fields The columns to search for in the current table.
/// @param expected Expected number of results * columns to be returned. Better to overshoot.
/// @param limit Limit the results searched for with this.
/// @return 1D list of chained results.
Nibble::sql_row Nibble::search_table(const std::string &fields, const unsigned int expected, const int limit)
{
    sql_row result;
    std::string sql;

    result.reserve(expected);
    sql = "SELECT " + fields + " FROM " + table;

    // Do not use limit constraint if limit is not specified.
    if (limit > 0) sql += " LIMIT " + std::to_string(limit);

    SQLite::Statement query(*db, sql);
    while (query.executeStep())
    {
        for (int i = 0; i < query.getColumnCount(); i++)
        {
            result.push_back(query.getColumn(i).getDouble());
        }
    }

    return result;
}

/// Given a vector returned by Nibble::search_table, return a single entry. We determine this knowing the index we want
/// and the length of a single result.
///
/// @param searched Result of a Nibble::search_table call.
/// @param column_length Number of fields queried for in search_result.
/// @param index Index of result to search for. **zero-based**
/// @return Vector of size 'result_size' containing the specified result.
Nibble::sql_row Nibble::table_results_at(const sql_row &searched, const unsigned int column_length, const int index)
{
    sql_row result;
    result.reserve(column_length);
    for (unsigned int i = 0; i < column_length; i++)
    {
        result.push_back(searched[(column_length * index) + i]);
    }

    return result;
}

/// Given a table, insert the set of values in order of the fields given. Requires an open database so we don't keep
/// changing connection. This function is overloaded to accept a vector of doubles instead of strings.
///
/// @param fields The fields corresponding to vector of in_values.
/// @param in_values Vector of values to insert to table.
/// @return 0 when finished.
int Nibble::insert_into_table(const std::string &fields, const sql_row &in_values)
{
    // Create bind statement with necessary amount of '?'.
    std::string sql = "INSERT INTO " + table + " (" + fields + ") VALUES (";
    for (unsigned int a = 0; a < in_values.size() - 1; a++) sql.append("?, ");
    sql.append("?)");

    // Bind all the fields to the in values.
    SQLite::Statement query(*db, sql);
    for (unsigned int i = 0; i < in_values.size(); i++) query.bind(i + 1, in_values[i]);
    query.exec();

    return 0;
}

/// Create a table in the Nibble database with the given schema.
///
/// @param table Name of the table to create.
/// @param schema Schema for the table.
/// @return -1 if a table already exists. 0 otherwise.
int Nibble::create_table(const std::string &table, const std::string &schema)
{
    SQLite::Statement query(*db, "SELECT name FROM sqlite_master WHERE type=\'table\' AND name=\'" + table + "\'");

    select_table(table);
    while (query.executeStep())
    {
        if (query.getColumnCount() > 0) return -1;
    }

    (*db).exec("CREATE TABLE " + table + "(" + schema + ")");
    return 0;
}

/// In the current table, get the fields and schema.
///
/// @param schema Reference to string that will hold schema.
/// @param fields Reference to string that will hold fields.
/// @return 0 when finished.
int Nibble::find_schema_fields(std::string &schema, std::string &fields)
{
    fields.clear();
    schema.clear();

    SQLite::Statement query(*db, "PRAGMA table_info (" + table + ")");
    while (query.executeStep())
    {
        fields += query.getColumn(1).getString() + ", ";
        schema += query.getColumn(1).getString() + " " + query.getColumn(2).getString() + ", ";
    }

    // Remove trailing commas from fields and schema.
    for (unsigned int i = 0; i < 2; i++)
    {
        fields.pop_back();
        schema.pop_back();
    }

    return 0;
}

/// In the given table, sort using the selected column.
///
/// @param focus_column The new table will be sorted and index by this column.
/// @return 0 when finished.
int Nibble::sort_table(const std::string &focus_column)
{
    SQLite::Transaction transaction(*db);
    std::string fields, schema;

    // Grab the fields and schema of the table.
    find_schema_fields(schema, fields);

    // Create temporary table to insert sorted data. Insert the sorted data by focus column.
    (*db).exec("CREATE TABLE " + table + "_SORTED (" + schema + ")");
    (*db).exec("INSERT INTO " + table + "_SORTED (" + fields + ")" + " SELECT " + fields +
        " FROM " + table + " ORDER BY " + focus_column);

    // Remove old table. Rename the table to original table name.
    (*db).exec("DROP TABLE " + table);
    (*db).exec("ALTER TABLE " + table + "_SORTED RENAME TO " + table);
    transaction.commit();

    return 0;
}

/// In the given table, sort using the selected column and create an index using the same column.
///
/// @param focus_column The new table will be sorted and index by this column.
/// @return 0 when finished.
int Nibble::polish_table(const std::string &focus_column)
{
    sort_table(focus_column);

    // Create the index name 'TABLE_FOCUSCOLUMN'.
    SQLite::Transaction transaction(*db);
    (*db).exec("CREATE INDEX " + table + "_" + focus_column + " ON " + table + "(" + focus_column + ")");
    transaction.commit();

    return 0;
}

/// Load all of the stars in BSC5 to star_list.
void Nibble::load_all_stars()
{
    // Reserve space for the list.
    this->all_stars.reserve(BSC5_TABLE_LENGTH);

    // Select all stars, and load this into RAM.
    SQLite::Statement query(*db, "SELECT i, j, k, hr FROM BSC5");
    while (query.executeStep())
    {
        this->all_stars.push_back(Star(query.getColumn(0).getDouble(), query.getColumn(1).getDouble(),
                                       query.getColumn(2).getDouble(), query.getColumn(3).getInt()));
    }
}
