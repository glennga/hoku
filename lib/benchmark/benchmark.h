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
///
/// @example
/// @code{.cpp}
/// // Find all stars around a random star within 7.5 degrees of it. Rotate all stars by same random rotation.
/// Benchmark b(15, Star::chance(), Rotation::chance());
///
/// // Generate 1 random blob of size 2 degrees in diameter. Remove any stars in set above that fall in this blob.
/// b.remove_light(1, 2);
///
/// // Shift a random star (Brownian shift) with sigma = 0.3 in terms of Cartesian position.
/// b.shift_light(1, 0.3);
///
/// // Append 2 extra stars to the data-set above.
/// b.add_extra_light(2);
///
/// // View our plot as a 3D projection.
/// b.display_plot();
/// @endcode
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
        /// Name of the error being applied.
        std::string model_name;
        /// MatPlotLib color to label the affected points.
        std::string plot_color;
        /// List of stars affected by the error.
        Star::list affected;
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