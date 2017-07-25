/*
 * @file: chomp.h
 *
 * @brief: Header file for Chomp class, which is a child class of Nibble and provides more
 * specific functions that facilitate the retrieval and storage of various lookup tables.
 */

#ifndef HOKU_CHOMP_H
#define HOKU_CHOMP_H

#include <iomanip>
#include <iostream>
#include "nibble.h"

/*
 * @class Chomp
 * @brief Chomp class, which exists to provide more specific functions that facilitate the
 * retrieval and storage of various lookup tables.
 *
 * Like Nibble, the following must hold true:
 * The environment variable HOKU_PROJECT_PATH must point to top level of this project.
 * The file %HOKU_PROJECT_PATH%/data/bsc5.dat must exist.
 *
 * The chomp class is used with nearly all identification implementations. Like Nibble, a group
 * of stars are linked with certain attributes. Unlike Nibble, the data structure and methods are
 * extended beyond that of a basic lookup table.
 */
class Chomp : private Nibble {
    public:
        // need to keep table selection and sql_row public
        using Nibble::select_table;
        using Nibble::sql_row;

        // create a K-Vector table given a focus field
        int create_k_vector(const std::string &);

        Nibble::sql_row k_vector_query(const std::string &, const std::string &,
                                       const double, const double, const unsigned int);

#ifndef DEBUGGING_MODE_IS_ON
    private:
#endif
        int build_k_vector_table(const std::string &, const double, const double);

        // standard machine epsilon for doubles, smallest possible change in precision
        const double DOUBLE_EPSILON = std::numeric_limits<double>::epsilon();
};

#endif /* HOKU_CHOMP_H */
