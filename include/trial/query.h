/// @file query.h
/// @author Glenn Galvizo
///
/// Header file for the query trials. This holds the namespace of functions that allow us to test various querying
/// methods and log the data.

#ifndef TRIAL_QUERY_H
#define TRIAL_QUERY_H

#include "identification/angle.h"
#include "identification/astrometry-net.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"

/// Contains constants and functions to test various hashing methods.
///
/// @code{.cpp}
/// Testing Spaces:                 query_sigma exists in [epsilon, ..., epsilon * 3^20]
///                                 shift_sigma exists in [epsilon, ..., epsilon * 1.01^20]
///
/// Current number of query trials: 100 * (20 + 20*20) = 42000
/// @endcode
namespace Query {
    /// Attribute header that corresponds to the log file for all query trials.
    const char *const ATTRIBUTE = "IdentificationMethod,QuerySigma,ShiftSigma,ResultSetSize,SExistence\n";

    const double WORKING_FOV = 20; ///< Field of view that all our test stars must be within.
    const int QUERY_SAMPLES = 1; ///< Number of samples to retrieve for each individual trial.
    
    const double QS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum query sigma (machine epsilon).
    const double QS_MULT = 3; ///< Query sigma multiplier for each variation.
    const int QS_ITER = 20; ///< Number of query sigma variations.
    
    const double SS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum shift sigma (using machine epsilon).
    const double SS_STEP = std::numeric_limits<double>::epsilon() * 5; ///< Shift sigma step for each variation.
    const int SS_ITER = 10; ///< Number of shift sigma variations.
    
    const std::string ANGLE_TABLE = "SEP_20"; ///< Name of table generated for Angle method.
    const std::string PLANE_TABLE = "PLANE_20"; ///< Name of table generated for Plane method.
    const std::string SPHERE_TABLE = "SPHERE_20"; ///< Name of table generated for Sphere method.
    
    Star::list generate_n_stars (Nibble &, unsigned int, std::random_device &);
    
    void trial_angle (Nibble &, std::ofstream &);
    void trial_angle_clean (Nibble &, std::ofstream &, std::random_device &);
    void trial_angle_shift (Nibble &, std::ofstream &, std::random_device &);
    
    void trial_plane (Nibble &, std::ofstream &);
    void trial_plane_clean (Nibble &, std::ofstream &, std::random_device &);
    void trial_plane_shift (Nibble &, std::ofstream &, std::random_device &);
    
    void trial_sphere (Nibble &, std::ofstream &);
    void trial_sphere_clean (Nibble &, std::ofstream &, std::random_device &);
    void trial_sphere_shift (Nibble &, std::ofstream &, std::random_device &);
}

#endif /* TRIAL_QUERY_H */
