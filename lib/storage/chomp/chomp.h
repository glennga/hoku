/*
 * @file: chomp.h
 *
 * @brief: Header file for Chomp namespace, which exists to supplement the Nibble namespace and
 * provide more specific functions that facilitate the retrieval and storage of various lookup
 * tables.
 */

#ifndef HOKU_CHOMP_H
#define HOKU_CHOMP_H

#include <iomanip>
#include <iostream>
#include "nibble.h"

/*
 * @brief Chomp namespace, which exists to supplement the Nibble namespace and provide more specific
 * functions that facilitate the retrieval and storage of various lookup tables.
 *
 * The chomp namespace is used with nearly all identification implementations. Like Nibble, a group
 * of stars are linked with certain attributes. Unlike Nibble, the data structure and methods are
 * extended beyond that of a basic lookup table.
 */
namespace Chomp {
    // k-vector table name, build k-vector equation, query a table using k-vector method
    int build_k_vector_table(const std::string &, const std::string &, const double, const double);
    int create_k_vector(const std::string &, const std::string &);
    std::vector<double> k_vector_query(SQLite::Database &, const std::string &, const std::string &,
                                       const std::string &, const double, const double,
                                       const unsigned int);

    // standard machine epsilon for doubles, smallest possible change in precision
    const double DOUBLE_EPSILON = std::numeric_limits<double>::epsilon();
}

#endif /* HOKU_CHOMP_H */
