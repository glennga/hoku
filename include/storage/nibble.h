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
///
/// The nibble class is used with all identification implementations. A group is stars are linked with certain
/// attributes. Inside this namespace includes the building blocks to generate entire tables, then search these tables.
///
/// The environment variable HOKU_PROJECT_PATH must point to top level of this project.
///
/// @example
/// @code{.cpp}
/// Nibble nb;
///
/// // Create the table 'TEST(int U, int B)'.
/// nb.create_table("TEST", "int U, int B");
///
/// // Table 'TEST' is selected from call above. Insert (10, 11) into table.
/// nb.insert_into_table("U, B", {10, 11});
///
/// // Sort the table by "U" and create an index for "U".
/// nb.polish_table("U");
///
/// // Change our working table from "TEST" to "BRIGHT".
/// nb.select_table("BRIGHT");
///
/// // Search the 'BRIGHT' table star 3's right ascension and declination. Expecting 2 floats, limit results by 1 row.
/// Nibble::tuples_d a = nb.search_table("label = 3", "alpha, delta", 2, 1);
/// for (const tuple_d &i : a) {
///     for (const double &b : i) { std::cout << b << std::endl; }
/// }
/// @endcode
class Nibble {
  public:
    /// Alias for a SQL row of results or input, provided the results are floating numbers.
    using tuple_d = std::vector<double>;
    
    /// Alias for a set of results (tuples), provided the results are floating numbers.
    using tuples_d = std::vector<tuple_d>;
    
    /// Alias for a SQL row of input, provided the results are integers.
    using tuple_i = std::vector<int>;
    
    /// Open and unique database connection object. This must be public to work with SQLiteCpp library.
    std::unique_ptr<SQLite::Database> conn;
    
    /// Returned when the result of a search returns no tuples.
    static constexpr double NO_RESULT_FOUND = 0;
    
    /// Returned when a table creation is not successful.
    static constexpr int TABLE_NOT_CREATED = -1;
  
  public:
    Nibble ();
    explicit Nibble (const std::string &, const std::string & = "");
    
    tuples_d search_table (const std::string &, const std::string &, unsigned int, int = -1);
    tuples_d search_table (const std::string &, unsigned int, int = -1);
    
    double search_single (const std::string &, const std::string & = "");
    
    void select_table (const std::string &, bool = false);
    int create_table (const std::string &, const std::string &);
    
    int find_attributes (std::string &, std::string &);
    int sort_table (const std::string &);
    int polish_table (const std::string &);
  
  public:
    /// Using the currently selected table, insert the set of values in order of the fields given.
    ///
    /// @tparam T Type of input vector. Should be tuple_i or tuple_d.
    /// @param fields The fields corresponding to vector of in_values.
    /// @param in_values Vector of values to insert to table.
    /// @return 0 when finished.
    template <typename T>
    int insert_into_table (const std::string &fields, const T &in_values) {
        // Create bind statement with necessary amount of '?'.
        std::string sql = "INSERT INTO " + table + " (" + fields + ") VALUES (";
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

#if !defined ENABLE_TESTING_ACCESS
  protected:
#endif
    static const std::string PROJECT_LOCATION;
    static const std::string DATABASE_LOCATION;

#if !defined ENABLE_TESTING_ACCESS
  protected:
#endif
    /// Current table being operated on.
    std::string table;
};

#endif /* HOKU_NIBBLE_H */
