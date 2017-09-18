/// @file trial-plane.cpp
/// @author Glenn Galvizo
///
/// Source file for the planar-triangle trials. This parses the benchmarks in Nibble, and logs the data into a CSV file.

#include <chrono>
#include "identification/planar-triangle.h"

/// Defining characteristics of the plane identification.
///
/// @code{.cpp}
/// Current number of permutations: (0.0000001 - 0.00000000000001) / 0.00000004    // 3
///                                 (0.0000001 - 0.00000000000001) / 0.00000004    // 3
///                                 (0.000001 - 0.00000000000001) / 0.000004       // 3
///                                 (30 - 3) / 7                                   // 4
///                                 (1500 - 500) / 1000                            // 2
///                                 --------------------------------------------
///                                 216 variations of Plane identification for each benchmark.
/// @endcode
namespace DCPI {
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
    static const int MM_STEP = 7; ///< Amount to increment for each test.
    
    static const int BQT_MIN = 500; ///< Minimum size of the square to project the nearby-stars quad tree with.
    static const int BQT_MAX = 1500; ///< Maximum size of the square to project the nearby-stars quad tree with.
    static const int BQT_STEP = 1000; ///< Amount to increment for each test.
    
    static const std::string TABLE_NAME = "PLANE_20"; ///< Name of table generated for PlanarTriangle method.
}

/// Wrap three dimensions of testing (area sigma, moment sigma, and match sigma) in a small function. Passed in the
/// working benchmark, the match minimum, and the quadtree w.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Working benchmark set id.
/// @param match_minimum Working match minimum for this set of trials.
/// @param bsc5_quadtree_w Working quadtree width for this set of trials.
/// @param q_root Working quad-tree root. Generated with every iteration of bsc5_quadtree_w.
void trial_as_ms_ms (Nibble &nb, std::ofstream &log, const unsigned int set_n, const unsigned int match_minimum,
                     const unsigned int bsc5_quadtree_w, std::shared_ptr<QuadNode> &q_root) {
    Plane::Parameters p;
    Star::list s;
    double fov;
    
    for (double sigma_a = DCPI::SA_MIN; sigma_a <= DCPI::SA_MAX; sigma_a += DCPI::SA_STEP) {
        for (double sigma_i = DCPI::SI_MIN; sigma_i <= DCPI::SI_MAX; sigma_i += DCPI::SI_STEP) {
            for (double match_sigma = DCPI::MS_MIN; match_sigma <= DCPI::MS_MAX; match_sigma += DCPI::MS_STEP) {
                p.sigma_a = sigma_a, p.sigma_i = sigma_i, p.match_sigma = match_sigma;
                p.match_minimum = match_minimum, p.bsc5_quadtree_w = bsc5_quadtree_w, p.table_name = DCPI::TABLE_NAME;
                
                // Read the benchmark, copy the list here.
                Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
                input.present_image(s, fov);
                
                // Identify the image, record the number of actual matches that exist.
                Star::list results = Plane::identify(input, p);
                int matches_found = Benchmark::compare_stars(input, results);
                
                log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
                log << sigma_a << "," << sigma_i << "," << match_sigma << "," << match_minimum << ",";
                log << bsc5_quadtree_w << std::endl;
            }
        }
    }
}

/// Wrap one dimensions of testing (match minimum) in a small function. Passed in the working benchmark and the
/// quadtree, and calls trial_as_ms_ms to test the other dimensions.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Working benchmark set id.
/// @param bsc5_quadtree_w Working quadtree width for this set of trials.
/// @param q_root Working quad-tree root. Generated with every iteration of bsc5_quadtree_w.
void trial_mm_et (Nibble &nb, std::ofstream &log, const unsigned int set_n, const unsigned int bsc5_quadtree_w,
                  std::shared_ptr<QuadNode> &q_root) {
    for (unsigned int match_minimum = DCPI::MM_MIN; match_minimum <= DCPI::MM_MAX; match_minimum += DCPI::MM_STEP) {
        trial_as_ms_ms(nb, log, set_n, match_minimum, bsc5_quadtree_w, q_root);
    }
}

/// Test each benchmark with varying Plane operating parameters.
///
/// @return 0 when finished.
int main () {
    std::ostringstream l;
    std::ofstream log;
    Nibble nb(Benchmark::TABLE_NAME, "set_n");
    
    /// Alias for the clock in the Chrono library.
    using clock = std::chrono::high_resolution_clock;
    
    // Construct the log file based on the HOKU_PROJECT_PATH environment variable.
    l << "/data/logs/trial/plane-" << clock::to_time_t(clock::now() - std::chrono::hours(24)) << ".csv";
    log.open(std::string(std::getenv("HOKU_PROJECT_PATH")) + l.str());
    
    // Make sure the log file is open before proceeding.
    if (!log.is_open()) {
        throw "Log file cannot be opened.";
    }
    
    // Set the attributes of the log.
    log << "SetNumber,InputSize,IdentificationSize,MatchesFound,SigmaA,SigmaI,MatchSigma,MatchMinimum";
    log << ",QuadtreeW" << std::endl;
    
    // Run the trials!
    nb.select_table(Benchmark::TABLE_NAME);
    const unsigned int BENCH_SIZE = (unsigned int) nb.search_table("MAX(set_n)", 1, 1)[0];
    for (unsigned int quadtree_w = DCPI::BQT_MIN; quadtree_w <= DCPI::BQT_MAX; quadtree_w += DCPI::BQT_STEP) {
        
        // Build the quadtree for the given W- as early as possible to avoid constant rebuilding.
        std::shared_ptr<QuadNode> q_root = std::make_shared<QuadNode>(QuadNode::load_tree(quadtree_w));
        for (unsigned int set_n = 0; set_n < BENCH_SIZE; set_n++) {
            std::cout << "\r" << "Current *Set* Number: " << set_n;
    
            trial_mm_et(nb, log, set_n, quadtree_w, q_root);
        }
    }
    
    return 0;
}
