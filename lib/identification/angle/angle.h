/*
 * @file: angle.h
 *
 * @brief: Header file for Angle class, which matches a set of body vectors (stars) to their
 * inertial counter-parts in the database.
 */

#ifndef HOKU_ANGLE_H
#define HOKU_ANGLE_H

#include "benchmark.h"
#include <iostream>

/*
 * Angle identification parameter structure, used to define the query and match parameters.
 */
typedef struct AngleParameters AngleParameters;
struct AngleParameters {
    double query_sigma = 0.00000000001;
    int query_limit = 5;

    double match_sigma = 0.00001;
    unsigned int match_minimum = 10;

    std::string table_name = "SEP20";
    std::string nibble_location = Nibble::database_location;
};

/*
 * @class Angle
 * @brief Angle class, which matches a set of body vectors (stars) to their inertial counter-
 * parts in the database.
 *
 * The angle class is an implementation of the identification portion of the LIS Stellar Attitude
 * Acquisition process.
 */
class Angle {
    public:
        // ensure default constructor is **not** generated
        Angle() = delete;

        // identity benchmark data
        static std::vector<Star> identify(const Benchmark &, const AngleParameters &);

        // generate the separation table
        static int generate_sep_table(const int, const std::string &);

#ifndef DEBUGGING_MODE_IS_ON
    private:
#endif
        // user is not meant to create Angle object, keep it private
        Angle(Benchmark);

        // the data we are working with, identification parameters = tweak performance
        std::vector<Star> input;
        AngleParameters parameters;
        double fov;
        Star focus = Star(0, 0, 0);

        // sort stars by distance to focus
        void sort_stars();

        // search for pair given an angle and a query limit
        std::array<int, 2> query_for_pair(const double);

        // search for pair given set of benchmark stars
        std::array<Star, 2> find_candidate_pair(SQLite::Database &, const Star &, const Star &);

        // find set of matches to benchmark given candidate set and a rotation
        std::vector<Star> find_matches(const std::vector<Star> &, const Rotation &);

        // find largest matching of inertial_a -> body_a and inertial_a -> body_b
        std::vector<Star> check_assumptions(const std::vector<Star> &, const std::array<Star, 2> &,
                                            const std::array<Star, 2> &);
};

#endif /* HOKU_ANGLE_H */