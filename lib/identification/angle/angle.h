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
struct AngleParameters {
    double query_sigma = 0.00000000001;
    unsigned int query_limit = 5;

    double match_sigma = 0.00001;
    unsigned int match_minimum = 10;

    std::string table_name = "SEP20";
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
        static Benchmark::star_list identify(const Benchmark &, const AngleParameters &);

        // generate the separation table
        static int generate_sep_table(const int, const std::string &);

#ifndef DEBUGGING_MODE_IS_ON
    private:
#endif
        using star_list = std::vector<Star>;
        using hr_list = std::vector<double>;
        using star_pair = std::array<Star, 2>;
        using hr_pair = std::array<int, 2>;

        // user is not meant to create Angle object, keep it private
        Angle(Benchmark);

        // the data we are working with, identification parameters = tweak performance
        Benchmark::star_list input;
        AngleParameters parameters;

        // for database access
        Nibble nb;

        // the focus and the field of view limit
        Star focus;
        double fov;

        // search for pair given an angle and a query limit
        hr_pair query_for_pair(const double);

        // search for pair given set of benchmark stars
        star_pair find_candidate_pair(const Star &, const Star &);

        // find set of matches to benchmark given candidate set and a rotation
        star_list find_matches(const star_list &, const Rotation &);

        // find largest matching of inertial_a -> body_a and inertial_a -> body_b
        star_list check_assumptions(const star_list &, const star_pair &, const star_pair &);
};

#endif /* HOKU_ANGLE_H */