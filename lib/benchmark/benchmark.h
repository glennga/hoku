/*
 * @file: benchmark.h
 *
 * @brief: Header file for Benchmark class, which generates the input data for star
 * identification testing.
 */

#ifndef HOKU_BENCHMARK_H
#define HOKU_BENCHMARK_H

#include <cstdio>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "star.h"
#include "rotation.h"
#include "nibble.h"

/*
 * @class Benchmark
 * @brief Benchmark class, which generates the input data for star identification testing.
 *
 * The environment variable HOKU_PROJECT_PATH must point to top level of this project.
 * The following Python script must exist: %HOKU_PROJECT_PATH%/lib/benchmark/generate_plot.py
 *
 * The benchmark class is used for all star identification implementation testing. To imitate
 * real data from a star detector, we search for all stars in a section of the sky and apply
 * various error models to this set.
 */
class Benchmark {
        friend class TestBenchmark;

    public:
        // error model structure, used to define the type of error and the stars affected.
        struct ErrorModel {
            std::string model_name;
            std::string plot_color;
            std::vector<Star> affected;
        };

        using star_list = std::vector<Star>;

        // default constructor must not be generated, user must specify fov, focus, and rotation
        Benchmark() = delete;
        Benchmark(const double, const Star &, const Rotation & = Rotation::identity());

        // find all stars that are near the focus
        void generate_stars();

        // set stars, focus, and fov from parameters
        void present_image(star_list &, Star &, double &);

        // write current star set to file, display plot using Python's MatPlotLib
        int record_current_plot();
        int display_plot();

        // error models: stray light, blocked light, shifted stars
        void add_extra_light(const int);
        void remove_light(const int, const double);
        void shift_light(const int, const double);

    private:
        using model_list = std::vector<ErrorModel>;

        // set all of the BCS IDs in the star set to 0 and return the current star set
        star_list clean_stars();

        // shuffle current star set
        void shuffle();

        // location of plot files, requires definition of HOKU_PROJECT_PATH
        const std::string PROJECT_LOCATION = std::string(std::getenv("HOKU_PROJECT_PATH"));
        const std::string CURRENT_PLOT = PROJECT_LOCATION + "/data/cuplt.tmp";
        const std::string ERROR_PLOT = PROJECT_LOCATION + "/data/errplt.tmp";
        const std::string PLOT_SCRIPT = PROJECT_LOCATION + "/lib/benchmark/generate_plot.py";

        // all stars in 'stars' must be near the focus
        star_list stars;
        Star focus;

        // limit a star must be from focus
        double fov;

        // rotation applied to all stars, moves from inertial to image
        Rotation inertial_to_image;

        // error models, all changed stars
        model_list error_models;
};

#endif /* HOKU_BENCHMARK_H */