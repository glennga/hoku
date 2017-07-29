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
 * @class TrianglePlanar
 * @brief TrianglePlanar class, which matches a set of body vectors (stars) to their inertial
 * counter-parts in the database.
 *
 * The triangle planar class is an implementation of Crassidis and Cole's Planar Triangle Pattern
 * Recognition Process.
 */
class PlanarTriangle {
        friend class TestPlanarTriangle;

    public:
        // defines the query and match operations
        struct Parameters {
            double sigma_a = 0.00000000001;
            double sigma_i = 0.00000000001;
            int query_expected = 10;

            double match_sigma = 0.00001;
            unsigned int match_minimum = 10;

            std::string table_name = "PLAN20";
        };

        // ensure default constructor is **not** generated
        PlanarTriangle() = delete;

        // identity benchmark data
        static Star::list identify(const Benchmark &, const Parameters &);

        // generate the separation table
        static int generate_triangle_table(const int, const std::string &);

#ifndef DEBUGGING_MODE_IS_ON
    private:
#endif
        using hr_list = std::vector<double>;
        using hr_trio = std::array<double, 3>;
        using index_trio = std::array<double, 3>;

        // user is not meant to create Angle object, keep it private
        PlanarTriangle(Benchmark);

        // the data we are working with, identification parameters = tweak performance
        Star::list input;
        Parameters parameters;

        // for database access
        Chomp ch;

        // the focus and the field of view limit
        Star focus = Star();
        double fov;

        // search for trio given an area and moment
        std::vector<hr_trio> query_for_trio(const double, const double);

        // search for matching pairs to body pair, use past searches to narrow search
        std::vector<Trio::stars> match_stars(const index_trio &);
        Trio::stars pivot(const index_trio &, const std::vector<Trio::stars> & = {{}});

        // find set of matches to benchmark given candidate set and a rotation
        Star::list find_matches(const Star::list &, const Rotation &);

        // find set of inertial frame to body frame matches
        Star::list check_assumptions(const Star::list &, const Trio::stars &, const index_trio &);

};

typedef PlanarTriangle Plane;

#endif /* HOKU_TRIANGLE_PLANAR_H */