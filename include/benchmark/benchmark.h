/// @file benchmark.h
/// @author Glenn Galvizo
///
/// Header file for Benchmark class, which generates the input data for star identification testing.

#ifndef HOKU_BENCHMARK_H
#define HOKU_BENCHMARK_H

#include <cstdio>
#include <algorithm>
#include <sstream>
#include <random>
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
    
    /// Indicates that a given identification object does not have an designated field of view.
    static constexpr double NO_FOV = -1;
    
    /// The default apparent magnitude, minimum brightness from Earth.
    static constexpr double DEFAULT_M_BAR = 6.0;
  
  public:
    Benchmark (Chomp &ch, double fov, double m_bar = DEFAULT_M_BAR);
    Benchmark (Chomp &ch, const Star &focus, const Rotation &q, double fov, double m_bar = DEFAULT_M_BAR);
    Benchmark (const Star::list &s, const Star &focus, double fov);
    static const Benchmark black ();
    
    void generate_stars (Chomp &ch, double m_bar = DEFAULT_M_BAR);
    void present_image (Star::list &image_s, double &image_fov) const;
    
    void record_current_plot ();
    void display_plot ();
    
    void add_extra_light (unsigned int n, bool cap_error = false);
    void shift_light (unsigned int n, double sigma, bool cap_error = false);
    void remove_light (unsigned int n, double psi);
    void barrel_light (double alpha);
    
    static int compare_stars (const Benchmark &, const Star::list &);

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    Star::list clean_stars () const;
    void shuffle ();
    
    static const std::string PROJECT_LOCATION;
    static const std::string CURRENT_TMP;
    static const std::string ERROR_TMP;
    static const std::string PLOT_SCRIPT;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Alias for the list (stack) of ErrorModels.
    using model_list = std::vector<ErrorModel>;
    
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