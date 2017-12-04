/// @file semi-crown.h
/// @author Glenn Galvizo
///
/// Header file for the semi-crown trials. This holds the namespace of functions that allows us to test various
/// identification methods (from beginning to the attitude determination step) and log the data.

#ifndef TRIAL_SEMI_CROWN_H
#define TRIAL_SEMI_CROWN_H

#include "identification/angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"
#include "identification/coin.h"

/// Contains constants and functions to test various star identification functions (end to almost end).
///
/// Reasoning for magnitude choice: https://www.hindawi.com/journals/ijae/2013/505720/
/// -- Right after eq. 1 "...typical star tracker sensitivity is up to visual magnitude of 6.5/7..."
///
/// Reasoning for shift sigma: Pyramid paper.
/// -- Pg. 14 "To simulate the centroiding errors, Gaussian noise of 50μrad (3 sigma) was added..."
///
/// Reasoning for extra stars: Pyramid paper.
/// -- Pg. 14 "... and up to 24 random spikes."
namespace SemiCrown {
    /// Attribute header that corresponds to the log file for all semi-crown trials.
    const char *const ATTRIBUTE = "IdentificationMethod,MatchSigma,QuerySigma,ShiftSigma,CameraSensitivity,FalseStars,"
        "ComparisonCount,RotationalAngle\n";
    
    const double WORKING_FOV = 20; ///< Field of view that all our test stars must be within.
    const int CROWN_SAMPLES = 1000; ///< Number of samples to retrieve for each individual trial.
    
    const double MB_MIN = 6.0; ///< Minimum magnitude bound.
    const double MB_STEP = 0.25; ///< Step to increment magnitude bound with for each variation.
    const int MB_ITER = 5; ///< Number of magnitude bound variations.
    
    const double SS_MULT = 0.00000001; ///< Shift sigma multiplier for each variation.
//    const double SS_STEP = 0.001; ///< Shift sigma step for each variation.
    const int SS_ITER = 5; ///< Number of shift sigma variations.
    
    const int ES_MIN = 0; ///< Minimum number of extra stars to add.
    const int ES_STEP = 5; ///< Step to increment extra stars with.
    const int ES_ITER = 5;  ///< Number of extra stars variations.
    
    const std::string ANGLE_TABLE = "ANG_20"; ///< Name of table generated for Angle method.
    const std::string PLANE_TABLE = "PLANE_20"; ///< Name of table generated for Plane method.
    const std::string SPHERE_TABLE = "SPHERE_20"; ///< Name of table generated for Sphere method.
    const std::string PYRAMID_TABLE = "PYRA_20"; ///< Name of table generated for Pyramid method.
    const std::string COIN_TABLE = "COIN_20"; ///< Name of the table generated for the Coin method.
    
    void present_benchmark (Chomp &, std::random_device &, Star::list &, Star &, double, Rotation &);
    
    void trial_angle (Chomp &, std::ofstream &);
    void trial_plane (Chomp &, std::ofstream &);
    void trial_sphere (Chomp &, std::ofstream &);
    void trial_pyramid (Chomp &, std::ofstream &);
    void trial_coin (Chomp &, std::ofstream &);
}

#endif /* TRIAL_SEMI_CROWN_H */
