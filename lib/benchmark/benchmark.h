/// @file benchmark.h
/// @author Glenn Galvizo
///
/// Header file for Benchmark class, which generates the input data for star identification testing.

#ifndef HOKU_BENCHMARK_H
#define HOKU_BENCHMARK_H

#include <cstdio>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "star.h"
#include "rotation.h"
#include "nibble.h"

/// The benchmark class is used for all star identification implementation testing. To imitate real data from a star
/// detector, we search for all stars in a section of the sky and apply various error models to this set.
///
/// The environment variable HOKU_PROJECT_PATH must point to top level of this project.
/// The following Python script must exist: %HOKU_PROJECT_PATH%/lib/benchmark/generate_plot.py
class Benchmark
{
private:
    friend class TestBenchmark;
    friend class TestAngle;
    friend class TestPlanarTriangle;

public:
    /// Error model structure. Defines the type of error and the stars affected.
    struct ErrorModel
    {
        std::string model_name; /// Name of the error being applied.
        std::string plot_color; /// MatPlotLib color to label the affected points.
        Star::list affected;    /// List of stars affected by the error.
    };

    /// Default constructor must not be generated. User must specify fov, focus, and rotation.
    Benchmark() = delete;

    Benchmark(const double, const Star &, const Rotation & = Rotation::identity());

    void generate_stars();

    void present_image(Star::list &, Star &, double &);

    int record_current_plot();
    int display_plot();

    void add_extra_light(const int);
    void remove_light(const int, const double);
    void shift_light(const int, const double);

private:
    /// Alias for the list (stack) of ErrorModels.
    using model_list = std::vector<ErrorModel>;

    Star::list clean_stars();
    void shuffle();

    /// String of the HOKU_PROJECT_PATH enviroment variable
    const std::string PROJECT_LOCATION = std::string(std::getenv("HOKU_PROJECT_PATH"));

    /// Path of the 'clean' star set on disk. One of the temporary files used for plotting.
    const std::string CURRENT_PLOT = PROJECT_LOCATION + "/data/cuplt.tmp";

    /// Path of the 'error' star set on disk. One of the temporary files used for plotting.
    const std::string ERROR_PLOT = PROJECT_LOCATION + "/data/errplt.tmp";

    /// Path of the Python script used to plot the current Benchmark instance.
    const std::string PLOT_SCRIPT = PROJECT_LOCATION + "/lib/benchmark/generate-plot.py";

    /// Current list of stars. All stars must be near the focus.
    Star::list stars;

    /// The focal point of the star list. Does not necessarily have to be a BSC5 star itself.
    Star focus;

    /// All stars must be fov degrees from the focus.
    double fov;

    /// Rotation applied to all stars. Moves stars from inertial to image.
    Rotation inertial_to_image;

    /// List (stack) of ErrorModels, which also holds all changed stars.
    model_list error_models;
};

#endif /* HOKU_BENCHMARK_H */