/// @file reduction.h
/// @author Glenn Galvizo
///
/// Header file for the reduction trials. This holds the namespace of functions that allow us to test various
/// solution reduction methods and log the data.

#ifndef TRIAL_REDUCTION_H
#define TRIAL_REDUCTION_H

#include "identification/angle.h"
#include "identification/astrometry-net.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"



/// Contains constants and functions to test various solution reduction functions.
///
/// @code{.cpp}
/// Testing Spaces:                     match_sigma exists in [epsilon, ..., epsilon + 3e-10 * 20]
///                                     q_sigma exists in [epsilon, ..., epsilon + 3e-16 * 20]
///                                     e exists in [1, 2, 3, ..., 10]
///
/// Current number of reduction trials: 100 * 2 * (20 + 20*20) = 42000
/// @endcode
namespace Reduction {
    /// Attribute header that corresponds to the log file for all query trials.
    const char *const ATTRIBUTE = "IdentificationMethod,MatchSigma,ShiftSigma,OptimalConfigRotation,"
        "NonOptimalConfigRotation\n";
    
    const double WORKING_FOV = 20; ///< Field of view that all our test stars must be within.
    const int REDUCTION_SAMPLES = 100; ///< Number of samples to retrieve for each individual trial.
    
    const double MS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum match sigma (machine epsilon).
    const double MS_STEP = 3e-10; ///< Step to increment match sigma with for each variation.
    const int MS_ITER = 20; ///< Number of match sigma variations.
    
    const double SS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum shift sigma (machine epsilon).
    const double SS_STEP = 3e-16; ///< Step to increment shift sigma with for each variation.
    const int SS_ITER = 20; ///< Number of shift sigma variations.
    
    const std::string ANGLE_TABLE = "SEP_20"; ///< Name of table generated for Angle method.
    const std::string PLANE_TABLE = "PLANE_20"; ///< Name of table generated for Plane method.
    const std::string SPHERE_TABLE = "SPHERE_20"; ///< Name of table generated for Sphere method.
    
    void present_candidates (Nibble &, Star::list &, Star::list &, Rotation &, double = 0, int = 0);
    
    void trial_angle (Nibble &, std::ofstream &);
    void trial_plane (Nibble &, std::ofstream &);
    void trial_sphere (Nibble &, std::ofstream &);
}

#endif /* TRIAL_REDUCTION_H */
