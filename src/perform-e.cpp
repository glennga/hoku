/// @file run-trial.cpp
/// @author Glenn Galvizo
///
/// Source file for the trial runner. Based on the arguments, run the specific trial for the given identification
/// method and log the data to a database. There exists four trial types, and six identification methods.
///
/// @code{.cpp}
/// To perform and log experiments...
/// - angle a -> Run trial A with the Angle method.
/// - dot a -> Run trial A with the DotAngle method.
/// - sphere a -> Run trial A with the SphericalTriangle method.
/// - plane a -> Run trial A with the PlanarTriangle method.
/// - pyramid a -> Run trial A with the Pyramid method.
/// - composite a -> Run trial A with the CompositePyramid method.
///
/// - b query -> Run the query trials with the B method.
/// - b reduction -> Run the reduction trials with the B method.
/// - b identification -> Run the identification trials with the B method.
/// - b overlay -> Run the overlay trials. The passed B method does not matter.
/// @endcode{.cpp}
/// @example
/// @code{.cpp}
/// # Run the identification trials using the Angle method.
/// RunTrial angle identification
/// @endcode

#include <chrono>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <libgen.h>

#include "identification/angle.h"
#include "identification/dot-angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"
#include "identification/composite-pyramid.h"
#include "experiment/experiment.h"

/// @brief Namespace containing all table 'hashes' for experimentation.
///
/// Hash functions used in experimentation (finding Lumberjack tables, identifier methods).
namespace EHA {
    /// Convert the given user argument specifying the experiment name, to its appropriate hash.
    ///
    /// @param experiment_in Input given by the user, to identify the type of experiment table.
    /// @return Index of the name space below. 3 is given if the given input is not in the name space.
    int experiment_to_hash (const std::string &experiment_in) {
        std::string experiment_in_upper;
        std::transform(experiment_in.begin(), experiment_in.end(), experiment_in_upper.begin(), ::toupper);

        std::array<std::string, 4> space = {"QUERY", "REDUCTION", "IDENTIFICATION", "OVERLAY"};
        return static_cast<int> (std::distance(space.begin(), std::find(space.begin(), space.end(),
                                                                        experiment_in_upper)));
    }

    /// Convert the given user argument specifying the experiment name, to its appropriate table name in Lumberjack.
    ///
    /// @param experiment_in Input given by the user, to identify the type of experiment table.
    /// @return Name of the table to log to in the Lumberjack database.
    /// @param cf Configuration file reader holding the *.ini information.
    std::string experiment_to_experiment_table (const std::string &experiment_in, INIReader &cf) {
        int d = experiment_to_hash(experiment_in);
        if (d < 0 || d > 5) {
            throw std::runtime_error(std::string("Experiment name is not in the space of trial names."));
        }

        // Extract the uppercase and lowercase versions of our name.
        std::string experiment_in_upper, experiment_in_lower;
        std::transform(experiment_in.begin(), experiment_in.end(), experiment_in_upper.begin(), ::toupper);
        std::transform(experiment_in.begin(), experiment_in.end(), experiment_in_lower.begin(), ::tolower);

        return cf.Get(experiment_in_lower + "-experiment", "lu", experiment_in_upper);
    }

    /// Convert the given user argument specifying the identifier name, to its appropriate hash.
    ///
    /// @param identifier_in Input given by the user, to identify the type of experiment table.
    /// @return Index of the name space below. 6 is given if the given input is not in the name space.
    int identifier_to_hash (const std::string &identifier_in) {
        std::array<std::string, 6> space = {"angle", "dot", "sphere", "plane", "pyramid", "composite"};
        return static_cast<int> (std::distance(space.begin(), std::find(space.begin(), space.end(), identifier_in)));
    }
}

/// Create the appropriate lumberjack table.
///
/// @param experiment_in Input given by the user, to identify the type of experiment table to create.
/// @param cf Configuration file reader holding the *.ini information.
/// @return TABLE_NOT_CREATED if the table already exists. Otherwise, 0 if successful.
int create_lumberjack_table (const std::string &experiment_in, INIReader &cf) {
    Chomp ch;

    // Using convention (query, reduction, map, overlay) in EHA. Create the appropriate table.
    std::array<std::string, 4> schema_space = {Experiment::Query::SCHEMA, Experiment::Reduction::SCHEMA,
                                               Experiment::Map::SCHEMA, Experiment::Overlay::SCHEMA};
    return Lumberjack::create_table(EHA::experiment_to_experiment_table(experiment_in, cf),
                                    schema_space[EHA::experiment_to_hash(experiment_in)]);
}

/// Run the specified trial! Select the appropriate header and trial function given the identification choice.
///
/// @param lu Open Lumberjack connection, used to log the results of each trial.
/// @param identifier_in Input given by the user, to identify the type of experiment table to log to.
/// @param experiment_in Input given by the user, to identify the type of identifier to use for the experiment.
/// @param cf Configuration file reader holding the *.ini information.
void perform_trial (Lumberjack &lu, const std::string &identifier_in, const std::string &experiment_in,
                    INIReader &cf) {
//    Chomp ch(cf.Get("table-names", identifier_in, ""), cf.Get("table-focus", identifier_in, ""));
    Chomp ch;

    // Can't really think of a better way to do this than just listing it out. So... yeah.
    std::array<void (*) (Chomp &, Lumberjack &, INIReader &, const std::string &), 24> experiment_space = {
            Experiment::Query::trial<Angle>, Experiment::Reduction::trial<Angle>,
            Experiment::Map::trial<Angle>, Experiment::Overlay::trial<Angle>,

            Experiment::Query::trial<Dot>, Experiment::Reduction::trial<Dot>,
            Experiment::Map::trial<Dot>, Experiment::Overlay::trial<Dot>,

            Experiment::Query::trial<Sphere>, Experiment::Reduction::trial<Sphere>,
            Experiment::Map::trial<Sphere>, Experiment::Overlay::trial<Sphere>,

            Experiment::Query::trial<Plane>, Experiment::Reduction::trial<Plane>,
            Experiment::Map::trial<Plane>, Experiment::Overlay::trial<Plane>,

            Experiment::Query::trial<Pyramid>, Experiment::Reduction::trial<Pyramid>,
            Experiment::Map::trial<Pyramid>, Experiment::Overlay::trial<Pyramid>,

            Experiment::Query::trial<Composite>, Experiment::Reduction::trial<Composite>,
            Experiment::Map::trial<Composite>, Experiment::Overlay::trial<Composite>
    };

    int d = (EHA::identifier_to_hash(identifier_in) * 4) + EHA::experiment_to_hash(experiment_in);
    experiment_space[d](ch, lu, cf, identifier_in);
}

/// Select the desired identification method given the first argument. In the second argument, indicate the type of
/// trial desired.
///
/// @code{.cpp}
/// To perform and log experiments...
/// - angle a -> Run trial A with the Angle method.
/// - dot a -> Run trial A with the DotAngle method.
/// - sphere a -> Run trial A with the SphericalTriangle method.
/// - plane a -> Run trial A with the PlanarTriangle method.
/// - pyramid a -> Run trial A with the Pyramid method.
/// - composite a -> Run trial A with the CompositePyramid method.
///
/// - b query -> Run the query trials with the B method.
/// - b reduction -> Run the reduction trials with the B method.
/// - b identification -> Run the identification trials with the B method.
/// - b overlay -> Run the overlay trials. The passed B method does not matter.
/// @endcode{.cpp}
///
/// @param argc Argument count. This must be equal to 3.
/// @param argv Argument vector. Determines which trial to run, and which identification method to test.
/// @return -1 if the arguments are incorrect. 0 otherwise.
int main (int argc, char *argv[]) {
    std::ios::sync_with_stdio(false);
    std::ostringstream l;

    /// Determine the timestamp.
    using clock = std::chrono::system_clock;
    l << clock::to_time_t(clock::now() - std::chrono::hours(24));

    /// INIReader to hold configuration associated with experiments.
    INIReader cf(std::getenv("HOKU_CONFIG_INI") ? std::string(std::getenv("HOKU_CONFIG_INI")) :
                 std::string(dirname(const_cast<char *>(__FILE__))) + "/../../CONFIG.ini");

    // Verify the arguments.
    if (argc != 3) {
        std::cout << "Usage: RunTrial [IdentificationChoice] [TrialChoice]" << std::endl;
        return -1;
    }
    else {
        // Verify that the arguments are within their appropriate spaces.
        auto is_valid_arg = [] (const char *arg, const std::vector<std::string> &input_space) -> bool {
            std::string a = std::string(arg);

            std::transform(a.begin(), a.end(), a.begin(), ::tolower);
            return std::find(input_space.begin(), input_space.end(), a) != input_space.end();
        };

        if (!is_valid_arg(argv[1], {"angle", "dot", "sphere", "plane", "pyramid", "composite"})
            || !is_valid_arg(argv[2], {"query", "reduction", "identification", "overlay"})) {
            std::cout << "Usage: RunTrial ['angle', ... , 'composite'] ['query', ... , 'identification']" << std::endl;
            return -1;
        }
    }

    // Attempt to connect to the Lumberjack database.
    std::string identification = std::string(argv[1]), identifier_name = std::string(argv[1]);
    std::string trial_name = std::string(argv[2]);
    try {
        Lumberjack lu(EHA::experiment_to_experiment_table(trial_name, cf), identification, l.str());
    }
    catch (const std::exception &e) {
        create_lumberjack_table(trial_name, cf);
    }

    // Capitalize our trial name. Perform our trial.
    identification[0] = static_cast<char> (toupper(identification[0]));
    Lumberjack lu(EHA::experiment_to_experiment_table(trial_name, cf), identification, l.str());
    perform_trial(lu, identifier_name, trial_name, cf);
    return 0;
}
