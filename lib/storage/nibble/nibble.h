/// @file nibble.h
/// @author Glenn Galvizo
///
/// Header file for Nibble class, which contain functions that facilitate the retrieval and storage of various
/// lookup tables.

#ifndef HOKU_NIBBLE_H
#define HOKU_NIBBLE_H

#include <fstream>
#include <sstream>
#include <memory>
#include "SQLiteCpp.h"
#include "star.h"

/// The nibble class is used with all identification implementations. A group is stars are linked with certain
/// attributes. Inside this namespace includes the building blocks to generate entire tables, then search these tables.
///
/// The environment variable HOKU_PROJECT_PATH must point to top level of this project.
/// The file %HOKU_PROJECT_PATH%/data/bsc5.dat must exist.
class Nibble
{
private:
    friend class TestNibble;

public:
    /// Alias for a SQL row of results or input. Must be real numbers.
    using sql_row = std::vector<double>;

    /// Length of the BSC5 table. Necessary if loading all stars into RAM.
    static const int BSC5_TABLE_LENGTH = 5029;

    /// Open and unique database object. This must be public to work with SQLiteCpp library.
    std::unique_ptr<SQLite::Database> db;

public:
    Nibble();

    void select_table(const std::string &);

    int generate_bsc5_table();

    int insert_into_table(const std::string &, const sql_row &);

    Star::list all_bsc5_stars();
    Star::list nearby_stars(const Star &, const double, const unsigned int);

    Star query_bsc5(const int);

    sql_row search_table(const std::string &, const std::string &,
                         const unsigned int, const int = -1);
    sql_row search_table(const std::string &, const unsigned int, const int = -1);
    sql_row table_results_at(const sql_row &, const unsigned int, const int);

    int create_table(const std::string &, const std::string &);

    int find_schema_fields(std::string &, std::string &);
    int sort_table(const std::string &);
    int polish_table(const std::string &);

protected:
    /// Current table being operated on.
    std::string table;

    /// All stars in the BSC5 table, from the 'load_all_stars' method.
    Star::list all_stars;

    /// String of the HOKU_PROJECT_PATH environment variable.
    const std::string PROJECT_LOCATION = std::string(std::getenv("HOKU_PROJECT_PATH"));

    // Path of the ASCII Yale Bright Star catalog.
    const std::string CATALOG_LOCATION = PROJECT_LOCATION + "/data/bsc5.dat";

    // Path of the Nibble database file.
    const std::string DATABASE_LOCATION = PROJECT_LOCATION + "/data/nibble.db";

private:
    void load_all_stars();

    static std::array<double, 6> components_from_line(const std::string &);
    void parse_catalog(std::ifstream &);
};

#endif /* HOKU_NIBBLE_H */
