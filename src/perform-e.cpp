/// @file run-trial.cpp
/// @author Glenn Galvizo
///
/// Source file for the trial runner. Based on the arguments, run the specific trial for the given identification
/// method and log the data to a database. There exists five trial types, and five identification methods.
///
/// @code{.cpp}
/// To perform and log experiments...
/// - angle a -> Run trial A with the Angle method.
/// - interior a -> Run trial A with the InteriorAngle method.
/// - sphere a -> Run trial A with the SphericalTriangle method.
/// - plane a -> Run trial A with the PlanarTriangle method.
/// - pyramid a -> Run trial A with the Pyramid method.
/// - composite a -> Run trial A with the CompositePyramid method.
///
/// - b query -> Run the query trials with the B method.
/// - b reduction -> Run the reduction trials with the B method.
/// - b identification -> Run the identification trials with the B method.
///
/// To create lumberjack tables...
/// - create-table query -> Create the query table in the lumberjack database.
/// - create-table reduction -> Create the reduction table in the lumberjack database.
/// - create-table identification -> Create the identification table in the lumberjack database.
/// @endcode{.cpp}
/// @example
/// @code{.cpp}
/// # Run the identification trials using the Angle method.
/// RunTrial angle identification
/// @endcode

#include <chrono>
#include "identification/angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"
#include "experiment/experiment.h"

/// Convert the given user argument specifying the experiment name, to its appropriate hash.
///
/// @param experiment_in Input given by the user, to identify the type of experiment table.
/// @return Index of the name space below. 3 is given if the given input is not in the name space.
int experiment_hash (const std::string &experiment_in) {
    std::array<std::string, 5> space = {"query", "reduction", "identification"};
    return static_cast<int> (std::distance(space.begin(), std::find(space.begin(), space.end(), experiment_in)));
}

/// Convert the given user argument specifying the experiment name, to its appropriate table name in Lumberjack.
///
/// @param experiment_in Input given by the user, to identify the type of experiment table.
/// @return Name of the table to log to in the Lumberjack database.
std::string experiment_table (const std::string &experiment_in) {
    switch (experiment_hash(experiment_in)) {
        case 0: return "QUERY";
        case 1: return "REDUCTION";
        case 2: return "IDENTIFICATION";
        default: throw "Experiment name is not in the space of trial names.";
    }
}

/// Convert the given user argument specifying the identifier name, to its appropriate hash.
///
/// @param identifier_in Input given by the user, to identify the type of experiment table.
/// @return Index of the name space below. 6 is given if the given input is not in the name space.
int identifier_hash (const std::string &identifier_in) {
    std::array<std::string, 6> space = {"angle", "interior", "sphere", "plane", "pyramid", "composite"};
    return static_cast<int> (std::distance(space.begin(), std::find(space.begin(), space.end(), identifier_in)));
}

/// Create the appropriate lumberjack table.
///
/// @param experiment_in Input given by the user, to identify the type of experiment table to create.
/// @return TABLE_NOT_CREATED if the table already exists. Otherwise, 0 if successful.
int create_lumberjack_table (const std::string &experiment_in) {
    Chomp ch;
    
    switch (experiment_hash(experiment_in)) {
        case 0: return Lumberjack::create_table("QUERY", Experiment::Query::SCHEMA);
        case 2: return Lumberjack::create_table("REDUCTION", Experiment::Reduction::SCHEMA);
        case 3: return Lumberjack::create_table("IDENTIFICATION", Experiment::Identification::SCHEMA);
        default: throw "Experiment name is not in the space of trial names.";
    }
}

/// Run the specified trial! Select the appropriate header and trial function given the identification choice.
///
/// @param lu Open Lumberjack connection, used to log the results of each trial.
/// @param identifier_in Input given by the user, to identify the type of experiment table to log to.
/// @param experiment_in Input given by the user, to identify the type of identifier to use for the experiment.
void perform_trial (Lumberjack &lu, const std::string &identifier_in, const std::string &experiment_in) {
    std::array<std::string, 6> table_names = {"ANGLE_20", "INTERIOR_20", "SPHERE_20", "PLANE_20", "PYRAMID_20",
        "COMPOSITE_20"};
    Chomp ch;
    
    switch ((identifier_hash(identifier_in) * 5) + experiment_hash(experiment_in)) {
        case 0: return Experiment::Query::trial<Angle>(ch, lu, table_names[0]);
        case 1: return Experiment::Reduction::trial<Angle>(ch, lu, table_names[0]);
        case 2: return Experiment::Identification::trial<Angle>(ch, lu, table_names[0]);
        
        case 3: throw "Not implemented.";
        case 4: throw "Not implemented.";
        case 5: throw "Not implemented.";
        
        case 6: return Experiment::Query::trial<Sphere>(ch, lu, table_names[1]);
        case 7: return Experiment::Reduction::trial<Sphere>(ch, lu, table_names[1]);
        case 8: return Experiment::Identification::trial<Sphere>(ch, lu, table_names[1]);
        
        case 9: return Experiment::Query::trial<Plane>(ch, lu, table_names[2]);
        case 10: return Experiment::Reduction::trial<Plane>(ch, lu, table_names[2]);
        case 11: return Experiment::Identification::trial<Plane>(ch, lu, table_names[2]);
        
        case 12: return Experiment::Query::trial<Pyramid>(ch, lu, table_names[3]);
        case 13: return Experiment::Reduction::trial<Pyramid>(ch, lu, table_names[3]);
        case 14: return Experiment::Identification::trial<Pyramid>(ch, lu, table_names[3]);
        
        case 15: throw "Not implemented.";
        case 16: throw "Not implemented.";
        case 17: throw "Not implemented.";
        
        default: throw "Choices not in appropriate spaces or test does not exist.";
    }
}

/// Select the desired identification method (or 'create-table' to create a table) given the first argument. In the
/// second argument, indicate the type of trial desired.
///
/// @code{.cpp}
/// To perform and log experiments...
/// - angle a -> Run trial A with the Angle method.
/// - sphere a -> Run trial A with the SphericalTriangle method.
/// - plane a -> Run trial A with the PlanarTriangle method.
/// - pyramid a -> Run trial A with the Pyramid method.
///
/// - b query -> Run the query trials with the B method.
/// - b first-identification -> Run the first identification trials with the B method.
/// - b reduction -> Run the reduction trials with the B method.
/// - b identification -> Run the identification trials with the B method.
/// - b crown -> Run the crown trials with the B method.
///
/// To create lumberjack tables...
/// - create-table query -> Create the query table in the lumberjack database.
/// - create-table first-identification -> Create the first identification table in the lumberjack database.
/// - create-table reduction -> Create the reduction table in the lumberjack database.
/// - create-table identification -> Create the identification table in the lumberjack database.
/// - create-table crown -> Create the crown table in the lumberjack database.
/// @endcode{.cpp}
///
/// @param argc Argument count. This must be equal to 3.
/// @param argv Argument vector. Determines which trial to run, and which identification method to test.
/// @return -1 if the arguments are incorrect. 0 otherwise.
int main (int argc, char *argv[]) {
    std::ios::sync_with_stdio(false);
    std::ostringstream l;
    
    /// Determine the timestamp.
    using clock = std::chrono::high_resolution_clock;
    l << clock::to_time_t(clock::now() - std::chrono::hours(24));
    
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
        
        if (!is_valid_arg(argv[1], {"angle", "interior", "sphere", "plane", "pyramid", "composite", "create-table"})
            || !is_valid_arg(argv[2], {"query", "reduction", "identification"})) {
            std::cout << "Usage: RunTrial ['angle', ... , 'create-table'] ['query', ... , 'identification']"
                      << std::endl;
            return -1;
        }
    }
    
    // Create the desired table if specified. Capitalize our trial name.
    std::string arg_1 = std::string(argv[1]), trial_name = std::string(argv[1]);
    if (arg_1 == "create-table") {
        return create_lumberjack_table(trial_name);
    }
    arg_1[0] = static_cast<char> (toupper(arg_1[0]));
    
    // Perform our trial.
    Lumberjack lu(experiment_table(std::string(argv[2])), arg_1, l.str());
    perform_trial(lu, std::string(argv[1]), std::string(argv[2]));
    return 0;
}
