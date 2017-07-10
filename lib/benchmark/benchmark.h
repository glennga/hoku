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
 * Error model structure, used to define the type of error and the stars affected.
 */
typedef struct ErrorModel ErrorModel;
struct ErrorModel {
    std::string model_name;
    std::string plot_color;
    std::vector<Star> affected;
};

/*
 * @class Benchmark
 * @brief Benchmark class, which generates the input data for star identification testing.
 *
 * The benchmark class is used for all star identification implementation testing. To imitate
 * real data from a star detector, we search for all stars in a section of the sky and apply
 * various error models to this set.
 */
class Benchmark {
    public:
        // default constructor must not be generated, user must specify fov, focus, and rotation
        Benchmark() = delete;
        Benchmark(const double, const Star &, const Rotation & = Rotation::identity());

        // find all stars that are near the focus
        void generate_stars();

        // set all of the BCS IDs in the star set to 0 and return the current star set
        std::vector<Star> present_stars();

        // accessor methods for fov and focus
        double get_fov();
        Star get_focus();
        
        // write current star set to file, display plot using Python's MatPlotLib
        int record_current_plot();
        int display_plot(const std::string &, const std::string &, const std::string &);

        // error models: stray light, blocked light, shifted stars
        void add_extra_light(const int);
        void remove_light(const int, const double);
        void shift_light(const int, const double);

#ifndef DEBUGGING_MODE_IS_ON
    private:
#endif
        // shuffle current star set
        void shuffle();

        // files used for plotting
        std::string current_plot;
        std::string error_plot;

        // all stars in 'stars' must be near the focus
        std::vector<Star> stars;
        Star focus = Star(0, 0, 0);

        // limit a star must be from focus
        double fov;

        // rotation applied to all stars, moves from inertial to image
        Rotation inertial_to_image;

        // error models, all changed stars
        std::vector<ErrorModel> error_models;
};

#endif /* HOKU_BENCHMARK_H */