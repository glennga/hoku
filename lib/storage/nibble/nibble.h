/*
 * @file: nibble.h
 *
 * @brief: Header file for Nibble namespace, which contain functions that facilitate the retrieval
 * and storage of various lookup tables.
 */

#ifndef HOKU_NIBBLE_H
#define HOKU_NIBBLE_H

#include <fstream>
#include <sstream>
#include "SQLiteCpp.h"
#include "star.h"

// enable exception messages if desired
//#include <iostream>
//#define NIBBLE_DISPLAY_EXCEPTION_MESSAGES 1

/*
 * @brief Nibble namespace, which contain functions that facilitate the retrieval and storage of
 * various lookup tables.
 *
 * The nibble namespace is used with all identification implementations. A group is stars are linked
 * with certain attributes. Inside this namespace includes the building blocks to generate entire
 * tables, then search these tables.
 */
namespace Nibble {
    using star_list = std::vector<Star>;
    using in_list = std::vector<double>;
    using result_list = std::vector<double>;
    using full_bsc5_star_list = std::array<Star, 5029>;

    // parse catalog, generate BSC5
    void parse_catalog(SQLite::Database &, std::ifstream &);
    int generate_bsc5_table();

    // generic table insertion method, limit by fov if desired
    int insert_into_table(SQLite::Database &, const std::string &, const std::string &,
                          const in_list &);

    // load all stars in catalog to array
    full_bsc5_star_list all_bsc5_stars();

    // find all stars near a given focus
    star_list nearby_stars(SQLite::Database &, const Star &, const double, const unsigned int);
    star_list nearby_stars(const Star &, const double, const unsigned int);

    // query BSC5 for i, j, k fields given BSC ID
    Star query_bsc5(SQLite::Database &, const int);
    Star query_bsc5(const int);

    // query a table for specified fields given a constraint, limit results by certain number
    result_list search_table(SQLite::Database &, const std::string &, const std::string &,
                             const std::string &, const unsigned int, const int = -1);
    result_list table_results_at(const result_list &, const unsigned int, const int);

    // sort table by specified column and create index
    int find_schema_fields(SQLite::Database &, const std::string &, std::string &, std::string &);
    int sort_table(const std::string &, const std::string &);
    int polish_table(const std::string &, const std::string &);

    // location of catalog and database, requires definition of HOKU_PROJECT_PATH
    static std::string catalog_location(std::string(std::getenv("HOKU_PROJECT_PATH")) +
                                        "/data/bsc5.dat");
    static std::string database_location = std::string(std::getenv("HOKU_PROJECT_PATH")) +
                                           "/data/nibble.db";

    // read and calculate star components from line
    std::array<double, 6> components_from_line(const std::string &);
}

#endif /* HOKU_NIBBLE_H */
