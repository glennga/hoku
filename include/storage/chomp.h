/// @file chomp.h
/// @author Glenn Galvizo
///
/// Header file for Chomp class, another class to facilitate the retrieval and storage of various data. The Chomp
/// class is meant to provide more specific functions than Nibble, and also deals with data that does not reside
/// inside a database.

#ifndef HOKU_CHOMP_H
#define HOKU_CHOMP_H

#include "storage/nibble.h"

/// @brief Class for accessing the Hipparcos catalog.
///
/// The chomp class is used with nearly all identification implementations. Like Nibble, a group of stars are linked
/// with certain attributes. Unlike Nibble, the data structure and methods are extended beyond that of a basic lookup
/// table.
///
/// Like Nibble, the following must hold true:
/// The environment variable HOKU_PROJECT_PATH must point to top level of this project.
/// The file %HOKU_PROJECT_PATH%/data/hip2.dat must exist.
///
/// @example
/// @code{.cpp}
/// Chomp ch;
///
/// // Create a K-Vector for the bright stars table, with future queries focused on the "m" column.
/// ch.select_table(BRIGHT_TABLE);
/// ch.create_k_vector("M");
///
/// // Search the bright stars table for I and J components of all stars with 2.2 < m <2.3. Expecting 20 floats total.
/// tuples_d a = ch.k_vector_query("I, J", 2.2, 2.3, 20);
///
/// // Print the results of the search.
/// for (const t
/// std::cout << ch.table_results_at(a, 0, 0) + ","  + ch.table_results_at(a, 1, 1) << std::endl;
///
/// // Find all stars near star 4 that are separated by no more than 7.5 degrees. Expecting 20 results.
/// Star::list b = ch.nearby_bright_stars(nb.query_hip(4), 15, 20);
/// @endcode
class Chomp : public Nibble {
  public:
    /// Length of the bright stars table. Necessary if loading all stars into RAM.
    static constexpr unsigned int BRIGHT_TABLE_LENGTH = 4559;
    
    /// Length of the general stars table. Necessary if loading all stars into RAM.
    static constexpr unsigned int HIP_TABLE_LENGTH = 117956;
    
    /// Largest catalog ID in the bright stars table.
    static constexpr unsigned int BRIGHT_TABLE_MAX_LABEL = 117930;
    
    /// Smallest catalog ID in the bright stars table.
    static constexpr unsigned int BRIGHT_TABLE_MIN_LABEL = 88;
    
    /// Returned from table generators when the table already exists in the database.
    static constexpr int TABLE_EXISTS = -1;
  
  public:
    using Nibble::tuples_d;
    
    Chomp ();
    
    int generate_bright_table ();
    int generate_hip_table ();
    
    int create_k_vector (const std::string &);
    tuples_d k_vector_query (const std::string &, const std::string &, double, double, unsigned int);
    
    tuples_d simple_bound_query (const std::string &, const std::string &, double, double, unsigned int);
    
    Star::list nearby_bright_stars (const Star &, double, unsigned int);
    Star::list nearby_hip_stars (const Star &, double, unsigned int);
    
    Star::list bright_as_list ();
    Star::list hip_as_list ();
    Star query_hip (int);
    
    static const Star NONEXISTENT_STAR;
    static const Star::list NONEXISTENT_STAR_LIST;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// All stars in the HIP_BRIGHT table, from the 'load_all_stars' method.
    Star::list all_bright_stars;
    
    /// All stars in the HIP table, from the 'load_all_stars' method.
    Star::list all_hip_stars;
    
    /// String of the Nibble table name holding all of the bright stars in the Hipparcos.
    const std::string BRIGHT_TABLE = "HIP_BRIGHT";
    
    /// String of the Nibble table name holding all of the stars in the Hipparcos.
    const std::string HIP_TABLE = "HIP";
    
    // Path of the ASCII Hipparcos Star catalog.
    const std::string HIP_CATALOG_LOCATION = PROJECT_LOCATION + "/data/hip2.dat";

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    int build_k_vector_table (const std::string &, double, double);
    void load_all_stars ();
    
    static std::array<double, 7> components_from_line (const std::string &);
    static const double DOUBLE_EPSILON;
};

#endif /* HOKU_CHOMP_H */
