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
/// Current number of alignment trials: 100 * 2 * (20 + 20*20) = 42000
/// @endcode
namespace Reduction {
    /// Attribute header that corresponds to the log file for all reduction trials.
    const char *const ATTRIBUTE = "IdentifcationMethod,MatchSigma,QuaternionSigma,ExtraStars,CleanCandidateSize,"
        "ResultSetSize\n";
    
    const double WORKING_FOV = 20; ///< Field of view that all our test stars must be within.
    const int QUERY_SAMPLES = 100; ///< Number of samples to retrieve for each individual trial.
    
    const double MS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum match sigma (machine epsilon).
    const double MS_STEP = 3e-10; ///< Step to increment match sigma with for each variation.
    const int MS_ITER = 20; ///< Number of match sigma variations.
    
    const double Q_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum quaternion shift sigma (machine epsilon).
    const double Q_STEP = 3e-16; ///< Step to increment quaternion shift sigma with for each variation.
    const int Q_ITER = 20; ///< Number of quaternion shift sigma variations.
    
    const int E_MIN = 1; ///< Minimum number of extra stars to append.
    const int E_STEP = 1; ///< Step to increment match sigma with for each variation.
    const int E_ITER = 10; ///< Number of extra star variations.
}



#endif /* TRIAL_REDUCTION_H */
