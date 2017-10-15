/// @file chomp.h
/// @author Glenn Galvizo
///
/// Header file for Chomp class, another class to facilitate the retrieval and storage of various data. The Chomp
/// class is meant to provide more specific functions than Nibble, and also deals with data that does not reside
/// inside a database.

#ifndef HOKU_CHOMP_H
#define HOKU_CHOMP_H

#include <iomanip>
#include <iostream>
#include "storage/nibble.h"

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
/// sql_row a = ch.k_vector_query("I, J", 2.2, 2.3, 20);
///
/// // Print the results of the search. a[0][0] = first result's I component. a[1][1] = second result's J component.
/// printf("%f, %f", ch.table_results_at(a, 0, 0), ch.table_results_at(a, 1, 1));
///
/// // Find all stars near star 4 that are separated by no more than 7.5 degrees. Expecting 20 results.
/// Star::list b = ch.nearby_bright_stars(nb.query_hip(4), 15, 20);
/// @endcode
class Chomp : public Nibble {
  public:
    /// Length of the HIP_BRIGHT table. Necessary if loading all stars into RAM.
    static const int BRIGHT_TABLE_LENGTH = 4559;
    
    /// Length of the HIPPO2 table. Necessary if loading all stars into RAM.
    static const int HIP_TABLE_LENGTH = 117956;
    
  public:
    using Nibble::tuples_d;
    
    Chomp(bool = false);
    
    int generate_bright_table ();
    int generate_hip_table ();
    
    int create_k_vector (const std::string &);
    tuples_d k_vector_query (const std::string &, const std::string &, double, double, unsigned int);
    
    Star::list nearby_bright_stars (const Star &, const double, const unsigned int);
    Star::list nearby_hip_stars (const Star &, const double, const unsigned int);
    
    Star::list bright_as_list ();
    Star query_hip (int);
#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Standard machine epsilon for doubles. This represents the smallest possible change in precision.
    const double DOUBLE_EPSILON = std::numeric_limits<double>::epsilon();
    
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
    
    static std::array<double, 6> components_from_line (const std::string &);
};

#endif /* HOKU_CHOMP_H */
