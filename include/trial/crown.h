/// @file crown.h
/// @author Glenn Galvizo
///
/// Header file for the crown trials. This holds the namespace of functions that allows us to test various 
/// identification methods (as a whole) and log the data.

#ifndef TRIAL_CROWN_H
#define TRIAL_CROWN_H

#include "identification/angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"

/// Contains constants and functions to test various star identification functions end to end.
namespace Crown {
    /// Attribute header that corresponds to the log file for all query trials.
    const char *const ATTRIBUTE = "IdentificationMethod,MatchSigma,QuerySigma,ShiftSigma,MBar,FalsePercentage,"
        "ComparisonCount,BenchmarkSetSize,ResultSetSize,PercentageCorrectInCleanResultSet\n";
    
    const double WORKING_FOV = 20; ///< Field of view that all our test stars must be within.
    const int CROWN_SAMPLES = 100; ///< Number of samples to retrieve for each individual trial.
    
    const double MB_MIN = 5.5; ///< Minimum magnitude bound.
    const double MB_STEP = 0.5; ///< Step to increment magnitude bound with for each variation.
    const int MB_ITER = 5; ///< Number of magnitude bound variations.
    
    const double SS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum shift sigma (using machine epsilon).
    const double SS_MULT = 3; ///< Shift sigma multiplier for each variation.
    //    const double SS_STEP = std::numeric_limits<double>::epsilon() * 5; ///< Shift sigma step for each variation.
    const int SS_ITER = 10; ///< Number of shift sigma variations.
    
    const double ES_MIN = 0; ///< Minimum percentage of extra stars to add.
    const double ES_STEP = 0.1; ///< Step to increment extra star percentage with.
    const int ES_ITER = 5; ///< Number of extra percentage variations.
    
    const std::string ANGLE_TABLE = "SEP_20"; ///< Name of table generated for Angle method.
    const std::string PLANE_TABLE = "PLANE_20"; ///< Name of table generated for Plane method.
    const std::string SPHERE_TABLE = "SPHERE_20"; ///< Name of table generated for Sphere method.
    const std::string PYRAMID_TABLE = "PYRA_20"; ///< Name of table generated for Pyramid method.
    
    void present_benchmark (Chomp &, std::random_device &, Star::list &, Star &, double = 0);
    
    void trial_angle (Chomp &, std::ofstream &);
    void trial_plane (Chomp &, std::ofstream &);
    void trial_sphere (Chomp &, std::ofstream &);
    void trial_pyramid (Chomp &, std::ofstream &);
}

#endif /* TRIAL_CROWN_H */
