/// @file trial-pyramid.cpp
/// @author Glenn Galvizo
///
/// Source file for the pyramid trials. This parses the benchmarks in Nibble, and logs the data into a CSV file.

#include <chrono>
#include "identification/pyramid.h"

/// Defining characteristics of the pyramid identification.
///
/// @code{.cpp}
///                                 (30 - 1) / 10                                  // 3
///                                 (0.000001 - 0.00000000000001) / 0.0000003      // 4
///                                 (30 - 3) / 5                                   // 6
///                                 --------------------------------------------
///                                 72 variations of Pyramid identification for each benchmark.
/// @endcode
namespace DCPI {
    static const double QS_MIN = 0.00000000000001; ///< Minimum query sigma.
    static const double QS_MAX = 0.000001; ///< Maximum query sigma.
    static const double QS_STEP = 0.0000001; ///< Amount to increment for each test.
    
    static const int QL_MIN = 1; ///< Minimum number of results to limit search by.
    static const int QL_MAX = 20; ///< Maximum number of results to limit search by.
    static const int QL_STEP = 3; ///< Amount to increment for each test.
    
    static const double MS_MIN = 0.00000000000001; ///< Minimum match sigma.
    static const double MS_MAX = 0.000001; ///< Maximum match sigma.
    static const double MS_STEP = 0.0000001; ///< Amount to increment for each test.
}

/// Wrap three dimensions of testing (query sigma, query limit, and match sigma) in a small function. Passed in the 
/// working benchmark. 
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Working benchmark set id.
void trial_qs_ql_ms (Nibble &nb, std::ofstream &log, const unsigned int set_n) {
    Pyramid::Parameters p;
    Star::list s;
    double fov;
    
    for (double query_sigma = DCPI::QS_MIN; query_sigma <= DCPI::QS_MAX; query_sigma += DCPI::QS_STEP) {
        for (int query_limit = DCPI::QL_MIN; query_limit <= DCPI::QL_MAX; query_limit += DCPI::QL_STEP) {
            for (double match_sigma = DCPI::MS_MIN; match_sigma <= DCPI::MS_MAX; match_sigma += DCPI::MS_STEP) {
                p.query_sigma = query_sigma, p.query_limit = (unsigned) query_limit, p.match_sigma = match_sigma;
    
                // Read the benchmark, copy the list here.
                Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
                input.present_image(s, fov);
    
                // Identify the image, record the number of actual matches that exist.
                Star::list results = Pyramid::identify(input, p);
                int matches_found = Benchmark::compare_stars(input, results);
    
                log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
                log << query_sigma << "," << query_limit << "," << match_sigma << std::endl;
            }
        }
    }
}

/// Test each benchmark with varying Pyramid operating parameters.
///
/// @return 0 when finished.
int main () {
    std::ostringstream l;
    std::ofstream log;
    Nibble nb;
    
    /// Alias for the clock in the Chrono library.
    using clock = std::chrono::high_resolution_clock;
    
    // Construct the log file based on the HOKU_PROJECT_PATH environment variable.
    l << "/data/logs/trial/pyramid-" << clock::to_time_t(clock::now() - std::chrono::hours(24)) << ".csv";
    log.open(std::string(std::getenv("HOKU_PROJECT_PATH")) + l.str());
    
    // Make sure the log file is open before proceeding.
    if (!log.is_open()) {
        throw "Log file cannot be opened.";
    }
    
    // Set the attributes of the log.
    log << "SetNumber,InputSize,IdentificationSize,MatchesFound,QuerySigma,QueryLimit,MatchSigma" << std::endl;
    
    // Run the trials!
    nb.select_table(Benchmark::TABLE_NAME);
    const unsigned int BENCH_SIZE = (unsigned int) nb.search_table("MAX(set_n)", 1, 1)[0];
    for (unsigned int set_n = 0; set_n < BENCH_SIZE; set_n++) {
        trial_qs_ql_ms(nb, log, set_n);
    }
    
    return 0;
}
