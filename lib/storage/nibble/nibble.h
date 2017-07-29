/*
 * @file: nibble.h
 *
 * @brief: Header file for Nibble class, which contain functions that facilitate the retrieval
 * and storage of various lookup tables.
 */

#ifndef HOKU_NIBBLE_H
#define HOKU_NIBBLE_H

#include <fstream>
#include <sstream>
#include <memory>
#include "SQLiteCpp.h"
#include "star.h"

/*
 * @class Nibble
 * @brief Nibble class, which contain functions that facilitate the retrieval and storage of
 * various lookup tables.
 *
 * The environment variable HOKU_PROJECT_PATH must point to top level of this project.
 * The file %HOKU_PROJECT_PATH%/data/bsc5.dat must exist.
 *
 * The nibble class is used with all identification implementations. A group is stars are linked
 * with certain attributes. Inside this namespace includes the building blocks to generate entire
 * tables, then search these tables.
 */
class Nibble {
        friend class TestNibble;

    public:
        using sql_row = std::vector<double>;
        Nibble();

        // length of BSC5 table, to know how much to reserve
        static const int BSC5_TABLE_LENGTH = 5029;

        // mutator methods for table
        void select_table(const std::string &);

        // populate the BSC5 table
        int generate_bsc5_table();

        // generic table insertion method, limit by fov if desired
        int insert_into_table(const std::string &, const sql_row &);

        // all_stars access method
        Star::list all_bsc5_stars();

        // find all stars near a given focus
        Star::list nearby_stars(const Star &, const double, const unsigned int);

        // query BSC5 for i, j, k fields given BSC ID
        Star query_bsc5(const int);

        // query a table for fields with/without a constraint, limit results by certain number
        sql_row search_table(const std::string &, const std::string &,
                             const unsigned int, const int = -1);
        sql_row search_table(const std::string &, const unsigned int, const int = -1);
        sql_row table_results_at(const sql_row &, const unsigned int, const int);

        // create a table with a given schema
        int create_table(const std::string &, const std::string &);

        // sort table by specified column and create index
        int find_schema_fields(std::string &, std::string &);
        int sort_table(const std::string &);
        int polish_table(const std::string &);

        // open database object, this needs to be public to work with SQLiteCpp
        std::unique_ptr<SQLite::Database> db;

    protected:
        // current table to operate open
        std::string table;

        // all stars in the BSC5 table, from load_all_stars method
        Star::list all_stars;

        // location of catalog and database, requires definition of HOKU_PROJECT_PATH
        const std::string PROJECT_LOCATION = std::string(std::getenv("HOKU_PROJECT_PATH"));
        const std::string CATALOG_LOCATION = PROJECT_LOCATION + "/data/bsc5.dat";
        const std::string DATABASE_LOCATION = PROJECT_LOCATION + "/data/nibble.db";

    private:
        void load_all_stars();

        // read and calculate star components from line
        static std::array<double, 6> components_from_line(const std::string &);

        // parse catalog, generate BSC5
        void parse_catalog(std::ifstream &);
};

#endif /* HOKU_NIBBLE_H */
