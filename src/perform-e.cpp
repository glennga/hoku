/// @file perform-e.cpp
/// @author Glenn Galvizo
///
/// Source file for the trial runner. Based on the arguments, run the specific trial for the given identification
/// method and log the data to a database. There exists four trial types, and six identification methods. This is
/// **not** meant to be used as is, rather is meant to be the entry point for the python script calling this.

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

enum PerformEArguments {
    REFERENCE_DB = 1,
    RECORD_DB = 2,
    HIP_TABLE = 3,
    BRIGHT_TABLE = 4,
    REFERENCE_TABLE = 5,
    EXPERIMENT_TABLE = 6,
    EXPERIMENT_SCHEMA = 7,
    IDENTIFICATION_STRATEGY = 8,
    IDENTIFICATION_PREFIX = 9,
    EPSILON_1 = 10,
    EPSILON_2 = 11,
    EPSILON_3 = 12,
    EPSILON_4 = 13,
    N_LIMIT = 14,
    M_LIMIT = 15,
    NU_LIMIT = 16,
    EXPERIMENT_NAME = 17,
    SAMPLES = 18,
    IMAGE_FOV = 19,
    SHIFT_STAR_ITER = 20,
    SHIFT_STAR_STEP = 21,
    EXTRA_STAR_MIN = 22,
    EXTRA_STAR_ITER = 23,
    EXTRA_STAR_STEP = 24,
    REMOVE_STAR_ITER = 25,
    REMOVE_STAR_STEP = 26,
    REMOVE_STAR_SIGMA = 27
};

using ExperimentFunction = void (*) (
        const std::shared_ptr<Chomp> &,
        const std::shared_ptr<Lumberjack> &,
        const std::shared_ptr<Experiment::Parameters> &
);
template<class T>
ExperimentFunction generic_experiment_factory (const std::string &experiment_name) {
    std::map<std::string, ExperimentFunction> experiment_map;
    experiment_map["QUERY"] = Experiment::Query::trial<T>;
    experiment_map["REDUCTION"] = Experiment::Reduction::trial<T>;
    experiment_map["MAP"] = Experiment::Map::trial<T>;

    std::string upper_experiment_name = experiment_name;
    std::transform(experiment_name.begin(), experiment_name.end(), upper_experiment_name.begin(), ::toupper);

    if (experiment_map.find(upper_experiment_name) == experiment_map.end())
        throw std::runtime_error("'experiment_name' must be in space [QUERY, REDUCTION, MAP].");

    return experiment_map[upper_experiment_name];
}

ExperimentFunction experiment_factory (const std::string &experiment_name, const std::string &strategy) {
    std::string upper_strategy = strategy;
    std::transform(strategy.begin(), strategy.end(), upper_strategy.begin(), ::toupper);

    if (upper_strategy == "ANGLE") return generic_experiment_factory<Angle>(experiment_name);
    else if (upper_strategy == "DOT") return generic_experiment_factory<Dot>(experiment_name);
    else if (upper_strategy == "PLANE") return generic_experiment_factory<Plane>(experiment_name);
    else if (upper_strategy == "SPHERE") return generic_experiment_factory<Sphere>(experiment_name);
    else if (upper_strategy == "PYRAMID") return generic_experiment_factory<Pyramid>(experiment_name);
    else if (upper_strategy == "COMPOSITE") return generic_experiment_factory<Composite>(experiment_name);
    else throw std::runtime_error("'strategy' must be in space [ANGLE, DOT, PLANE, SPHERE, PYRAMID, COMPOSITE].");
}

void connect_to_lumberjack (char *argv[], std::ostringstream &l) {
    try {
        Lumberjack::Builder()
                .with_database_name(argv[PerformEArguments::RECORD_DB])
                .using_timestamp(l.str())
                .using_trial_table(argv[PerformEArguments::EXPERIMENT_TABLE])
                .with_prefix(argv[PerformEArguments::IDENTIFICATION_PREFIX])
                .build();
    }
    catch (const std::exception &e) {
        Lumberjack::create_table(
                argv[PerformEArguments::RECORD_DB],
                argv[PerformEArguments::EXPERIMENT_TABLE],
                argv[PerformEArguments::EXPERIMENT_SCHEMA]
        );
    }
}

int main (int, char *argv[]) {
    std::ios::sync_with_stdio(false); // Determine the timestamp.
    std::ostringstream l;
    l << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() - std::chrono::hours(24));
    connect_to_lumberjack(argv, l);  // Populate lumberjack table if it does not already exist.

    // Perform the experiment! It looks like I really like the builder pattern.
    experiment_factory(argv[PerformEArguments::EXPERIMENT_NAME], argv[PerformEArguments::IDENTIFICATION_STRATEGY])(
            std::make_shared<Chomp>(
                    Chomp::Builder()
                            .with_database_name(argv[PerformEArguments::REFERENCE_DB])
                            .with_hip_name(argv[PerformEArguments::HIP_TABLE])
                            .with_bright_name(argv[PerformEArguments::BRIGHT_TABLE])
                            .build()
            ),
            std::make_shared<Lumberjack>(
                    Lumberjack::Builder()
                            .with_database_name(argv[PerformEArguments::RECORD_DB])
                            .using_timestamp(l.str())
                            .using_trial_table(argv[PerformEArguments::EXPERIMENT_TABLE])
                            .with_prefix(argv[PerformEArguments::IDENTIFICATION_PREFIX])
                            .build()
            ),
            std::make_shared<Experiment::Parameters>(
                    Experiment::ParametersBuilder()
                            .prefixed_by(argv[PerformEArguments::IDENTIFICATION_PREFIX])
                            .using_reference_table(argv[PerformEArguments::REFERENCE_TABLE])
                            .with_image_of_size(std::stod(argv[PerformEArguments::IMAGE_FOV]))
                            .with_epsilon(
                                    std::stod(argv[PerformEArguments::EPSILON_1]),
                                    std::stod(argv[PerformEArguments::EPSILON_2]),
                                    std::stod(argv[PerformEArguments::EPSILON_3]),
                                    std::stod(argv[PerformEArguments::EPSILON_4])
                            )
                            .limited_by_n(std::stoi(argv[PerformEArguments::N_LIMIT]))
                            .limited_by_m(std::stod(argv[PerformEArguments::M_LIMIT]))
                            .limited_by_nu(std::stoi(argv[PerformEArguments::NU_LIMIT]))
                            .repeated_for_n_times(std::stoi(argv[PerformEArguments::SAMPLES]))
                            .with_n_shift_star_trials(std::stoi(argv[PerformEArguments::SHIFT_STAR_ITER]))
                            .with_n_extra_star_trials(std::stoi(argv[PerformEArguments::EXTRA_STAR_ITER]))
                            .with_n_remove_star_trials(std::stoi(argv[PerformEArguments::REMOVE_STAR_ITER]))
                            .using_shift_star_parameters(std::stod(argv[PerformEArguments::SHIFT_STAR_STEP]))
                            .using_extra_star_parameters(
                                    std::stoi(argv[PerformEArguments::EXTRA_STAR_MIN]),
                                    std::stoi(argv[PerformEArguments::EXTRA_STAR_STEP])
                            )
                            .using_remove_star_parameters(
                                    std::stoi(argv[PerformEArguments::REMOVE_STAR_STEP]),
                                    std::stoi(argv[PerformEArguments::REMOVE_STAR_SIGMA])
                            )
                            .build()
            )
    );
}