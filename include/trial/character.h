/// @file character.h
/// @author Glenn Galvizo
///
/// Header file for the trials. This holds all of the namespaces that define identification working characteristics.

/// Defining characteristics of the angle identification.
///
/// @code{.cpp}
/// Current number of permutations: (0.0000001 - 0.00000000000001) / 0.00000003    // 4
///                                 (0.000001 - 0.00000000000001) / 0.0000003      // 4
///                                 (30 - 3) / 5                                   // 6
///                                 --------------------------------------------
///                                 96 variations of Angle identification for each benchmark.
/// @endcode
namespace DCANG {
    static const double QS_MIN = 0.00000000000001; ///< Minimum search sigma.
    static const double QS_MAX = 0.0000001; ///< Maximum search sigma.
    static const double QS_STEP = 0.00000003; ///< Amount to increment for each test.
    
    static const double MS_MIN = 0.00000000000001; ///< Minimum match sigma.
    static const double MS_MAX = 0.000001; ///< Maximum match sigma.
    static const double MS_STEP = 0.0000003; ///< Amount to increment for each test.
    
    static const int MM_MIN = 3; ///< Minimum number of stars that define a match.
    static const int MM_MAX = 30; ///< Maximum number of stars that define a match.
    static const int MM_STEP = 5; ///< Amount to increment for each test.
    
    static const std::string TABLE_NAME = "SEP_20"; ///< Name of table generated for Angle method.
}

/// Defining characteristics of the plane identification.
///
/// @code{.cpp}
/// Current number of permutations: (0.0000001 - 0.00000000000001) / 0.00000004    // 3
///                                 (0.0000001 - 0.00000000000001) / 0.00000004    // 3
///                                 (0.000001 - 0.00000000000001) / 0.000004       // 3
///                                 (30 - 3) / 15                                  // 3
///                                 (1499 - 500) / 1000                            // 2
///                                 --------------------------------------------
///                                 162 variations of Plane identification for each benchmark.
/// @endcode
namespace DCPLA {
    static const double SA_MIN = 0.00000000000001; ///< Minimum area sigma.
    static const double SA_MAX = 0.000001; ///< Maximum area sigma.
    static const double SA_STEP = 0.0000004; ///< Amount to increment for each test.
    
    static const double SI_MIN = 0.00000000000001; ///< Minimum moment sigma.
    static const double SI_MAX = 0.000001; ///< Maximum moment sigma.
    static const double SI_STEP = 0.0000004; ///< Amount to increment for each test.
    
    static const double MS_MIN = 0.00000000000001; ///< Minimum match sigma.
    static const double MS_MAX = 0.000001; ///< Maximum match sigma.
    static const double MS_STEP = 0.0000004; ///< Amount to increment for each test.
    
    static const int MM_MIN = 3; ///< Minimum number of stars that define a match.
    static const int MM_MAX = 30; ///< Maximum number of stars that define a match.
    static const int MM_STEP = 15; ///< Amount to increment for each test.
    
    static const int BQT_MIN = 500; ///< Minimum size of the square to project the nearby-stars quad tree with.
    static const int BQT_MAX = 1499; ///< Maximum size of the square to project the nearby-stars quad tree with.
    static const int BQT_STEP = 1000; ///< Amount to increment for each test.
    static const int BQT_ITER = 1 + (int) ((BQT_MAX - BQT_MIN) / BQT_STEP); ///< Number of quad-trees generated.
    
    static const std::string TABLE_NAME = "PLANE_20"; ///< Name of table generated for PlanarTriangle method.
}

/// Defining characteristics of the sphere identification.
///
/// @code{.cpp}
/// Current number of permutations: (0.0000001 - 0.00000000000001) / 0.00000004    // 3
///                                 (0.0000001 - 0.00000000000001) / 0.00000004    // 3
///                                 (0.000001 - 0.00000000000001) / 0.000004       // 3
///                                 (30 - 3) / 15                                  // 3
///                                 (1499 - 500) / 1000                            // 2
///                                 --------------------------------------------
///                                 162 variations of Sphere identification for each benchmark.
/// @endcode
namespace DCSPH {
    static const double SA_MIN = 0.00000000000001; ///< Minimum area sigma.
    static const double SA_MAX = 0.000001; ///< Maximum area sigma.
    static const double SA_STEP = 0.0000004; ///< Amount to increment for each test.
    
    static const double SI_MIN = 0.00000000000001; ///< Minimum moment sigma.
    static const double SI_MAX = 0.000001; ///< Maximum moment sigma.
    static const double SI_STEP = 0.0000004; ///< Amount to increment for each test.
    
    static const double MS_MIN = 0.00000000000001; ///< Minimum match sigma.
    static const double MS_MAX = 0.000001; ///< Maximum match sigma.
    static const double MS_STEP = 0.0000004; ///< Amount to increment for each test.
    
    static const int MM_MIN = 3; ///< Minimum number of stars that define a match.
    static const int MM_MAX = 30; ///< Maximum number of stars that define a match.
    static const int MM_STEP = 15; ///< Amount to increment for each test.
    
    static const int BQT_MIN = 500; ///< Minimum size of the square to project the nearby-stars quad tree with.
    static const int BQT_MAX = 1499; ///< Maximum size of the square to project the nearby-stars quad tree with.
    static const int BQT_STEP = 1000; ///< Amount to increment for each test.
    static const int BQT_ITER = 1 + (int) ((BQT_MAX - BQT_MIN) / BQT_STEP); ///< Number of quad-trees generated.
    
    static const std::string TABLE_NAME = "SPHERE_20"; ///< Name of table generated for SphericalTriangle method.
    static const int TD_H_FOR_TREE = 3; ///< This MUST be the td_h used to construct the Nibble table.
}

/// Defining characteristics of the astro identification.
///
/// @code{.cpp}
/// Current number of permutations: (0.0000001 - 0.00000000000001) / 0.00000005    // 2
///                                 (0.000001 - 0.00000000000001) / 0.0000005      // 2
///                                 (1499 - 500) / 1000                            // 2
///                                 (250 - 50) / 200                               // 2
///                                 (10 - 1) / 10                                  // 2
///                                 (10 - 1) / 10                                  // 2
///                                 (10 - 1) / 10                                  // 2
///                                 (10 - 1) / 10                                  // 2
///                                 --------------------------------------------
///                                 256 variations of Astro identification for each benchmark.
/// @endcode
namespace DCAST {
    static const double QS_MIN = 0.00000000000001; ///< Minimum query sigma.
    static const double QS_MAX = 0.000001; ///< Maximum queyr sigma.
    static const double QS_STEP = 0.0000005; ///< Amount to increment for each test.
    
    static const double MS_MIN = 0.00000000000001; ///< Minimum match sigma.
    static const double MS_MAX = 0.000001; ///< Maximum match sigma.
    static const double MS_STEP = 0.0000005; ///< Amount to increment for each test.
    
    static const int BKT_MIN = 500; ///< Minimum size of the square to project the nearby-stars kd-tree with.
    static const int BKT_MAX = 1499; ///< Maximum size of the square to project the nearby-stars kd-tree with.
    static const int BKT_STEP = 1000; ///< Amount to increment for each test.
    static const int BKT_ITER = 1 + (int) ((BKT_MAX - BKT_MIN) / BKT_STEP); ///< Number of kd-trees generated.
    
    static const int KAA_MIN = 50; ///< Minimum bayes factor to accept an alignment.
    static const int KAA_MAX = 250; ///< Maximum bayes factor to accept an alignment.
    static const int KAA_STEP = 200; ///< Amount to increment for each test.
    
    static const int UT_MIN = 1; ///< The minimum utility for a tp, fp, tn, and fn.
    static const int UT_MAX = 10; ///< The maximum utlity for a tp, fp, tn, and fn.
    static const int UT_STEP = 10; ///< Amount to increment for each test.
    
    static const std::string ASTROH_NAME = "ASTRO_H20"; ///< Name of hash table generated for AstrometryNet method.
    static const std::string ASTROC_NAME = "ASTRO_C20"; ///< Name of centers table generated for AstrometryNet method.
}

/// Defining characteristics of the pyramid identification.
///
/// @code{.cpp}
///                                 (0.000001 - 0.00000000000001) / 0.0000003      // 4
///                                 (0.000001 - 0.00000000000001) / 0.0000003      // 4
///                                 --------------------------------------------
///                                 16 variations of Pyramid identification for each benchmark.
/// @endcode
namespace DCPYR {
    static const double QS_MIN = 0.00000000000001; ///< Minimum query sigma.
    static const double QS_MAX = 0.000001; ///< Maximum query sigma.
    static const double QS_STEP = 0.0000003; ///< Amount to increment for each test.
    
    static const double MS_MIN = 0.00000000000001; ///< Minimum match sigma.
    static const double MS_MAX = 0.000001; ///< Maximum match sigma.
    static const double MS_STEP = 0.0000003; ///< Amount to increment for each test.
    
    static const std::string TABLE_NAME = "PYRA_20"; ///< Name of table generated for Pyramid method.
}