/// @file character.h
/// @author Glenn Galvizo
///
/// Header file for the trials. This holds all of the namespaces that define identification working characteristics.

/// Defining characteristics of the angle identification.
///
/// @code{.cpp}
/// Dimension Sets:                 query_sigma exists in [epsilon, ..., epsilon + 0.00000003 * 9]
///                                 match_sigma exists in [epsilon, ..., epsilon + 0.00000003 * 9]
///                                 match_minimum exists in [1, 3, 5, ..., 41]
///
/// Current number of permutations: 10 * 10 * 40 = 4000 variations of Angle identification.
/// @endcode
namespace DCANG {
    static const double QS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum search sigma.
    static const double QS_STEP = 0.00000003; ///< Amount to increment for each test.
    static const int QS_ITER = 10; ///< Cardinality of search sigma set.
    
    static const double MS_MIN =  std::numeric_limits<double>::epsilon(); ///< Minimum match sigma.
    static const double MS_STEP = 0.00000003; ///< Amount to increment for each test.
    static const int MS_ITER = 10; ///< Cardinality of match sigma set.
    
    static const int MM_MIN = 1; ///< Minimum number of stars that define a match.
    static const int MM_STEP = 2; ///< Amount to increment for each test.
    static const int MM_ITER = 20; ///< Cardinality of match minimum set.
    
    static const std::string TABLE_NAME = "SEP_20"; ///< Name of table generated for Angle method.
}

/// Defining characteristics of the plane identification.
///
/// @code{.cpp}
/// Dimension Sets:                 sigma_area exists in [epsilon, ..., epsilon + 0.00000003 * 9]
///                                 sigma_moment exists in [epsilon, ..., epsilon + 0.00000003 * 9]
///                                 match_sigma exists in [epsilon, ..., epsilon + 0.00000003 * 9]
///                                 match_minimum exists in [1, 3, 5, ..., 41]
///                                 bsc5_quadtree_width exists in [500, 1500, 2500, 3500]
///
/// Current number of permutations: 10 * 10 * 10 * 20 * 4 = 80000 variations of Plane identification.
/// @endcode
namespace DCPLA {
    static const double SA_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum area sigma.
    static const double SA_STEP = 0.00000003; ///< Amount to increment for each test.
    static const int SA_ITER = 10; ///< Cardinality of area sigma set.
    
    static const double SI_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum moment sigma.
    static const double SI_STEP = 0.00000003; ///< Amount to increment for each test.
    static const int SI_ITER = 10; ///< Cardinality of moment sigma set.
    
    static const double MS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum match sigma.
    static const double MS_STEP = 0.00000003; ///< Amount to increment for each test.
    static const int MS_ITER = 10; ///< Cardinality of match sigma set.
    
    static const int MM_MIN = 1; ///< Minimum number of stars that define a match.
    static const int MM_STEP = 2; ///< Amount to increment for each test.
    static const int MM_ITER = 20; ///< Cardinality of match minimum set.
    
    static const int BQT_MIN = 500; ///< Minimum size of the square to project the nearby-stars quad tree with.
    static const int BQT_STEP = 1000; ///< Amount to increment for each test.
    static const int BQT_ITER = 4; ///< Cardinality of quad-tree size set.
    
    static const std::string TABLE_NAME = "PLANE_20"; ///< Name of table generated for PlanarTriangle method.
}

/// Defining characteristics of the sphere identification.
///
/// @code{.cpp}
/// Dimension Sets:                 sigma_area exists in [epsilon, ..., epsilon + 0.00000003 * 9]
///                                 sigma_moment exists in [epsilon, ..., epsilon + 0.00000003 * 9]
///                                 match_sigma exists in [epsilon, ..., epsilon + 0.00000003 * 9]
///                                 match_minimum exists in [1, 3, 5, ..., 41]
///                                 bsc5_quadtree_width exists in [500, 1500, 2500, 3500]
///
/// Current number of permutations: 10 * 10 * 10 * 20 * 4 = 80000 variations of Sphere identification.
/// @endcode
namespace DCSPH {
    static const double SA_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum area sigma.
    static const double SA_STEP = 0.00000003; ///< Amount to increment for each test.
    static const int SA_ITER = 10; ///< Cardinality of area sigma set.
    
    static const double SI_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum moment sigma.
    static const double SI_STEP = 0.00000003; ///< Amount to increment for each test.
    static const int SI_ITER = 10; ///< Cardinality of moment sigma set.
    
    static const double MS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum match sigma.
    static const double MS_STEP = 0.00000003; ///< Amount to increment for each test.
    static const int MS_ITER = 10; ///< Cardinality of match sigma set.
    
    static const int MM_MIN = 1; ///< Minimum number of stars that define a match.
    static const int MM_STEP = 2; ///< Amount to increment for each test.
    static const int MM_ITER = 20; ///< Cardinality of match minimum set.
    
    static const int BQT_MIN = 500; ///< Minimum size of the square to project the nearby-stars quad tree with.
    static const int BQT_STEP = 1000; ///< Amount to increment for each test.
    static const int BQT_ITER = 4; ///< Cardinality of quad-tree size set.
    
    static const std::string TABLE_NAME = "PLANE_20"; ///< Name of table generated for SphericalTriangle method.
    static const int TD_H_FOR_TREE = 3; ///< This MUST be the td_h used to construct the Nibble table.
}

/// Defining characteristics of the astro identification.
///
/// @code{.cpp}
/// Dimension Sets:                 query_sigma exists in [epsilon, ..., epsilon + 0.00000003 * 9]
///                                 match_sigma exists in [epsilon, ..., epsilon + 0.00000003 * 9]
///                                 kd_tree_width exists in [500, 1500, 2500, 3500]
///                                 k_accept_alignment exists in [50, 100, 150, ..., 500]
///                                 all utility points exists in [1, 6, ..., 26]
///
/// Current number of permutations: 2^8 = 256 variations of Astro identification.
/// @endcode
namespace DCAST {
    static const double QS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum query sigma.
    static const double QS_STEP = 0.00000003; ///< Amount to increment for each test.
    static const int QS_ITER = 10; ///< Cardinality of query sigma set.
    
    static const double MS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum match sigma.
    static const double MS_STEP = 0.00000003; ///< Amount to increment for each test.
    static const int MS_ITER = 10; ///< Cardinality of match sigma set.
    
    static const int BKT_MIN = 500; ///< Minimum size of the square to project the nearby-stars kd-tree with.
    static const int BKT_STEP = 1000; ///< Amount to increment for each test.
    static const int BKT_ITER = 4; ///< Cardinality of kd-tree size set.
    
    static const int KAA_MIN = 50; ///< Minimum bayes factor to accept an alignment.
    static const int KAA_STEP = 50; ///< Amount to increment for each test.
    static const int KAA_ITER = 10; ///< Cardinality of bayes factor set.
    
    static const int UT_MIN = 1; ///< The minimum utility for a tp, fp, tn, and fn.
    static const int UT_STEP = 5; ///< Amount to increment for each test.
    static const int UT_ITER = 5; ///< Cardinality of each utility point set.
    
    static const std::string ASTROH_NAME = "ASTRO_H20"; ///< Name of hash table generated for AstrometryNet method.
    static const std::string ASTROC_NAME = "ASTRO_C20"; ///< Name of centers table generated for AstrometryNet method.
}

/// Defining characteristics of the pyramid identification.
///
/// @code{.cpp}
/// Dimension Sets:                 query_sigma exists in [epsilon, ..., epsilon + 0.0000003 * 9]
///                                 match_sigma exists in [epsilon, ..., epsilon + 0.0000003 * 9]
///
/// Current number of permutations: 10 * 10 = 100 variations of Pyramid identification.
/// @endcode
namespace DCPYR {
    static const double QS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum query sigma.
    static const double QS_STEP = 0.0000003; ///< Amount to increment for each test.
    static const int QS_ITER = 10; ///< Cardinality of query sigma set.
    
    static const double MS_MIN = std::numeric_limits<double>::epsilon(); ///< Minimum match sigma.
    static const double MS_STEP = 0.0000003; ///< Amount to increment for each test.
    static const int MS_ITER = 10; ///< Cardinality of match sigma set.
    
    static const std::string TABLE_NAME = "PYRA_20"; ///< Name of table generated for Pyramid method.
}