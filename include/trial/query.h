/// @file query.h
/// @author Glenn Galvizo
///
/// Header file for the query trials. This holds the namespace of functions that allow us to test various querying
/// methods and log the data.

#ifndef TRIAL_QUERY_H
#define TRIAL_QUERY_H

#include "identification/angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"

/// Contains constants and functions to test various hashing methods.
///
/// Reasoning for shift sigma: Pyramid paper.
/// -- Pg. 14 "To simulate the centroiding errors, Gaussian noise of 50μrad (3 sigma) was added..."
namespace Query {
    /// Attribute header that corresponds to the log file for all query trials.
    const char *const ATTRIBUTE = "IdentificationMethod,QuerySigma,ShiftSigma,CandidateSetSize,SExistence\n";
    
    const double WORKING_FOV = 20; ///< Field of view that all our test stars must be within.
    const int QUERY_SAMPLES = 100; ///< Number of samples to retrieve for each individual trial.

    const double QS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum query sigma (machine epsilon).
    const double QS_TRIANGLE_K = 0.01; ///< Query sigma scale for triangle methods.
    
    const double SS_MIN = 0; ///< Minimum shift sigma.
    const double SS_STEP = 0.001; ///< Shift sigma step for each variation.
    const int SS_ITER = 5; ///< Number of shift sigma variations.
    
    const std::string ANGLE_TABLE = "ANG_20"; ///< Name of table generated for Angle method.
    const std::string PLANE_TABLE = "PLANE_20"; ///< Name of table generated for Plane method.
    const std::string SPHERE_TABLE = "SPHERE_20"; ///< Name of table generated for Sphere method.
    const std::string PYRAMID_TABLE = "PYRA_20"; ///< Name of table generated for Pyramid method.
    const std::string COIN_TABLE = "COIN_20"; ///< Name of the table generated for the Coin method.
    
    Star::list generate_n_stars (Chomp &, unsigned int, std::random_device &);
    
    /// Return true if the given body set exists somewhere in a collection of reference sets.
    ///
    /// @tparam T Type of set to compare with (pairs, trios, asterisms, etc...).
    /// @param r_set Reference to the reference frame sets. An in-sort place will occur for each element.
    /// @param b Reference to the body frame set. An in-sort place will occur.
    /// @return True if 'b' exists somewhere in r.
    template <typename T>
    bool set_existence (std::vector<T> &r_set, T &b) {
        for (T &r_bar : r_set) {
            std::sort(r_bar.begin(), r_bar.end()), std::sort(b.begin(), b.end());
            if (std::equal(r_bar.begin(), r_bar.end(), b.begin())) {
                return true;
            }
        }
        
        return false;
    }
    
    void trial_angle (Chomp &, std::ofstream &);
    void trial_plane (Chomp &, std::ofstream &);
    void trial_sphere (Chomp &, std::ofstream &);
    void trial_pyramid (Chomp &, std::ofstream &);
    void trial_coin (Chomp &, std::ofstream &);
}

#endif /* TRIAL_QUERY_H */
