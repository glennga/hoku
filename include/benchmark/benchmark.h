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
#include "math/star.h"
#include "math/rotation.h"
#include "storage/chomp.h"

/// The benchmark class is used for all star identification implementation testing. To imitate real data from a star
/// detector, we search for all stars in a section of the sky and apply various error models to this set.
///
/// The environment variable HOKU_PROJECT_PATH must point to top level of this project.
/// The following Python script must exist: %HOKU_PROJECT_PATH%/lib/benchmark/generate_plot.py
///
/// @example
/// @code{.cpp}
/// // Find all bright stars around a random star within 7.5 degrees of it. Rotate all stars by same random rotation.
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
class Benchmark {
  public:
    /// Error model structure. Defines the type of error and the stars affected.
    struct ErrorModel {
        std::string model_name; ///< Name of the error being applied.
        std::string plot_color; ///< MatPlotLib color to label the affected points.
        Star::list affected; ///< List of stars affected by the error.
    };
    
    /// Default constructor must not be generated. User must specify fov, focus, and rotation.
    Benchmark () = delete;
  
  public:
    Benchmark (Chomp &, std::random_device &, double, double = 6.0);
    Benchmark (Chomp &, std::random_device &, const Star &, const Rotation &, double, double = 6.0);
    
    void generate_stars (Chomp &, double = 6.0);
    
    void present_image (Star::list &, double &) const;
    
    int record_current_plot ();
    int display_plot ();
    
    void add_extra_light (int, bool = false);
    void shift_light (int, double, bool = false);
    void remove_light (int, double);
    
    static int compare_stars (const Benchmark &, const Star::list &);

#if !defined ENABLE_IDENTIFICATION_ACCESS && !defined ENABLE_TESTING_ACCESS
  private:
#endif
    Benchmark (std::random_device &, const Star::list &, const Star &, double);
    
    Star::list clean_stars () const;
    void shuffle ();

#if !defined ENABLE_IDENTIFICATION_ACCESS && !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// String of the HOKU_PROJECT_PATH environment variable.
    const std::string PROJECT_LOCATION = std::string(std::getenv("HOKU_PROJECT_PATH"));
    
    /// String of the current plot temp file.
    const std::string CURRENT_TMP = PROJECT_LOCATION + "/data/logs/tmp/cuplt.tmp";
    
    /// String of the error plot temp file.
    const std::string ERROR_TMP = PROJECT_LOCATION + "/data/logs/tmp/errplt.tmp";
    
    /// Location of the Python benchmark plotter.
    const std::string PLOT_SCRIPT = "\"" + PROJECT_LOCATION + "/script/python/generate-plot.py\"";
    
    /// Alias for the list (stack) of ErrorModels.
    using model_list = std::vector<ErrorModel>;
    
    /// Random device pointer, used as source of randomness.
    std::random_device *seed;
    
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