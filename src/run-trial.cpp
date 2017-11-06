/// @file run-trial.cpp
/// @author Glenn Galvizo
///
/// Source file for the trial runner. Based on the arguments, run the specific trial for the given identification
/// method and log the data to a CSV file. There exists three trial types, and four identification methods.
///
/// @code{.cpp}
/// - 0 a -> Run trial A with the Angle method.
/// - 1 a -> Run trial A with the SphericalTriangle method.
/// - 2 a -> Run trial A with the PlanarTriangle method.
/// - 3 a -> Run trial A with the Pyramid method.
///
/// - b 0 -> Run the query trials with the B method.
/// - b 1 -> Run the alignment trials with the B method.
/// - b 2 -> Run the crown trials with the B method.
/// @endcode{.cpp}
/// @example
/// @code{.cpp}
/// # Run the alignment trials using the Angle method.
/// RunTrial 0 1
/// @endcode

#include <chrono>
#include "trial/query.h"
#include "trial/alignment.h"
#include "trial/crown.h"

/// Alias for trial function pointers.
typedef void (*trial_function) (Chomp &, std::ofstream &);

/// Record the header given the trial choice.
///
/// @param choice Choice corresponding to the trial. Must exist in space [0, 1, 2].
/// @param log Open file-stream to log. This is the same stream that will be used to record results.
void record_header (const int choice, std::ofstream &log) {
    switch (choice) {
        case 0: return (void) (log << Query::ATTRIBUTE);
        case 1: return (void) (log << Alignment::ATTRIBUTE);
        case 2: return (void) (log << Crown::ATTRIBUTE);
        default: throw "Trial choice is not within space {0, 1, 2}.";
    }
}

/// Return the appropriate trial function given the identification and trial choices.
///
/// @param identification_choice Choice corresponding to the identification method. Must exist in space [0, 1, 2, 3, 4].
/// @param trial_choice Choice corresponding to the trial. Must exist in space [0, 1, 2].
/// @return
trial_function select_trial (const int identification_choice, const int trial_choice) {
    switch (trial_choice * 5 + identification_choice) {
        case 0: return &Query::trial_angle;
        case 1: return &Query::trial_sphere;
        case 2: return &Query::trial_plane;
        case 3: return &Query::trial_pyramid;
        case 4: return &Query::trial_coin;
        case 5: return &Alignment::trial_angle;
        case 6: return &Alignment::trial_sphere;
        case 7: return &Alignment::trial_plane;
        case 8: return &Alignment::trial_pyramid;
        case 9: return &Alignment::trial_coin;
        case 10: return &Crown::trial_angle;
        case 11: return &Crown::trial_sphere;
        case 12: return &Crown::trial_plane;
        case 13: return &Crown::trial_pyramid;
        case 14: return &Crown::trial_coin;
        default: throw "Choices not in appropriate spaces or test does not exist.";
    }
}

/// Run the specified trial! Select the appropriate header and trial function given the identification choice.
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param log Open file-stream to log to.
/// @param id_choice Identification choice, in space [0, 1, 2, 3].
/// @param trial_choice Trial choice, in space [0, 1, 2].
/// @return 0 when finished.
int perform_trial (Chomp &ch, std::ofstream &log, const int id_choice, const int trial_choice) {
    // Set the attributes of the log.
    record_header(trial_choice, log);
    
    // Select the specific trial functions, and run those specific trials.
    select_trial(id_choice, trial_choice)(ch, log);
    
    return 0;
}

/// Select the desired identification method given the first argument. In the second argument, indicate the type of
/// trial desired.
///
/// @code{.cpp}
/// - 0 a -> Run trial A with the Angle method.
/// - 1 a -> Run trial A with the SphericalTriangle method.
/// - 2 a -> Run trial A with the PlanarTriangle method.
/// - 3 a -> Run trial A with the Pyramid method.
///
/// - b 0 -> Run the query trials with the B method.
/// - b 1 -> Run the alignment trials with the B method.
/// - b 2 -> Run the crown trials with the B method.
/// @endcode{.cpp}
///
/// @param argc Argument count. This must be equal to 3.
/// @param argv Argument vector. Determines which trial to run, and which identification method to test.
/// @return -1 if the arguments are incorrect. 0 otherwise.
int main (int argc, char *argv[]) {
    std::ios::sync_with_stdio(false);
    std::ostringstream l;
    std::ofstream log;
    Chomp ch;
    
    /// Alias for the clock in the Chrono library.
    using clock = std::chrono::high_resolution_clock;
    
    // Verify the arguments.
    if (argc != 3) {
        std::cout << "Usage: RunTrial [IdentificationChoice] [TrialChoice]" << std::endl;
        return -1;
    }
    else {
        // Verify that the arguments are within their appropriate spaces.
        auto is_valid_arg = [] (const char *arg, const std::vector<std::string> &input_space) -> bool {
            std::string a = std::string(arg);
            return std::find(input_space.begin(), input_space.end(), a) != input_space.end();
        };
        if (!is_valid_arg(argv[1], {"0", "1", "2", "3", "4"}) || !is_valid_arg(argv[2], {"0", "1", "2", "3"})) {
            std::cout << "Usage: RunTrial [0 - 4] [0 - 3]" << std::endl;
            return -1;
        }
    }
    
    // Construct the log file based on the HOKU_PROJECT_PATH environment variable.
    l << "/data/logs/trial/" << argv[1] << "-" << clock::to_time_t(clock::now() - std::chrono::hours(24)) << ".csv";
    log.open(std::string(std::getenv("HOKU_PROJECT_PATH")) + l.str());
    
    // Make sure the log file is open before proceeding.
    if (!log.is_open()) {
        throw "Log file cannot be opened.";
    }
    
    return perform_trial(ch, log, (int) strtol(argv[1], nullptr, 10), (int) strtol(argv[2], nullptr, 10));
}
