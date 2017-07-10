/*
 * @file: triangleplanar.h
 *
 * @brief: Header file for TrianglePlanar class, which matches a set of body vectors (stars) to
 * their inertial counter-parts in the database.
 */

#ifndef HOKU_TRIANGLE_PLANAR_H
#define HOKU_TRIANGLE_PLANAR_H

#include "benchmark.h"
#include "trio.h"
#include <iostream>

/*
 * Angle identification parameter structure, used to define the query and match parameters.
 */
typedef struct TrianglePlanarParameters TrianglePlanarParameters;
struct TrianglePlanarParameters {
//    double query_sigma = 0.00000000001;
//    int query_limit = 5;
//
//    double match_sigma = 0.00001;
//    unsigned int match_minimum = 10;
//
    std::string table_name = "20";
    std::string nibble_location = Nibble::database_location;
};

/*
 * @class TrianglePlanar
 * @brief TrianglePlanar class, which matches a set of body vectors (stars) to their inertial
 * counter-parts in the database.
 *
 * The triangle planar class is an implementation of CaPlanar Triangle
 * Acquisition process.
 */
class PlanarTriangle {
    public:
        // ensure default constructor is **not** generated
        PlanarTriangle() = delete;

        // identity benchmark data
        static std::vector<Star> identify(const Benchmark &, const TrianglePlanarParameters &);

        // generate the separation table
        static int generate_triangle_table(const int, const std::string &);

#ifndef DEBUGGING_MODE_IS_ON
    private:
#endif
        // user is not meant to create Angle object, keep it private
        PlanarTriangle(Benchmark);

        // the data we are working with, identification parameters = tweak performance
        std::vector<Star> input;
        TrianglePlanarParameters parameters;

        // the focus and the field of view limit
        Star focus = Star(0, 0, 0);
        double fov;

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

#endif /* HOKU_TRIANGLE_PLANAR_H */