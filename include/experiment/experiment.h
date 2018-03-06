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
    void present_benchmark (Chomp &ch, Star::list &big_i, Star &center, double fov, double m_bar = 6.0);
    
    /// @brief Namespace that holds all parameters and functions to conduct the query experiment with.
    ///
    /// The query experiment is used to characterize the choose query stars and search catalog steps.
    namespace Query {
        /// Schema comma separated string that corresponds to the creation of the query table.
        const char *const SCHEMA = "IdentificationMethod TEXT, Timestamp TEXT, Sigma1 FLOAT, Sigma2 FLOAT, "
            "Sigma3 FLOAT, ShiftDeviation FLOAT, CandidateSetSize FLOAT, SExistence INT";
        
        Star::list generate_n_stars (Chomp &ch, unsigned int n, Star &center, double fov);
        bool set_existence (std::vector<Identification::labels_list> &big_r_ell,
                            Identification::labels_list &big_i_ell);
        
        /// Generic experiment function for the query trials. Performs a query trial and records the experiment in
        /// the lumberjack. The provided Nibble connection is used for generating the input image.
        ///
        /// @tparam T Identification class to perform query experiment with.
        /// @param ch Nibble connection used to generate the input image.
        /// @param lu Lumberjack connection used to record the results of each trial.
        /// @param identifier Name of the identification method, in format (space): [angle, dot, sphere, plane,
        /// pyramid, composite].
        template <class T>
        void trial (Chomp &ch, Lumberjack &lu, INIReader &cf, const std::string &identifier) {
            Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
            double fov = cf.GetReal("hardware", "fov", 0);
            Star focus;
            
            // Define our hyperparameters and testing parameters.
            Identification::collect_parameters(p, cf, identifier);
            int samples = cf.GetInteger("general-experiment", "samples", 0);
            int ss_iter = cf.GetInteger("query-experiment", "ss-iter", 0);
            double ss_step = cf.GetReal("query-experiment", "ss-step", 0);
            
            for (int ss_i = 0; ss_i < ss_iter; ss_i++) {
                double ss = (ss_i == 0) ? 0 : pow(ss_step, ss_i);
                
                // Repeat each trial n = SAMPLES times.
                for (int i = 0; i < samples; i++) {
                    Star::list s = generate_n_stars(ch, T::QUERY_STAR_SET_SIZE, focus, fov);
                    Benchmark beta(s, focus, fov);
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
                    lu.log_trial({p.sigma_1, p.sigma_2, p.sigma_3, ss, static_cast<double> (r.size()),
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
        const char *const SCHEMA = "IdentificationMethod TEXT, Timestamp TEXT, Sigma1 FLOAT, Sigma2 FLOAT, "
            "Sigma3 FLOAT, Sigma4 FLOAT, ShiftDeviation FLOAT, CameraSensitivity FLOAT, ResultMatchesInput INT";
        
        bool is_correctly_identified (const Star::list &big_i, const Identification::labels_list &r_ell);
        
        /// Generic experiment function for the reduction trials. Performs a reduction trial and records the experiment
        /// in the lumberjack. The provided Nibble connection is used for generating the input image.
        ///
        /// @tparam T Identification class to perform reduction experiment with.
        /// @param ch Nibble connection used to generate the input image.
        /// @param lu Lumberjack connection used to record the results of each trial.
        /// @param cf Configuration reader holding all parameters to use.
        /// @param identifier Name of the identification method, in format (space): [angle, dot, sphere, plane,
        /// pyramid, composite].
        template <class T>
        void trial (Chomp &ch, Lumberjack &lu, INIReader &cf, const std::string &identifier) {
            Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
            double fov = cf.GetReal("hardware", "fov", 0);
            Star::list body;
            Star focus;
            
            // Define our hyperparameters and testing parameters.
            Identification::collect_parameters(p, cf, identifier), p.nu = std::make_shared<unsigned int>(0);
            int samples = cf.GetInteger("general-experiment", "samples", 0);
            int ss_iter = cf.GetInteger("reduction-experiment", "ss-iter", 0);
            int mb_iter = cf.GetInteger("reduction-experiment", "mb-iter", 0);
            double ss_step = cf.GetReal("reduction-experiment", "ss-step", 0);
            double mb_min = cf.GetReal("reduction-experiment", "mb-min", 0);
            double mb_step = cf.GetReal("reduction-experiment", "mb-step", 0);
            
            // First run is clean, without shifts. Following are the shift trials.
            for (int ss_i = 0; ss_i < ss_iter; ss_i++) {
                double ss = (ss_i == 0) ? 0 : pow(ss_step, ss_i);
                for (int mb_i = 0; mb_i < mb_iter; mb_i++) {
                    double mb = mb_min + static_cast<double>(mb_i) * mb_step;
                    
                    // Repeat each trial n = SAMPLES times.
                    for (int i = 0; i < samples; i++) {
                        present_benchmark(ch, body, focus, fov, mb);
                        Benchmark input(body, focus, fov);
                        input.shift_light(static_cast<signed> (body.size()), ss);
                        
                        // Perform a single trial.
                        Identification::labels_list w = T(input, p).reduce();
                        
                        // Log the results of our trial.
                        lu.log_trial({p.sigma_1, p.sigma_2, p.sigma_3, p.sigma_4, ss, mb,
                                         (is_correctly_identified(body, w) ? 1.0 : 0)});
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
        const char *const SCHEMA = "IdentificationMethod TEXT, Timestamp TEXT, Sigma1 FLOAT, Sigma2 FLOAT, "
            "Sigma3 FLOAT, Sigma4 FLOAT, ShiftDeviation FLOAT, CameraSensitivity FLOAT, FalseStars INT, "
            "ComparisonCount INT, PercentageCorrect FLOAT";
        
        double percentage_correct (const Star::list &big_i, const Star::list &b);
        
        /// Generic experiment function for the identification trials. Performs a identification trial and records the
        /// experiment in the lumberjack. The provided Nibble connection is used for generating the input image.
        ///
        /// @tparam T Identification class to perform identification experiment with.
        /// @param ch Nibble connection used to generate the input image.
        /// @param lu Lumberjack connection used to record the results of each trial.
        /// @param cf Configuration reader holding all parameters to use.
        /// @param identifier Name of the identification method, in format (space): [angle, dot, sphere, plane,
        /// pyramid, composite].
        template <class T>
        void trial (Chomp &ch, Lumberjack &lu, INIReader &cf, const std::string &identifier) {
            std::shared_ptr<unsigned int> nu = std::make_shared<unsigned int>(0);
            Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
            double fov = cf.GetReal("hardware", "fov", 0);
            Star::list body;
            Star focus;
            
            // Define our hyperparameters, and testing parameters.
            Identification::collect_parameters(p, cf, identifier), p.nu = nu;
            int samples = cf.GetInteger("general-experiment", "samples", 0);
            int ss_iter = cf.GetInteger("identification-experiment", "ss-iter", 0);
            int mb_iter = cf.GetInteger("identification-experiment", "mb-iter", 0);
            int es_iter = cf.GetInteger("identification-experiment", "es-iter", 0);
            double ss_step = cf.GetReal("identification-experiment", "ss-step", 0);
            double mb_min = cf.GetReal("identification-experiment", "mb-min", 0);
            double mb_step = cf.GetReal("identification-experiment", "mb-step", 0);
            double es_min = cf.GetReal("identification-experiment", "es-min", 0);
            double es_step = cf.GetReal("identification-experiment", "es-step", 0);
            
            for (int ss_i = 0; ss_i < ss_iter; ss_i++) {
                double ss = (ss_i == 0) ? 0 : pow(ss_step, ss_i);
                for (int mb_i = 0; mb_i < mb_iter; mb_i++) {
                    double mb = mb_min + static_cast<double>(mb_i) * mb_step;
                    for (int es_i = 0; es_i < es_iter; es_i++) {
                        double es = es_min + static_cast<double>(es_i) * es_step;
                        
                        // Repeat each trial n = SAMPLES times.
                        for (int i = 0; i < samples; i++) {
                            present_benchmark(ch, body, focus, fov, mb);
                            Benchmark input(body, focus, fov);
                            
                            // Append our error.
                            input.shift_light(static_cast<int> (body.size()), ss);
                            input.add_extra_light(static_cast<unsigned int> (es));
                            
                            // Perform a single trial.
                            Star::list w = T(input, p).identify();
                            
                            // Log the results of our trial.
                            lu.log_trial({p.sigma_1, p.sigma_2, p.sigma_3, p.sigma_4, ss, mb, es, static_cast<double>
                            (*nu), percentage_correct(body, w)});
                        }
                    }
                }
            }
        }
    }
}

#endif /* HOKU_EXPERIMENT_H */
