/// @file trial-astro.cpp
/// @author Glenn Galvizo
///
/// Source file for the astrometry-net trials. This parses the benchmarks in Nibble, and logs the data into a 
/// CSV file.

#include <chrono>
#include "identification/astrometry-net.h"

/// Defining characteristics of the astro identification.
///
/// @code{.cpp}
/// Current number of permutations: (0.0000001 - 0.00000000000001) / 0.00000003    // 4
///                                 (0.000001 - 0.00000000000001) / 0.0000003      // 4
///                                 (1499 - 500) / 500                             // 3
///                                 (250 - 50) / 100                               // 3
///                                 (10 - 1) / 10                                  // 2
///                                 (10 - 1) / 10                                  // 2
///                                 (10 - 1) / 10                                  // 2
///                                 (10 - 1) / 10                                  // 2
///                                 --------------------------------------------
///                                 2304 variations of Astro identification for each benchmark.
/// @endcode
namespace DCANI {
    static const double QS_MIN = 0.00000000000001; ///< Minimum query sigma.
    static const double QS_MAX = 0.000001; ///< Maximum queyr sigma.
    static const double QS_STEP = 0.0000001; ///< Amount to increment for each test.
    
    static const double MS_MIN = 0.00000000000001; ///< Minimum match sigma.
    static const double MS_MAX = 0.000001; ///< Maximum match sigma.
    static const double MS_STEP = 0.0000001; ///< Amount to increment for each test.
    
    static const int BKT_MIN = 500; ///< Minimum size of the square to project the nearby-stars kd-tree with.
    static const int BKT_MAX = 1499; ///< Maximum size of the square to project the nearby-stars kd-tree with.
    static const int BKT_STEP = 500; ///< Amount to increment for each test.
    
    static const int KAA_MIN = 50; ///< Minimum bayes factor to accept an alignment.
    static const int KAA_MAX = 250; ///< Maximum bayes factor to accept an alignment.
    static const int KAA_STEP = 100; ///< Amount to increment for each test.
    
    static const int UT_MIN = 1; ///< The minimum utility for a tp, fp, tn, and fn.
    static const int UT_MAX = 10; ///< The maximum utlity for a tp, fp, tn, and fn.
    static const int UT_STEP = 10; ///< Amount to increment for each test.
}

/// Wrap three dimensions of testing (area sigma, moment sigma, and match sigma) in a small function. Passed in the
/// working benchmark, the match minimum, and the quadtree w.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Working benchmark set id.
/// @param ut Working utility values in order: u_tp, u_tn, u_fp, and u_fn.
/// @param k_accept_alignment Working bayes factor for this set of trials.
/// @param kd_tree_w Working KD-tree projection width for this set of trials.
void trial_qs_ms (Nibble &nb, std::ofstream &log, const unsigned int set_n, const std::array<int, 4> &ut,
                  const int k_accept_alignment, const int kd_tree_w) {
    Astro::Parameters p;
    Star::list s;
    double fov;
    
    for (double query_sigma = DCANI::QS_MIN; query_sigma <= DCANI::QS_MAX; query_sigma += DCANI::QS_STEP) {
        for (double match_sigma = DCANI::MS_MIN; match_sigma <= DCANI::MS_MAX; match_sigma += DCANI::MS_STEP) {
            p.query_sigma = query_sigma, p.match_sigma = match_sigma, p.k_alignment_accept = k_accept_alignment;
            p.kd_tree_w = kd_tree_w; p.u_tp = ut[0], p.u_tn = ut[1], p.u_fp = ut[2], p.u_fn = ut[3];
            
            // Read the benchmark, copy the list here.
            Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
            input.present_image(s, fov);
            
            // Identify the image, record the number of actual matches that exist.
            Star::list results = Astro::identify(input, p);
            int matches_found = Benchmark::compare_stars(input, results);
    
            log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
            log << query_sigma << "," << match_sigma << "," << kd_tree_w << "," << k_accept_alignment << ",";
            log << ut[3] << "," << ut[2] << "," << ut[1] << "," << ut[0] << std::endl;
        }
    }
}

/// Wrap two dimensions of testing (bayes factor and kd-tree w) in a small function. Passed in the working benchmark
/// and utility points. Calls trial_qs_ms to test the other dimensions.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Working benchmark set id.
/// @param ut Working utility values in order: u_tp, u_tn, u_fp, and u_fn.
void trial_ka_kw_et (Nibble &nb, std::ofstream &log, const unsigned int set_n, const std::array<int, 4> &ut) {
    for (int kaa = DCANI::KAA_MIN; kaa <= DCANI::KAA_MAX; kaa += DCANI::KAA_STEP) {
        for (int kd_tree_w = DCANI::BKT_MIN; kd_tree_w <= DCANI::BKT_MAX; kd_tree_w += DCANI::BKT_STEP) {
            trial_qs_ms(nb, log, set_n, ut, kaa, kd_tree_w);
        }
    }
}

/// Wrap four dimensions of testing (u_tp, u_fp, u_tn, and u_fn) in a small function. Passed in the working
/// benchmark, and calls trial_ka_kw_et to test the other dimensions.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Working benchmark set id.
void trial_tp_fp_tn_fn_et (Nibble &nb, std::ofstream &log, const unsigned int set_n) {
    for (int u_tp = DCANI::UT_MIN; u_tp <= DCANI::UT_MAX; u_tp += DCANI::UT_STEP) {
        for (int u_tn = DCANI::UT_MIN; u_tn <= DCANI::UT_MAX; u_tn += DCANI::UT_STEP) {
            for (int u_fp = DCANI::UT_MIN; u_fp <= DCANI::UT_MAX; u_fp += DCANI::UT_STEP) {
                for (int u_fn = DCANI::UT_MIN; u_fn <= DCANI::UT_MAX; u_fn += DCANI::UT_STEP) {
                    trial_ka_kw_et(nb, log, set_n, {u_tp, u_tn, u_fp, u_fn});
                }
            }
        }
    }
}

/// Test each benchmark with varying Astro operating parameters.
///
/// @return 0 when finished.
int main () {
    std::ostringstream l;
    std::ofstream log;
    Nibble nb(Benchmark::TABLE_NAME, "set_n");
    
    /// Alias for the clock in the Chrono library.
    using clock = std::chrono::high_resolution_clock;
    
    // Construct the log file based on the HOKU_PROJECT_PATH environment variable.
    l << "/data/logs/trial/astro-" << clock::to_time_t(clock::now() - std::chrono::hours(24)) << ".csv";
    log.open(std::string(std::getenv("HOKU_PROJECT_PATH")) + l.str());
    
    // Make sure the log file is open before proceeding.
    if (!log.is_open()) {
        throw "Log file cannot be opened.";
    }
    
    // Set the attributes of the log.
    log << "SetNumber,InpputSize,IdentificationSize,MatchesFound,QuerySigma,MatchSigma,KdTreeW,KAcceptAlignment";
    log << ",UtilityFalseNegative,UtilityFalsePositive,UtilityTrueNegative,UtilityTruePositive" << std::endl;
    
    // Run the trials!
    nb.select_table(Benchmark::TABLE_NAME);
    const unsigned int BENCH_SIZE = (unsigned int) nb.search_table("MAX(set_n)", 1, 1)[0];
    for (unsigned int set_n = 0; set_n < BENCH_SIZE; set_n++) {
        trial_tp_fp_tn_fn_et(nb, log, set_n);
    }
    
    return 0;
}
