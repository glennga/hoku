/// @file trial.h
/// @author Glenn Galvizo
///
/// Header file for all trials. This holds the namespaces of functions that allow us to test various
/// methods and log the data.

#ifndef HOKU_EXPERIMENT_H
#define HOKU_EXPERIMENT_H

#include "identification/identification.h"
#include "experiment/lumberjack.h"

/// This namespace holds all namespaces and functions used to conduct every experiment with.
namespace Experiment {
    const double WORKING_FOV = 20; ///< Field of view that all our test stars must be within.
    const int SAMPLES = 10; ///< Number of samples to retrieve for each individual trial.
    
    void present_benchmark (Chomp &, Star::list &, Star &, double = 0);
    
    /// The query experiment is used to characterize the choose query stars and search catalog steps.
    namespace Query {
        /// Schema comma separated string that corresponds to the creation of the query table.
        const char *const SCHEMA = "IdentificationMethod TEXT, Timestamp TEXT, SigmaQuery FLOAT, ShiftDeviation FLOAT, "
            "CandidateSetSize FLOAT, SExistence INT";
        
        const double SQ_MIN = std::numeric_limits<double>::epsilon() * pow(3, 5); ///< Minimum query sigma.
        const double SS_MULT = 0.00000001; ///< Shift sigma multiplier for each variation.
        const int SS_ITER = 5; ///< Number of shift sigma variations.
        
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
            Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
            Star focus;
            
            p.sigma_query = Query::SQ_MIN, p.table_name = table_name;
            for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
                double ss = (ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i);
                
                // Repeat each trial n = SAMPLES times.
                for (int i = 0; i < SAMPLES; i++) {
                    Star::list s = generate_n_stars(ch, T::QUERY_STAR_SET_SIZE, focus);
                    Benchmark beta(s, focus, WORKING_FOV);
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
                    lu.log_trial({SQ_MIN, ss, static_cast<double> (r.size()), (set_existence(r, w) ? 1.0 : 0)});
                }
            }
        }
    }
    
    /// The reduction experiment is used to characterize the choose query stars step to our attitude determination step.
    namespace Reduction {
        /// Schema header that corresponds to the log file for all reduction trials.
        const char *const SCHEMA = "IdentificationMethod TEXT, Timestamp TEXT, SigmaQuery FLOAT, SigmaOverlay FLOAT, "
            "ShiftDeviation FLOAT, CameraSensitivity FLOAT, ResultMatchesInput INT";
        
        const double SQ_MIN = std::numeric_limits<double>::epsilon() * pow(3, 5); ///< Minimum query sigma.
        const double SO_MIN = std::numeric_limits<double>::epsilon() * 3; ///< Minimum match sigma.
        const double SS_MULT = 0.00000001; ///< Shift sigma multiplier for each variation.
        const int SS_ITER = 5; ///< Number of shift sigma variations.
        
        const double MB_MIN = 6.0; ///< Minimum magnitude bound.
        const double MB_STEP = 0.25; ///< Step to increment magnitude bound with for each variation.
        const int MB_ITER = 5; ///< Number of magnitude bound variations.
        
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
                    for (int i = 0; i < SAMPLES; i++) {
                        present_benchmark(ch, body, focus, MB_MIN + mb_i * MB_STEP);
                        Benchmark input(body, focus, WORKING_FOV);
                        input.shift_light(static_cast<signed> (body.size()),
                                          ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)));
                        
                        // Perform a single trial.
                        Identification::labels_list w = T(input, p).reduction();
                        
                        // Log the results of our trial.
                        lu.log_trial({Reduction::SQ_MIN, Reduction::SO_MIN, ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)),
                                         MB_MIN + mb_i * MB_STEP, (is_correctly_identified(body, w) ? 1.0 : 0)});
                    }
                }
            }
        }
    }
    
    /// The alignment experiment is used to characterize each identifier from start to alignment determination.
    namespace Alignment {
        /// Schema header that corresponds to the log file for all alignment trials.
        const char *const SCHEMA = "IdentificationMethod TEXT, Timestamp TEXT, SigmaQuery FLOAT, SigmaOverlay FLOAT, "
            "ShiftDeviation FLOAT, CameraSensitivity FLOAT, FalseStars INT, ComparisonCount INT, "
            "IsCorrectlyAligned INT";
        
        const double SQ_MIN = std::numeric_limits<double>::epsilon() * pow(3, 5); ///< Minimum query sigma.
        const double SO_MIN = std::numeric_limits<double>::epsilon() * 3; ///< Minimum match sigma.
        const double SS_MULT = 0.00000001; ///< Shift sigma multiplier for each variation.
        const int SS_ITER = 5; ///< Number of shift sigma variations.
        
        const double MB_MIN = 6.0; ///< Minimum magnitude bound.
        const double MB_STEP = 0.25; ///< Step to increment magnitude bound with for each variation.
        const int MB_ITER = 5; ///< Number of magnitude bound variations.
        
        const int ES_MIN = 0; ///< Minimum number of extra stars to add.
        const int ES_STEP = 5; ///< Step to increment extra stars with.
        const int ES_ITER = 5;  ///< Number of extra stars variations.
        
        bool is_correctly_aligned (const Star::list &body, const Star::list &w);
        
        /// Generic experiment function for the alignment trials. Performs a alignment trial and records the experiment
        /// in the lumberjack. The provided Nibble connection is used for generating the input image.
        ///
        /// @tparam T Identification class to perform alignment experiment with.
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
            p.nu_max = 20000, p.sigma_overlay = Alignment::SO_MIN, p.sigma_query = Alignment::SQ_MIN;
            p.table_name = table_name, p.nu = std::make_shared<unsigned int>(nu);
            
            for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
                double ss = ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i));
                for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
                    for (int es_i = 0; es_i < ES_ITER; es_i++) {
                        
                        // Repeat each trial n = SAMPLES times.
                        for (int i = 0; i < SAMPLES; i++) {
                            present_benchmark(ch, body, focus, MB_MIN + mb_i * MB_STEP);
                            Benchmark input(body, focus, WORKING_FOV);
                            
                            // Append our error.
                            input.shift_light(static_cast<int> (body.size()), ss);
                            double e = ES_MIN + ES_STEP * es_i, clean_size = body.size();
                            input.add_extra_light(static_cast<unsigned int> ((e / (1 - e)) * clean_size));
                            
                            // Perform a single trial.
                            Star::list w = T(input, p).align();
                            
                            // Log the results of our trial.
                            lu.log_trial({Alignment::SQ_MIN, Alignment::SO_MIN, ss, MB_MIN + mb_i * MB_STEP, e,
                                             static_cast<double> (*p.nu), (is_correctly_aligned(body, w) ? 1.0 : 0)});
                        }
                    }
                }
            }
        }
    }
}

#endif /* HOKU_EXPERIMENT_H */
