/// @file alignment.h
/// @author Glenn Galvizo
///
/// Header file for the alignment trials. This holds the namespace of functions that allow us to test various
/// attitude determination methods and log the data.

#ifndef TRIAL_ALIGNMENT_H
#define TRIAL_ALIGNMENT_H

#include "identification/angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"

/// Contains constants and functions to test various attitude determination functions.
///
/// @code{.cpp}
/// Testing Spaces:                     match_sigma exists in [epsilon, ..., epsilon + 3e-10 * 20]
///                                     shift_sigma exists in [epsilon, ..., epsilon + 3e-16 * 20]
///                                     magnitude_bound exists in [4.0, 4.5, ..., 8.0]
///
/// Current number of alignment trials: 100 * 9 * (20 + 20*20) = 378,000
/// @endcode
namespace Alignment {
    /// Attribute header that corresponds to the log file for all alignment trials.
    const char *const ATTRIBUTE = "IdentificationMethod,MatchSigma,ShiftSigma,MBar,OptimalConfigRotation,"
        "NonOptimalConfigRotation\n";
    
    const double WORKING_FOV = 20; ///< Field of view that all our test stars must be within.
    const int ALIGNMENT_SAMPLES = 100; ///< Number of samples to retrieve for each individual trial.
    
    const double MS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum match sigma (machine epsilon).
    const double MS_STEP = 3e-10; ///< Step to increment match sigma with for each variation.
    const int MS_ITER = 20; ///< Number of match sigma variations.
    
    const double SS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum shift sigma (machine epsilon).
    const double SS_STEP = 3e-16; ///< Step to increment shift sigma with for each variation.
    const int SS_ITER = 20; ///< Number of shift sigma variations.
    
    const double MB_MIN = 4.0; ///< Minimum magnitude bound.
    const double MB_STEP = 0.5; ///< Step to increment magnitude bound with for each variation.
    const int MB_ITER = 9; ///< Number of magniude bound variations.
    
    const std::string ANGLE_TABLE = "SEP_20"; ///< Name of table generated for Angle method.
    const std::string PLANE_TABLE = "PLANE_20"; ///< Name of table generated for Plane method.
    const std::string SPHERE_TABLE = "SPHERE_20"; ///< Name of table generated for Sphere method.
    
    void present_candidates (Chomp &, std::random_device &, Star::list &, Star::list &, Rotation &, double);
    void shift_candidates (std::random_device &, Star::list &, double, int);
    
    void trial_angle (Chomp &, std::ofstream &);
    void trial_plane (Chomp &, std::ofstream &);
    void trial_sphere (Chomp &, std::ofstream &);
}

#endif /* TRIAL_ALIGNMENT_H */
