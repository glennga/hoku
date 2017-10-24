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
namespace Alignment {
    /// Attribute header that corresponds to the log file for all alignment trials.
    const char *const ATTRIBUTE = "IdentificationMethod,MatchSigma,ShiftSigma,MBar,OptimalConfigRotation,"
        "NonOptimalConfigRotation,OptimalComponentError,NonOptimalComponentError\n";
    
    const double WORKING_FOV = 20; ///< Field of view that all our test stars must be within.
    const int ALIGNMENT_SAMPLES = 1000; ///< Number of samples to retrieve for each individual trial.
    
    const double MS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum query sigma (machine epsilon).
    const double MS_MULT = 3; ///< Query sigma multiplier for each variation.
//    const double MS_STEP = 3e-10; ///< Step to increment match sigma with for each variation.
    const int MS_ITER = 20; ///< Number of query sigma variations.
    
    const double SS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum shift sigma (using machine epsilon).
    const double SS_MULT = 3; ///< Shift sigma multiplier for each variation.
    //    const double SS_STEP = std::numeric_limits<double>::epsilon() * 5; ///< Shift sigma step for each variation.
    const int SS_ITER = 10; ///< Number of shift sigma variations.
    
    const double MB_MIN = 5.5; ///< Minimum magnitude bound.
    const double MB_STEP = 0.5; ///< Step to increment magnitude bound with for each variation.
    const int MB_ITER = 5; ///< Number of magnitude bound variations.
    
    const std::string ANGLE_TABLE = "SEP_20"; ///< Name of table generated for Angle method.
    const std::string PLANE_TABLE = "PLANE_20"; ///< Name of table generated for Plane method.
    const std::string SPHERE_TABLE = "SPHERE_20"; ///< Name of table generated for Sphere method.
    const std::string PYRAMID_TABLE = "PYRA_20"; ///< Name of table generated for Pyramid method.
    
    void present_stars (Chomp &, std::random_device &, Star::list &, Star::list &, Rotation &, const double);
    void shift_body (std::random_device &, Star::list &, const double, const int);
    
    void trial_angle (Chomp &, std::ofstream &);
    void trial_plane (Chomp &, std::ofstream &);
    void trial_sphere (Chomp &, std::ofstream &);
    void trial_pyramid (Chomp &, std::ofstream &);
}

#endif /* TRIAL_ALIGNMENT_H */
