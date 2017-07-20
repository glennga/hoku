/*
 * @file: planar-triangle.h
 *
 * @brief: Header file for PlanarTriangle class, which matches a set of body vectors (stars) to
 * their inertial counter-parts in the database.
 */

#ifndef HOKU_TRIANGLE_PLANAR_H
#define HOKU_TRIANGLE_PLANAR_H

#include "benchmark.h"
#include "chomp.h"
#include "trio.h"
#include <iostream>

/*
 * TrianglePlanar identification parameter structure, used to define the query and match parameters.
 */
struct TrianglePlanarParameters {
    double sigma_a = 0.00000000001;
    double sigma_i = 0.00000000001;
    int query_expected = 10;

    double match_sigma = 0.00001;
    unsigned int match_minimum = 10;

    std::string table_name = "PLAN20";
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
        static Benchmark::star_list identify(const Benchmark &, const TrianglePlanarParameters &);

        // generate the separation table
        static int generate_triangle_table(const int, const std::string &);

#ifndef DEBUGGING_MODE_IS_ON
    private:
#endif
        using star_list = std::vector<Star>;
        using hr_list = std::vector<double>;
        using hr_trio = std::array<double, 3>;
        using b_index_trio = std::array<double, 3>;
        using star_trio = std::array<Star, 3>;

        // user is not meant to create Angle object, keep it private
        PlanarTriangle(Benchmark);

        // the data we are working with, identification parameters = tweak performance
        Benchmark::star_list input;
        TrianglePlanarParameters parameters;

        // the focus and the field of view limit
        Star focus = Star();
        double fov;

        // search for trio given an area and moment
        std::vector<hr_trio> query_for_trio(SQLite::Database &, const double, const double);

        // search for matching pairs to body pair, use past searches to narrow search
        std::vector<star_trio> match_stars(SQLite::Database &, const b_index_trio &);
        star_trio pivot(SQLite::Database &, const b_index_trio &,
                        const std::vector<star_trio> & = {{}});

        // find set of matches to benchmark given candidate set and a rotation
        star_list find_matches(const star_list &, const Rotation &);

        // find set of inertial frame to body frame matches
        star_list check_assumptions(const star_list &, const star_trio &, const b_index_trio &);

};

#endif /* HOKU_TRIANGLE_PLANAR_H */