/// @file chomp.h
/// @author Glenn Galvizo
///
/// Header file for Chomp class, another class to facilitate the retrieval and storage of various data. The Chomp
/// class is meant to provide more specific functions than Nibble, and also deals with data that does not reside
/// inside a database.

#ifndef HOKU_CHOMP_H
#define HOKU_CHOMP_H

#include "third-party/inih/INIReader.h"

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
    using Nibble::tuples_d;

    // For the single value search with potential errors, we define an "either" struct.
    struct either_single {
        double result; // Result associated with the computation.
        int error; // Error associated with the computation.
    };

    Chomp ();

    Chomp (const std::string &table_name, const std::string &focus);

    int generate_table (INIReader &cf, bool m_flag = true);

    int create_k_vector (const std::string &focus);

    tuples_d k_vector_query (const std::string &focus, const std::string &fields, double y_a, double y_b,
                             unsigned int expected);

    tuples_d simple_bound_query (const std::vector<std::string> &foci, const std::string &fields,
                                 const std::vector<double> &y_a, const std::vector<double> &y_b, unsigned int limit);

    Star::list nearby_bright_stars (const Vector3 &focus, double fov, unsigned int expected);

    Star::list nearby_hip_stars (const Vector3 &focus, double fov, unsigned int expected);

    Star::list bright_as_list ();

    Star query_hip (int label);

    static const unsigned int BRIGHT_TABLE_LENGTH;
    static const unsigned int HIP_TABLE_LENGTH;
    static const int TABLE_EXISTS;

#if !defined ENABLE_TESTING_ACCESS
    private:
#endif
    /// All stars in the HIP_BRIGHT table, from the 'load_all_stars' method.
    Star::list all_bright_stars;

    /// All stars in the HIP table, from the 'load_all_stars' method.
    Star::list all_hip_stars;

    /// String of the Nibble table name holding all of the bright stars in the Hipparcos.
    std::string bright_table;

    /// String of the Nibble table name holding all of the stars in the Hipparcos.
    std::string hip_table;
#if !defined ENABLE_TESTING_ACCESS
    private:
#endif
    static const double DOUBLE_EPSILON;

    int build_k_vector_table (const std::string &focus_column, double m, double q);

    void load_all_stars ();

    static std::array<double, 7> components_from_line (const std::string &entry, double y_t);

    static double year_difference (INIReader &cf);
};

#endif /* HOKU_CHOMP_H */
