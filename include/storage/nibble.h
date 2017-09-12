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
#include <algorithm>
#include "../third-party/sqlite-cpp/SQLiteCpp.h"
#include "math/star.h"

/// The nibble class is used with all identification implementations. A group is stars are linked with certain
/// attributes. Inside this namespace includes the building blocks to generate entire tables, then search these tables.
///
/// The environment variable HOKU_PROJECT_PATH must point to top level of this project.
/// The file %HOKU_PROJECT_PATH%/data/bsc5.dat must exist.
///
/// @example
/// @code{.cpp}
/// Nibble nb;
///
/// // Generate the BSC5 table. If nibble.db does not exist, then create it.
/// nb.generate_bsc5_table();
///
/// /* The snippet above should only be run ONCE to populate Nibble.db and the BSC5 table. */
///
/// // Create the table 'TEST(int U, int B)'.
/// nb.create_table("TEST", "int U, int B");
///
/// // Table 'TEST' is selected from call above. Insert (10, 11) into table.
/// nb.insert_into_table("U, B", {10, 11});
///
/// // Insert the I and J components for star 3.
/// nb.insert_into_table("U, B", {nb.query_bsc5(3)[0], nb.query_bsc5(3)[1]});
///
/// // Sort the table by "U" and create an index for "U".
/// nb.polish_table("U");
///
/// // Change our working table from "TEST" to "BSC5".
/// nb.select_table("BSC5");
///
/// // Search the 'BSC5' table star 3's right ascension and declination. Expecting 2 floats, limit results by 1 row.
/// Nibble::tuple a = nb.search_table("HR = 3", "alpha, delta", 2, 1);
///
/// // Print the results of the search. Search for a[0][0] (which is star 3's alpha) and a[0][1] (star 3's delta).
/// printf("%f, %f", nb.table_results_at(a, 0, 0), nb.table_results_at(a, 0, 1));
///
/// // Find all stars near star 4 that are separated by no more than 7.5 degrees. Expecting 20 results.
/// Star::list b = nb.nearby_stars(nb.query_bsc5(4), 15, 20);
/// @endcode
class Nibble {
  private:
    friend class TestNibble;
  
  public:
    /// Alias for a SQL row of results or input. Must be real numbers.
    using tuple = std::vector<double>;
    
    /// Length of the BSC5 table. Necessary if loading all stars into RAM.
    static const int BSC5_TABLE_LENGTH = 5029;
    
    /// Maximum HR value for BSC5 table. Used in sparse representations of BSC5.
    static const int BSC5_MAX_HR = 9110;
    
    /// Open and unique database object. This must be public to work with SQLiteCpp library.
    std::unique_ptr<SQLite::Database> db;
  
  public:
    Nibble ();
    Nibble (const std::string &, const std::string & = "");
    
    void select_table (const std::string &);
    
    int generate_bsc5_table ();
    
    int insert_into_table (const std::string &, const tuple &);
    
    Star::list all_bsc5_stars ();
    Star::list nearby_stars (const Star &, const double, const unsigned int);
    
    Star query_bsc5 (const int);
    
    tuple search_table (const std::string &, const std::string &, const unsigned int, const int = -1);
    tuple search_table (const std::string &, const unsigned int, const int = -1);
    tuple table_results_at (const tuple &, const unsigned int, const int);
    
    int create_table (const std::string &, const std::string &);
    
    int find_attributes (std::string &, std::string &);
    int sort_table (const std::string &);
    int polish_table (const std::string &);
  
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
    void load_all_stars ();
    
    static std::array<double, 6> components_from_line (const std::string &);
    void parse_catalog (std::ifstream &);
};

#endif /* HOKU_NIBBLE_H */
