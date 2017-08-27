/// @file trial-angle.cpp
/// @author Glenn Galvizo
///
/// Source file for the angle trials. This parses the benchmarks in Nibble, and logs the data into a CSV file.

#include <chrono>
#include "angle.h"

/// Defining characteristics of the angle identification.
/// @code{.cpp}
/// Current number of permutations: (0.0000001 - 0.00000000000001) / 0.00000001    // ~10
///                                 (20 - 1) / 3                                   // ~7
///                                 (0.000001 - 0.00000000000001) / 0.0000001      // ~10
///                                 (10 - 3)                                       // 7
///                                 --------------------------------------------
///                                 4900 variations of Angle identification for each benchmark.
/// @endcode
namespace DCAI {
    static const double QS_MIN = 0.00000000000001; ///< Minimum search sigma.
    static const double QS_MAX = 0.0000001; ///< Maximum search sigma.
    static const double QS_STEP = 0.00000001; ///< Amount to increment for each test.
    
    static const int QL_MIN = 1; ///< Minimum number of results to limit search by.
    static const int QL_MAX = 20; ///< Maximum number of results to limit search by.
    static const int QL_STEP = 3; ///< Amount to increment for each test.
    
    static const double MS_MIN = 0.00000000000001; ///< Minimum match sigma.
    static const double MS_MAX = 0.000001; ///< Maximum match sigma.
    static const double MS_STEP = 0.0000001; ///< Amount to increment for each test.
    
    static const int MM_MIN = 3; ///< Minimum number of stars that define a match.
    static const int MM_MAX = 10; ///< Maximum number of stars that define a match.
}

/// Wrap three dimensions of testing (match sigma, query limit, and match minimum) in a small function. Passed in the
/// working benchmark and the query sigma.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Working benchmark set id.
/// @param query_sigma Working sigma to query Nibble with.
void trial_ms_sl_mm (Nibble &nb, std::ofstream &log, const unsigned int set_n, const double query_sigma) {
    for (double match_sigma = DCAI::MS_MIN; match_sigma <= DCAI::MS_MAX; match_sigma += DCAI::MS_STEP) {
        for (int query_limit = DCAI::QL_MIN; query_limit <= DCAI::QL_MAX; query_limit += DCAI::QL_STEP) {
            for (int match_minimum = DCAI::MM_MIN; match_minimum <= DCAI::MM_MAX; match_minimum++) {
                Angle::Parameters p;
                p.query_limit = (unsigned) match_sigma, p.match_minimum = (unsigned) match_minimum;
                p.query_sigma = query_sigma, p.match_sigma = match_sigma;
                
                // Read the benchmark, identify the stars, and log the matches found.
                Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
                Star::list results = Angle::identify(input, p);
                int matches_found = Benchmark::compare_stars(input, results);
                
                log << set_n << "," << results.size() << "," << matches_found << std::endl;
            }
        }
    }
}

/// Test each benchmark with varying Angle operating parameters.
///
/// @return -1 if the log file cannot be opened. 0 otherwise.
int main () {
    std::ostringstream l;
    std::ofstream log;
    Nibble nb;
    
    /// Alias for the clock in the Chrono library.
    using clock = std::chrono::high_resolution_clock;
    
    // Construct the log file based on the HOKU_PROJECT_PATH environment variable.
    l << "/data/logs/trial/angle-" << clock::to_time_t(clock::now() - std::chrono::hours(24)) << ".csv";
    log.open(std::string(std::getenv("HOKU_PROJECT_PATH")) + l.str());
    
    // Make sure the log file is open before proceeding.
    if (!log.is_open()) {
        return -1;
    }
    
    // Set the attributes of the log.
    log << "SetNumber,IdentificationSize,MatchesFound" << std::endl;
    
    // Run the trials! All five dimensions! ¯\_(ツ)_/¯
    const unsigned int BENCH_SIZE = (unsigned int) nb.search_table("MAX(set_n)", 1, 1)[0];
    for (unsigned int set_n = 0; set_n < BENCH_SIZE; set_n++) {
        for (double query_sigma = DCAI::QS_MIN; query_sigma <= DCAI::QS_MAX; query_sigma += DCAI::QS_STEP) {
            trial_ms_sl_mm(nb, log, set_n, query_sigma);
        }
    }
    
    return 0;
}
