/// @file run-trial.cpp
/// @author Glenn Galvizo
///
/// Source file for the trial runner. This parses the benchmarks in Nibble, and logs the results of various
/// identification methods into a CSV file. There exists two arguments (last optional), where the user decides which
/// identification method to run for the trial, and which benchmark to start from.
///
/// @code{.cpp}
/// - 0 -> Run the trials with the Angle method.
/// - 1 -> Run the trials with the AstrometryNet method.
/// - 2 -> Run the trials with the SphericalTriangle method.
/// - 3 -> Run the trials with the PlanarTriangle method.
/// - 4 -> Run the trials with the Pyramid method.
///
/// - x [0->MAX(set_n)] -> Start the trial with the given set_n (Benchmark) number.
/// @endcode
/// @example
/// @code{.cpp}
/// # Run the trials using the Angle method. Start at benchmark 100.
/// RunTrial 0 100
/// @endcode

#include <chrono>
#include <set>
#include "trial/trial.h"

/// Alias for trial function pointers.
typedef void (*trial_function) (Nibble &, std::ofstream &, unsigned int);

/// Record the header given the identification choice.
///
/// @param choice Identification method choice.
/// @param log Open file-stream to log. This is the same stream that will be used to record results.
void record_header(const int choice, std::ofstream &log) {
    switch(choice) {
        case 0: return (void) (log << Trial::ANGLE_ATTRIBUTE);
        case 1: return (void) (log << Trial::ASTRO_ATTRIBUTE);
        case 2: return (void) (log << Trial::SPHERE_ATTRIBUTE);
        case 3: return (void) (log << Trial::PLANE_ATTRIBUTE);
        case 4: return (void) (log << Trial::PYRAMID_ATTRIBUTE);
        default: throw "Identification choice is not within space {0, 1, 2, 3, 4}.";
    }
}

/// Return the appropriate trial function that varies the identification parameters given the identification choice.
///
/// @param choice Identification method choice.
/// @return Appropriate function pointer to a function in Trial.
trial_function select_parameter_trial(const int choice) {
    switch(choice) {
        case 0: return &Trial::iterate_angle_parameters;
        case 1: return &Trial::iterate_astro_parameters;
        case 2: return &Trial::iterate_sphere_parameters;
        case 3: return &Trial::iterate_plane_parameters;
        case 4: return &Trial::iterate_pyramid_parameters;
        default: throw "Identification choice is not within space {0, 1, 2, 3, 4}.";
    }
}

/// Return the appropriate trial function that varies the set_n given the identification choice.
///
/// @param choice Identification method choice.
/// @return Appropriate function pointer to a function in Trial.
trial_function select_set_n_trial(const int choice) {
    switch(choice) {
        case 0: return &Trial::iterate_angle_set_n;
        case 1: return &Trial::iterate_astro_set_n;
        case 2: return &Trial::iterate_sphere_set_n;
        case 3: return &Trial::iterate_plane_set_n;
        case 4: return &Trial::iterate_pyramid_set_n;
        default: throw "Identification choice is not within space {0, 1, 2, 3, 4}.";
    }
}

/// Run the trials! Select the appropriate header and trial function given the identification choice, and iterate
/// this for the defined benchmark bounds.
///
/// @param nb Open Nibble connection.
/// @param log Open file-stream to log to.
/// @param choice Identification choice.
/// @param start_bench Starting benchmark set_n.
/// @return 0 when finished.
int perform_trial (Nibble &nb, std::ofstream &log, const int choice, const int start_bench) {
    // Set the attributes of the log.
    record_header(choice, log);
    const auto BENCH_START = (unsigned) start_bench;

    // Select all clean benchmarks with field-of-view of 20.
    nb.select_table(Benchmark::TABLE_NAME);
    Nibble::tuple s = nb.search_table("fov = 17.5", "set_n", 30);

    // Select the specific trial functions, and run those specific trials.
    trial_function t_p = select_parameter_trial(choice), t_s = select_set_n_trial(choice);
    t_s(nb, log, BENCH_START);
    for (const double set_n : std::set<double> (s.begin(), s.end())) {
        t_p(nb, log, (unsigned int) set_n);
    }

    return 0;
}

/// Select the desired identification method given the first argument. In the second argument, indicate a starting
/// benchmark to run the trials from.
///
/// /// @code{.cpp}
/// - 0 x -> Run the trials with the Angle method.
/// - 1 x -> Run the trials with the AstrometryNet method.
/// - 2 x -> Run the trials with the SphericalTriangle method.
/// - 3 x -> Run the trials with the PlanarTriangle method.
/// - 4 x -> Run the trials with the Pyramid method.
///
/// - x [0->MAX(set_n)] -> Start the trial with the given set_n (Benchmark) number.
/// @endcode
///
/// @param argc Argument count. This must be equal to 3.
/// @param argv Argument vector. Determines which identification method to use, and which Benchmark to start from.
/// @return -1 if the arguments are incorrect. 0 otherwise.
int main (int argc, char *argv[]) {
    std::ostringstream l;
    std::ofstream log;
    Nibble nb(Benchmark::TABLE_NAME, "set_n");
    std::ios::sync_with_stdio(false);
    
    /// Alias for the clock in the Chrono library.
    using clock = std::chrono::high_resolution_clock;
    
    // Verify the arguments.
    if (argc != 3) {
        std::cout << "Usage: RunTrial [IdentificationChoice] [StartingBenchmark]" << std::endl;
        return -1;
    }
    else {
        // Verify that the first argument is within [0, 1, 2, 3, 4].
        auto is_valid_arg = [] (const char *arg, const std::vector<std::string> &input_space) -> bool {
            std::string a = std::string(arg);
            return std::find(input_space.begin(), input_space.end(), a) != input_space.end();
        };
        if (!is_valid_arg(argv[1], {"0", "1", "2", "3", "4"})) {
            std::cout << "Usage: RunTrial [0 - 4] [0 - MAX(set_n)]" << std::endl;
            return -1;
        }
        
        
        if ((int) strtol(argv[2], nullptr, 10) < 0 || (int) strtol(argv[2], nullptr, 10) > 
                nb.search_table("MAX (set_n)", 1, 1)[0]) {
            // Verify that the second argument is a valid set_n number.
            std::cout << "Usage: RunTrial [0 - 4] [0 - MAX(set_n)]" << std::endl;
            return -1;
        }
    }
    
    // Construct the log file based on the HOKU_PROJECT_PATH environment variable.
    l << "/data/logs/trial/" << argv[1] << "-"  << clock::to_time_t(clock::now() - std::chrono::hours(24)) << ".csv";
    log.open(std::string(std::getenv("HOKU_PROJECT_PATH")) + l.str());
    
    // Make sure the log file is open before proceeding.
    if (!log.is_open()) {
        throw "Log file cannot be opened.";
    }

    return perform_trial(nb, log, (int) strtol(argv[1], nullptr, 10), (int) strtol(argv[2], nullptr, 10));
}
