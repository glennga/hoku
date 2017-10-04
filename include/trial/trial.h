/// @file trial.h
/// @author Glenn Galvizo
///
/// Header file for the trials. The holds the namespace of functions that iterate through various dimensions and
/// log the data.

#ifndef HOKU_RECORD_ANGLE_H
#define HOKU_RECORD_ANGLE_H

#include "identification/angle.h"
#include "identification/astrometry-net.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"

/// Contains functions to iterate through various dimensions of identification and to log the results.
namespace Trial {
    /// Attribute header that corresponds to the log file for Angle trials.
    const char* const ANGLE_ATTRIBUTE = "SetNumber,InputSize,IdentificationSize,MatchesFound,QuerySigma,MatchSigma,"
        "MatchMinimum,ComparisonCount\n";
    
    /// Attribute header that corresponds to the log file for Plane trials.
    const char* const PLANE_ATTRIBUTE = "SetNumber,InputSize,IdentificationSize,MatchesFound,SigmaA,SigmaI,MatchSigma,"
        "MatchMinimum,QuadtreeW,ComparisonCount\n";
    
    /// Attribute header that corresponds to the log file for Sphere trials.
    const char* const SPHERE_ATTRIBUTE = "SetNumber,InputSize,IdentificationSize,MatchesFound,SigmaA,SigmaI,MatchSigma,"
        "MatchMinimum,QuadtreeW,ComparisonCount\n";
    
    /// Attribute header that corresponds to the log file for Astro trials.
    const char* const ASTRO_ATTRIBUTE = "SetNumber,InpputSize,IdentificationSize,MatchesFound,QuerySigma,MatchSigma,"
        "KdTreeW,KAcceptAlignment,UtilityFalseNegative,UtilityFalsePositive,UtilityTrueNegative,UtilityTruePositive,"
        "ComparisonCount\n";
    
    /// Attribute header that corresponds to the log file for Pyramid trials.
    const char* const PYRAMID_ATTRIBUTE = "SetNumber,InputSize,IdentificationSize,MatchesFound,QuerySigma,MatchSigma,"
        "ComparisonCount\n";

    void iterate_angle_set_n(Nibble &, std::ofstream &, unsigned int);
    void iterate_angle_parameters(Nibble &, std::ofstream &, unsigned int);

    void iterate_plane_set_n(Nibble &, std::ofstream &, unsigned int);
    void iterate_plane_parameters(Nibble &, std::ofstream &, unsigned int);
    void iterate_plane_helper (Nibble &, std::ofstream &, unsigned int);

    void iterate_sphere_set_n (Nibble &, std::ofstream &, unsigned int);
    void iterate_sphere_parameters (Nibble &, std::ofstream &, unsigned int);
    void iterate_sphere_helper (Nibble &, std::ofstream &, unsigned int);
    
    void generate_astro_trees (std::shared_ptr<KdNode> &, std::shared_ptr<KdNode> &, int);
    void iterate_astro_set_n (Nibble &, std::ofstream &, unsigned int);
    void iterate_astro_parameters (Nibble &, std::ofstream &, unsigned int);
    void iterate_astro_helper (Nibble &, std::ofstream &, unsigned int);
    
    void iterate_pyramid_set_n (Nibble &, std::ofstream &, unsigned int);
    void iterate_pyramid_parameters (Nibble &, std::ofstream &, unsigned int);
}

#endif /* HOKU_RECORD_ANGLE_H */
