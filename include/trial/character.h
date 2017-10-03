/// @file character.h
/// @author Glenn Galvizo
///
/// Header file for the trials. This holds all of the namespaces that define identification working characteristics.

/// Defining characteristics of the angle identification.
///
/// @code{.cpp}
/// Dimension Sets:                 query_sigma exists in [1e-14, 3.000001e-08, 6.000001e-08, 9.000001e-08]
///                                 match_sigma exists in [1e-14, 3.000001e-08, 6.000001e-08, 9.000001e-08]
///                                 match_minimum exists in [3, 8, 13, 18, 23, 27]
///
/// Current number of permutations: 4 * 4 * 6 = 96 variations of Angle identification for each benchmark.
/// @endcode
namespace DCANG {
    static const double QS_MIN = 1e-14; ///< Minimum search sigma.
    static const double QS_STEP = 0.00000003; ///< Amount to increment for each test.
    static const int QS_ITER = 4; ///< Cardinality of search sigma set.
    
    static const double MS_MIN = 1e-14; ///< Minimum match sigma.
    static const double MS_STEP = 0.00000003; ///< Amount to increment for each test.
    static const int MS_ITER = 4; ///< Cardinality of match sigma set.
    
    static const int MM_MIN = 3; ///< Minimum number of stars that define a match.
    static const int MM_STEP = 5; ///< Amount to increment for each test.
    static const int MM_ITER = 6; ///< Cardinality of match minimum set.
    
    static const std::string TABLE_NAME = "SEP_20"; ///< Name of table generated for Angle method.
}

/// Defining characteristics of the plane identification.
///
/// @code{.cpp}
/// Dimension Sets:                  sigma_area exists in [1e-14, 4.0000000999999997e-07, 8.0000001e-07]
///                                  sigma_moment exists in [1e-14, 4.0000000999999997e-07, 8.0000001e-07]
///                                  match_sigma exists in [1e-14, 4.0000000999999997e-07, 8.0000001e-07]
///                                  match_minimum exists in [3, 18, 33]
///                                  bsc5_quadtree_width exists in [500, 1500]
///
/// Current number of permutations: 3 * 3 * 3 * 3 * 2 = 162 variations of Plane identification for each benchmark.
/// @endcode
namespace DCPLA {
    static const double SA_MIN = 1e-14; ///< Minimum area sigma.
    static const double SA_STEP = 0.0000004; ///< Amount to increment for each test.
    static const int SA_ITER = 3; ///< Cardinality of area sigma set.
    
    static const double SI_MIN = 1e-14; ///< Minimum moment sigma.
    static const double SI_STEP = 0.0000004; ///< Amount to increment for each test.
    static const int SI_ITER = 3; ///< Cardinality of moment sigma set.
    
    static const double MS_MIN = 1e-14; ///< Minimum match sigma.
    static const double MS_STEP = 0.0000004; ///< Amount to increment for each test.
    static const int MS_ITER = 3; ///< Cardinality of match sigma set.
    
    static const int MM_MIN = 3; ///< Minimum number of stars that define a match.
    static const int MM_STEP = 15; ///< Amount to increment for each test.
    static const int MM_ITER = 3; ///< Cardinality of match minimum set.
    
    static const int BQT_MIN = 500; ///< Minimum size of the square to project the nearby-stars quad tree with.
    static const int BQT_STEP = 1000; ///< Amount to increment for each test.
    static const int BQT_ITER = 2; ///< Cardinality of quad-tree size set.
    
    static const std::string TABLE_NAME = "PLANE_20"; ///< Name of table generated for PlanarTriangle method.
}

/// Defining characteristics of the sphere identification.
///
/// @code{.cpp}
/// Dimension Sets:                  sigma_area exists in [1e-14, 4.0000000999999997e-07, 8.0000001e-07]
///                                  sigma_moment exists in [1e-14, 4.0000000999999997e-07, 8.0000001e-07]
///                                  match_sigma exists in [1e-14, 4.0000000999999997e-07, 8.0000001e-07]
///                                  match_minimum exists in [3, 18, 33]
///                                  bsc5_quadtree_width exists in [500, 1500]
///
/// Current number of permutations: 3 * 3 * 3 * 3 * 2 = 162 variations of Sphere identification for each benchmark.
/// @endcode
namespace DCSPH {
    static const double SA_MIN = 1e-14; ///< Minimum area sigma.
    static const double SA_STEP = 0.0000004; ///< Amount to increment for each test.
    static const int SA_ITER = 3; ///< Cardinality of area sigma set.
    
    static const double SI_MIN = 1e-14; ///< Minimum moment sigma.
    static const double SI_STEP = 0.0000004; ///< Amount to increment for each test.
    static const int SI_ITER = 3; ///< Cardinality of moment sigma set.
    
    static const double MS_MIN = 1e-14; ///< Minimum match sigma.
    static const double MS_STEP = 0.0000004; ///< Amount to increment for each test.
    static const int MS_ITER = 3; ///< Cardinality of match sigma set.
    
    static const int MM_MIN = 3; ///< Minimum number of stars that define a match.
    static const int MM_STEP = 15; ///< Amount to increment for each test.
    static const int MM_ITER = 3; ///< Cardinality of match minimum set.
    
    static const int BQT_MIN = 500; ///< Minimum size of the square to project the nearby-stars quad tree with.
    static const int BQT_STEP = 1000; ///< Amount to increment for each test.
    static const int BQT_ITER = 2; ///< Cardinality of quad-tree size set.
    
    static const std::string TABLE_NAME = "PLANE_20"; ///< Name of table generated for SphericalTriangle method.
    static const int TD_H_FOR_TREE = 3; ///< This MUST be the td_h used to construct the Nibble table.
}

/// Defining characteristics of the astro identification.
///
/// @code{.cpp}
/// Dimension Sets:                 query_sigma exists in [1e-14, 5.0000001e-07]
///                                 match_sigma exists in [1e-14, 5.0000001e-07]
///                                 kd_tree_width exists in [500, 1500]
///                                 k_accept_alignment exists in [50, 250]
///                                 all utility points exists in [1, 10]
///
/// Current number of permutations: 2^8 = 256 variations of Astro identification for each benchmark.
/// @endcode
namespace DCAST {
    static const double QS_MIN = 1e-14; ///< Minimum query sigma.
    static const double QS_STEP = 0.0000006; ///< Amount to increment for each test.
    static const int QS_ITER = 2; ///< Cardinality of query sigma set.
    
    static const double MS_MIN = 1e-14; ///< Minimum match sigma.
    static const double MS_STEP = 0.0000006; ///< Amount to increment for each test.
    static const int MS_ITER = 2; ///< Cardinality of match sigma set.
    
    static const int BKT_MIN = 500; ///< Minimum size of the square to project the nearby-stars kd-tree with.
    static const int BKT_STEP = 1000; ///< Amount to increment for each test.
    static const int BKT_ITER = 2; ///< Cardinality of kd-tree size set.
    
    static const int KAA_MIN = 50; ///< Minimum bayes factor to accept an alignment.
    static const int KAA_STEP = 200; ///< Amount to increment for each test.
    static const int KAA_ITER = 2; ///< Cardinality of bayes factor set.
    
    static const int UT_MIN = 1; ///< The minimum utility for a tp, fp, tn, and fn.
    static const int UT_STEP = 9; ///< Amount to increment for each test.
    static const int UT_ITER = 2; ///< Cardinality of each utility point set.
    
    static const std::string ASTROH_NAME = "ASTRO_H20"; ///< Name of hash table generated for AstrometryNet method.
    static const std::string ASTROC_NAME = "ASTRO_C20"; ///< Name of centers table generated for AstrometryNet method.
}

/// Defining characteristics of the pyramid identification.
///
/// @code{.cpp}
/// Dimension Sets:                 query_sigma exists in [1e-14, 3.0000001e-07, 6.0000001e-07, 9.0000001e-07]
///                                 match_sigma exists in [1e-14, 3.0000001e-07, 6.0000001e-07, 9.0000001e-07]
///
/// Current number of permutations: 4 * 4 16 variations of Pyramid identification for each benchmark.
/// @endcode
namespace DCPYR {
    static const double QS_MIN = 1e-14; ///< Minimum query sigma.
    static const double QS_STEP = 0.0000003; ///< Amount to increment for each test.
    static const int QS_ITER = 4; ///< Cardinality of query sigma set.
    
    static const double MS_MIN = 1e-14; ///< Minimum match sigma.
    static const double MS_STEP = 0.0000003; ///< Amount to increment for each test.
    static const int MS_ITER = 4; ///< Cardinality of match sigma set.
    
    static const std::string TABLE_NAME = "PYRA_20"; ///< Name of table generated for Pyramid method.
}