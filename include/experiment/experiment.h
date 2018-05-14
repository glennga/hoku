/// @file trial.h
/// @author Glenn Galvizo
///
/// Header file for all trials. This holds the namespaces of functions that allow us to test various
/// methods and log the data.

#ifndef HOKU_EXPERIMENT_H
#define HOKU_EXPERIMENT_H

#include <cmath>
#include "third-party/inih/INIReader.h"
#include "third-party/cxxtimer/cxxtimer.hpp"

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
    void present_benchmark (Chomp &ch, Star::list &big_i, Star::list &big_c, Star &center, double fov,
                            double m_bar = 6.0);

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
        /// @param cf Configuration reader holding all parameters to use.
        /// @param identifier Name of the identification method, in format (space): [angle, dot, sphere, plane,
        /// pyramid, composite].
        template<class T>
        void trial (Chomp &ch, Lumberjack &lu, INIReader &cf, const std::string &identifier) {
            Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
            double fov = cf.GetReal("hardware", "fov", 0);
            Star focus;

            // Define our hyperparameters and testing parameters.
            Identification::collect_parameters(p, cf, identifier);
            int samples = static_cast<int>(cf.GetInteger("general-experiment", "samples", 0));
            int ss_iter = static_cast<int>(cf.GetInteger("query-experiment", "ss-iter", 0));
            double ss_step = cf.GetReal("query-experiment", "ss-step", 0);

            for (int ss_i = 0; ss_i < ss_iter; ss_i++) {
                double ss = (ss_i == 0) ? 0 : 1.0 / pow(ss_step, ss_i - 1);

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
                                   "Sigma3 FLOAT, ShiftDeviation FLOAT, FalseStars INT, ComparisonCount INT, "
                                   "TimeToResult FLOAT, PercentageCorrect FLOAT";

        double percentage_correct (const Star::list &big_i, const Star::list &r);

        /// Generic experiment function for the reduction trials. Performs a reduction trial and records the experiment
        /// in the lumberjack. The provided Nibble connection is used for generating the input image.
        ///
        /// @tparam T Identification class to perform reduction experiment with.
        /// @param ch Nibble connection used to generate the input image.
        /// @param lu Lumberjack connection used to record the results of each trial.
        /// @param cf Configuration reader holding all parameters to use.
        /// @param identifier Name of the identification method, in format (space): [angle, dot, sphere, plane,
        /// pyramid, composite].
        template<class T>
        void trial (Chomp &ch, Lumberjack &lu, INIReader &cf, const std::string &identifier) {
            std::shared_ptr<unsigned int> nu = std::make_shared<unsigned int>(0);
            Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
            double fov = cf.GetReal("hardware", "fov", 0);
            cxxtimer::Timer t(false);
            Star::list big_i, big_c;
            Star focus;

            // Define our hyperparameters and testing parameters.
            Identification::collect_parameters(p, cf, identifier), p.nu = nu;
            int samples = static_cast<int>(cf.GetInteger("general-experiment", "samples", 0));
            int ss_iter = static_cast<int>(cf.GetInteger("reduction-experiment", "ss-iter", 0));
            int es_iter = static_cast<int>(cf.GetInteger("reduction-experiment", "es-iter", 0));
            double ss_step = cf.GetReal("reduction-experiment", "ss-step", 0);
            double es_min = cf.GetReal("reduction-experiment", "es-min", 0);
            double es_step = cf.GetReal("reduction-experiment", "es-step", 0);

            // Perform the shift trials, then the extra stars trial.
            auto trial_specific_error = [&] (const bool is_shift, const int iter, const double step, const double min) {
                for (int s_i = 0; s_i < iter; s_i++) {
                    double s = is_shift ? ((s_i == 0) ? 0 : 1.0 / pow(step, s_i - 1)) :
                               (min + static_cast<double>(s_i) * step);

                    // Repeat each trial n = SAMPLES times.
                    for (int i = 0; i < samples; i++) {
                        present_benchmark(ch, big_i, big_c, focus, fov);
                        Benchmark input(big_i, focus, fov);
                        (is_shift) ? input.shift_light(static_cast<signed> (big_i.size()), s)
                                   : input.add_extra_light(
                                static_cast<unsigned int> (s));

                        // Perform a single trial. Record it's duration.
                        t.start();
                        Star::list w = T(input, p).reduce();
                        t.stop();

                        // Log the results of our trial, and reset our timer.
                        lu.log_trial({p.sigma_1, p.sigma_2, p.sigma_3, is_shift ? s : 0, !is_shift ? s : es_min,
                                      static_cast<double>(*nu),
                                      static_cast<double>(t.count()), percentage_correct(big_c, w)}), t.reset();
                    }
                }
            };
            trial_specific_error(true, ss_iter, ss_step, 0), trial_specific_error(false, es_iter, es_step, es_min);
        }
    }

    /// @brief Namespace that holds all parameters and functions to conduct the identification experiment with.
    ///
    /// The identification experiment is used to characterize each identifier from start to identification.
    namespace Map {
        /// Schema header that corresponds to the log file for all identification trials.
        const char *const SCHEMA = "IdentificationMethod TEXT, Timestamp TEXT, Sigma1 FLOAT, Sigma2 FLOAT, "
                                   "Sigma3 FLOAT, Sigma4 FLOAT, ShiftDeviation FLOAT, FalseStars INT, "
                                   "ComparisonCount INT, TimeToResult FLOAT, PercentageCorrect FLOAT";

        double percentage_correct (const Star::list &big_i, const Star::list &b, double fov);

        /// Generic experiment function for the identification trials. Performs a identification trial and records the
        /// experiment in the lumberjack. The provided Nibble connection is used for generating the input image.
        ///
        /// @tparam T Identification class to perform identification experiment with.
        /// @param ch Nibble connection used to generate the input image.
        /// @param lu Lumberjack connection used to record the results of each trial.
        /// @param cf Configuration reader holding all parameters to use.
        /// @param identifier Name of the identification method, in format (space): [angle, dot, sphere, plane,
        /// pyramid, composite].
        template<class T>
        void trial (Chomp &ch, Lumberjack &lu, INIReader &cf, const std::string &identifier) {
            std::shared_ptr<unsigned int> nu = std::make_shared<unsigned int>(0);
            Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
            double fov = cf.GetReal("hardware", "fov", 0);
            cxxtimer::Timer t(false);
            Star::list big_i, big_c;
            Star focus;

            // Define our hyperparameters and testing parameters.
            Identification::collect_parameters(p, cf, identifier), p.nu = nu;
            int samples = static_cast<int>(cf.GetInteger("general-experiment", "samples", 0));
            int ss_iter = static_cast<int>(cf.GetInteger("identification-experiment", "ss-iter", 0));
            int es_iter = static_cast<int>(cf.GetInteger("identification-experiment", "es-iter", 0));
            double ss_step = cf.GetReal("identification-experiment", "ss-step", 0);
            double es_min = cf.GetReal("identification-experiment", "es-min", 0);
            double es_step = cf.GetReal("identification-experiment", "es-step", 0);

            // Perform the shift trials, then the extra stars trial.
            auto trial_specific_error = [&] (const bool is_shift, const int iter, const double step, const double min) {
                for (int s_i = 0; s_i < iter; s_i++) {
                    double s = is_shift ? ((s_i == 0) ? 0 : 1.0 / pow(step, s_i - 1)) :
                               (min + static_cast<double>(s_i) * step);

                    // Repeat each trial n = SAMPLES times.
                    for (int i = 0; i < samples; i++) {
                        present_benchmark(ch, big_i, big_c, focus, fov);
                        Benchmark input(big_i, focus, fov);
                        (is_shift) ? input.shift_light(static_cast<signed> (big_i.size()), s)
                                   : input.add_extra_light(static_cast<unsigned int> (s));

                        // Perform a single trial. Record it's duration.
                        t.start();
                        Star::list w = T(input, p).identify();
                        t.stop();

                        // Log the results of our trial.
                        lu.log_trial(
                                {p.sigma_1, p.sigma_2, p.sigma_3, p.sigma_4, is_shift ? s : 0, !is_shift ? s : es_min,
                                 static_cast<double>(*nu), static_cast<double>(t.count()),
                                 percentage_correct(big_i, w, fov)}), t.reset();
                    }
                }
            };
            trial_specific_error(true, ss_iter, ss_step, 0), trial_specific_error(false, es_iter, es_step, es_min);
        }
    }

    /// @brief Namespace that holds all parameters and functions to conduct the Overlay experiment with.
    ///
    /// The overlay experiment is used to characterize the direct match test process.
    namespace Overlay {
        /// Schema comma separated string that corresponds to the creation of the DMT table.
        const char *const SCHEMA = "IdentificationMethod TEXT, Timestamp TEXT, Sigma4 FLOAT, ShiftDeviation FLOAT, "
                                   "FalseStars INT, TruePositive INT, FalsePositive INT, TrueNegative INT, "
                                   "FalseNegative INT, N INT";

        std::array<double, 5> confusion_matrix (const Star::list &big_i_prime, const Star::list &big_i,
                                                const std::vector<int> &big_i_i, double es);

        /// Experiment function for the DMT trials. Performs a overlay trial and records the experiment in the
        /// lumberjack. The provided Nibble connection is used for generating the input image.
        ///
        /// @tparam T Identification class to perform overlay trials with. Used only as a wrapper for FPO method.
        /// @param ch Nibble connection used to generate the input image.
        /// @param lu Lumberjack connection used to record the results of each trial.
        /// @param cf Configuration reader holding all parameters to use.
        /// @param identifier Name of the identification method, in format (space): [angle, dot, sphere, plane,
        /// pyramid, composite].
        template<class T>
        void trial (Chomp &ch, Lumberjack &lu, INIReader &cf, const std::string &identifier) {
            Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
            double fov = cf.GetReal("hardware", "fov", 0);
            std::vector<int> big_i_i;
            Star::list big_i, big_c;
            Star focus;

            // Define our hyperparameters and testing parameters.
            Identification::collect_parameters(p, cf, identifier);
            int samples = static_cast<int>(cf.GetInteger("general-experiment", "samples", 0));
            int ss_iter = static_cast<int>(cf.GetInteger("overlay-experiment", "ss-iter", 0));
            int es_iter = static_cast<int>(cf.GetInteger("overlay-experiment", "es-iter", 0));
            double ss_step = cf.GetReal("overlay-experiment", "ss-step", 0);
            double es_min = cf.GetReal("overlay-experiment", "es-min", 0);
            double es_step = cf.GetReal("overlay-experiment", "es-step", 0);

            // Perform the shift trials, then the extra stars trial.
            auto trial_specific_error = [&] (const bool is_shift, const int iter, const double step, const double min) {
                for (int s_i = 0; s_i < iter; s_i++) {
                    double s = is_shift ? ((s_i == 0) ? 0 : 1.0 / pow(step, s_i - 1)) :
                               (min + static_cast<double>(s_i) * step);

                    // Repeat each trial n = SAMPLES times.
                    for (int i = 0; i < samples; i++) {
                        present_benchmark(ch, big_i, big_c, focus, fov);
                        Benchmark input(big_i, focus, fov);
                        (is_shift) ? input.shift_light(static_cast<signed> (big_i.size()), s, false)
                                   : input.add_extra_light(static_cast<unsigned int> (s), false);

                        // Perform a single trial.
                        Star::list w = T(input, p).find_positive_overlay(big_c, p.f({big_i[0], big_i[1]},
                                                                                    {big_c[0], big_c[1]}), big_i_i);

                        // Log the results of our trial.
                        std::array<double, 5> c = confusion_matrix(w, big_i, big_i_i, (is_shift) ? es_min : s);
                        lu.log_trial(
                                {p.sigma_4, is_shift ? s : 0, !is_shift ? s : es_min, c[0], c[1], c[2], c[3], c[4]});
                    }
                }
            };
            trial_specific_error(true, ss_iter, ss_step, 0), trial_specific_error(false, es_iter, es_step, es_min);
        }
    }
}

#endif /* HOKU_EXPERIMENT_H */
