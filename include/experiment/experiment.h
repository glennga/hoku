/// @file trial.h
/// @author Glenn Galvizo
///
/// Header file for all trials. This holds the namespaces of functions that allow us to test various
/// methods and log the data.

#ifndef HOKU_EXPERIMENT_H
#define HOKU_EXPERIMENT_H

#include <cmath>
#include "third-party/inih/INIReader.h"

#include "benchmark/benchmark.h"
#include "identification/identification.h"
#include "experiment/lumberjack.h"

/// @brief Namespace that holds all namespaces, functions, and parameters used to conduct every experiment with.
///
/// All experiment modifications (parameters) should be performed in **this file only**. \n
/// There exist three subnamespaces holding the required functions for each experiment:
/// @code{.cpp}
/// 1. Query
/// 2. Reduction
/// 3. Identification
/// @endcode
namespace Experiment {
    void present_benchmark (Chomp &, Star::list &, Star &, double = 0);
    
    /// @brief Namespace that holds all parameters and functions to conduct the query experiment with.
    ///
    /// The query experiment is used to characterize the choose query stars and search catalog steps.
    namespace Query {
        /// Schema comma separated string that corresponds to the creation of the query table.
        const char *const SCHEMA = "IdentificationMethod TEXT, Timestamp TEXT, SigmaQuery FLOAT, ShiftDeviation FLOAT, "
            "CandidateSetSize FLOAT, SExistence INT";
        
        Star::list generate_n_stars (Chomp &ch, unsigned int n, Star &focus);
        bool set_existence (std::vector<Identification::labels_list> &r_set, Identification::labels_list &b);
        
        /// Generic experiment function for the query trials. Performs a query trial and records the experiment in
        /// the lumberjack. The provided Nibble connection is used for generating the input image.
        ///
        /// @tparam T Identification class to perform query experiment with.
        /// @param ch Nibble connection used to generate the input image.
        /// @param lu Lumberjack connection used to record the results of each trial.
        /// @param table_name Name of the table used with the specified identifier.
        template <class T>
        void trial (Chomp &ch, Lumberjack &lu, const std::string &table_name) {
            INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));
            Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
            Star focus;
            
            p.sigma_query = cf.GetReal("id-parameters", "sq", 0), p.table_name = cf.Get("query-experiment", "lu", "");
            for (int ss_i = 0; ss_i < cf.GetReal("query-experiment", "ss-iter", 0); ss_i++) {
                double ss = (ss_i == 0) ? 0 : cf.GetReal("query-experiment", "ss-step", 0) * ss_i;
                
                // Repeat each trial n = SAMPLES times.
                for (int i = 0; i < cf.GetInteger("general-experiment", "samples", 0); i++) {
                    Star::list s = generate_n_stars(ch, T::QUERY_STAR_SET_SIZE, focus);
                    Benchmark beta(s, focus, cf.GetReal("hardware", "fov", 0));
                    beta.shift_light(T::QUERY_STAR_SET_SIZE, ss);
                    
                    // Perform a single trial.
                    std::vector<Identification::labels_list> r = T(beta, p).query(s);
                    
                    // Create the list to compare to.
                    Identification::labels_list w;
                    w.reserve(T::QUERY_STAR_SET_SIZE);
                    for (unsigned int j = 0; j < T::QUERY_STAR_SET_SIZE; j++) {
                        w.emplace_back(s[j].get_label());
                    }
                    
                    // Log the results of the trial.
                    lu.log_trial({cf.GetReal("id-parameters", "sq", 0), ss, static_cast<double> (r.size()),
                                     (set_existence(r, w) ? 1.0 : 0)});
                }
            }
        }
    }
    
    /// @brief Namespace that holds all parameters and functions to conduct the reduction experiment with.
    ///
    /// The reduction experiment is used to characterize the choose query stars step to our attitude determination step.
    namespace Reduction {
        /// Schema header that corresponds to the log file for all reduction trials.
        const char *const SCHEMA = "IdentificationMethod TEXT, Timestamp TEXT, SigmaQuery FLOAT, SigmaOverlay FLOAT, "
            "ShiftDeviation FLOAT, CameraSensitivity FLOAT, ResultMatchesInput INT";
        
        bool is_correctly_identified (const Star::list &body, const Identification::labels_list &r_labels);
        
        /// Generic experiment function for the reduction trials. Performs a reduction trial and records the experiment
        /// in the lumberjack. The provided Nibble connection is used for generating the input image.
        ///
        /// @tparam T Identification class to perform reduction experiment with.
        /// @param ch Nibble connection used to generate the input image.
        /// @param lu Lumberjack connection used to record the results of each trial.
        /// @param table_name Name of the table used with the specified identifier.
        template <class T>
        void trial (Chomp &ch, Lumberjack &lu, const std::string &table_name) {
            Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
            Star::list body;
            unsigned int nu = 0;
            Star focus;
            
            // Define our hyperparameters.
            p.nu_max = 20000, p.sigma_overlay = Reduction::SO_MIN, p.sigma_query = Reduction::SQ_MIN;
            p.table_name = table_name, p.nu = std::make_shared<unsigned int>(nu);
            
            // First run is clean, without shifts. Following are the shift trials.
            for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
                for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
                    
                    // Repeat each trial n = SAMPLES times.
                    for (int i = 0; i < cf.GetInteger("general-experiment", "samples", 0); i++) {
                        present_benchmark(ch, body, focus, MB_MIN + mb_i * MB_STEP);
                        Benchmark input(body, focus, cf.GetReal("hardware", "fov", 0));
                        input.shift_light(static_cast<signed> (body.size()),
                                          ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)));
                        
                        // Perform a single trial.
                        Identification::labels_list w = T(input, p).reduce();
                        
                        // Log the results of our trial.
                        lu.log_trial({Reduction::SQ_MIN, Reduction::SO_MIN, ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)),
                                         MB_MIN + mb_i * MB_STEP, (is_correctly_identified(body, w) ? 1.0 : 0)});
                    }
                }
            }
        }
    }
    
    /// @brief Namespace that holds all parameters and functions to conduct the identification experiment with.
    ///
    /// The identification experiment is used to characterize each identifier from start to identification.
    namespace Map {
        /// Schema header that corresponds to the log file for all identification trials.
        const char *const SCHEMA = "IdentificationMethod TEXT, Timestamp TEXT, SigmaQuery FLOAT, SigmaOverlay FLOAT, "
            "ShiftDeviation FLOAT, CameraSensitivity FLOAT, FalseStars INT, ComparisonCount INT, "
            "IsCorrectlyIdentified INT";
        
        bool is_correctly_identified (const Star::list &body, const Star::list &w);
        
        /// Generic experiment function for the identification trials. Performs a identification trial and records the
        /// experiment in the lumberjack. The provided Nibble connection is used for generating the input image.
        ///
        /// @tparam T Identification class to perform identification experiment with.
        /// @param ch Nibble connection used to generate the input image.
        /// @param lu Lumberjack connection used to record the results of each trial.
        /// @param table_name Name of the table used with the specified identifier.
        template <class T>
        void trial (Chomp &ch, Lumberjack &lu, const std::string &table_name) {
            Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
            Star::list body;
            unsigned int nu = 0;
            Star focus;
            
            // Define our hyperparameters.
            p.nu_max = 20000, p.sigma_overlay = Map::SO_MIN, p.sigma_query = Map::SQ_MIN;
            p.table_name = table_name, p.nu = std::make_shared<unsigned int>(nu);
            
            for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
                double ss = ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i));
                for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
                    for (int es_i = 0; es_i < ES_ITER; es_i++) {
                        
                        // Repeat each trial n = SAMPLES times.
                        for (int i = 0; i < cf.GetInteger("general-experiment", "samples", 0); i++) {
                            present_benchmark(ch, body, focus, MB_MIN + mb_i * MB_STEP);
                            Benchmark input(body, focus, cf.GetReal("hardware", "fov", 0));
                            
                            // Append our error.
                            input.shift_light(static_cast<int> (body.size()), ss);
                            double e = ES_MIN + ES_STEP * es_i, clean_size = body.size();
                            input.add_extra_light(static_cast<unsigned int> ((e / (1 - e)) * clean_size));
                            
                            // Perform a single trial.
                            Star::list w = T(input, p).identify();
                            
                            // Log the results of our trial.
                            lu.log_trial(
                                {Map::SQ_MIN, Map::SO_MIN, ss, MB_MIN + mb_i * MB_STEP, e, static_cast<double> (*p.nu),
                                    (is_correctly_identified(body, w) ? 1.0 : 0)});
                        }
                    }
                }
            }
        }
    }
}

#endif /* HOKU_EXPERIMENT_H */
