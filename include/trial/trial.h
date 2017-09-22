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
    /// All trial functions require a Nibble connection and the number of the working benchmark.
    struct WorkingBenchmark {
        std::shared_ptr<Nibble> nb; ///< Pointer to a connection to Nibble. Only requires the BENCH table.
        const unsigned int set_n; ///< Working benchmark number.
    };
    
    /// Attribute header that corresponds to the log file for Angle trials.
    const std::string ANGLE_ATTRIBUTE = "SetNumber,InputSize,IdentificationSize,MatchesFound,QuerySigma,MatchSigma,"
        "MatchMinimum\n";
    
    /// Attribute header that corresponds to the log file for Plane trials.
    const std::string PLANE_ATTRIBUTE = "SetNumber,InputSize,IdentificationSize,MatchesFound,SigmaA,SigmaI,MatchSigma,"
        "MatchMinimum,QuadtreeW\n";
    
    /// Attribute header that corresponds to the log file for Sphere trials.
    const std::string SPHERE_ATTRIBUTE = "SetNumber,InputSize,IdentificationSize,MatchesFound,SigmaA,SigmaI,MatchSigma,"
        "MatchMinimum,QuadtreeW\n";
    
    /// Attribute header that corresponds to the log file for Astro trials.
    const std::string ASTRO_ATTRIBUTE = "SetNumber,InpputSize,IdentificationSize,MatchesFound,QuerySigma,MatchSigma,"
        "KdTreeW,KAcceptAlignment,UtilityFalseNegative,UtilityFalsePositive,UtilityTrueNegative,UtilityTruePositive";
    
    /// Attribute header that corresponds to the log file for Pyramid trials.
    const std::string PYRAMID_ATTRIBUTE = "SetNumber,InputSize,IdentificationSize,MatchesFound,QuerySigma,MatchSigma";
    
    void record_angle (WorkingBenchmark &, std::ofstream &);
    
    void record_plane (WorkingBenchmark &, std::ofstream &);
    void record_plane_as_ms_ms (WorkingBenchmark &, std::ofstream &, std::shared_ptr<QuadNode> &, Plane::Parameters &);
    
    void record_sphere (WorkingBenchmark &, std::ofstream &);
    void record_sphere_as_ms_ms (WorkingBenchmark &, std::ofstream &, std::shared_ptr<QuadNode> &,
                                 Sphere::Parameters &);
    
    void record_astro (WorkingBenchmark &, std::ofstream &);
    void record_astro_ka_qs_ms (WorkingBenchmark &, std::ofstream &, std::shared_ptr<KdNode> &,
                                std::shared_ptr<KdNode> &, Astro::Parameters &);
    
    void record_pyramid (WorkingBenchmark &, std::ofstream &);
}

#endif /* HOKU_RECORD_ANGLE_H */
