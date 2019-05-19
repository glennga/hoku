/// @file trial.h
/// @author Glenn Galvizo
///
/// Header file for all trials. This holds the namespaces of functions that allow us to test various methods and log
/// the data.

#ifndef HOKU_EXPERIMENT_H
#define HOKU_EXPERIMENT_H

#include <cmath>
#include <functional>
#include "third-party/cxxtimer/cxxtimer.hpp"

#include "benchmark/benchmark.h"
#include "identification/identification.h"
#include "experiment/lumberjack.h"

/// @brief Namespace that holds all namespaces, functions, and parameters used to conduct every experiment with.
namespace Experiment {
    struct Parameters { // Make your life easier, use the builder. (:
        std::string identifier, reference_table;
        double epsilon_1, epsilon_2, epsilon_3, epsilon_4;
        double m_bar, image_fov;
        unsigned int n_limit, nu_limit;

        unsigned int samples, extra_star_min, extra_star_step, remove_star_step;
        unsigned int shift_star_iter, extra_star_iter, remove_star_iter;
        double shift_star_step, remove_star_sigma;
    };

    class ParametersBuilder;

    /// @brief Namespace that holds all parameters and functions to conduct the query experiment with.
    namespace Query {
        template<class T>
        void trial (const std::shared_ptr<Chomp> &, const std::shared_ptr<Lumberjack> &,
                    const std::shared_ptr<Experiment::Parameters> &) {
            throw std::runtime_error("Not implemented.");
        }
    }

    /// @brief Namespace that holds all parameters and functions to conduct the reduction experiment with.
    namespace Reduction {
        template<class T>
        void trial (const std::shared_ptr<Chomp> &, const std::shared_ptr<Lumberjack> &,
                    const std::shared_ptr<Experiment::Parameters> &) {
            throw std::runtime_error("Not implemented.");
        }
    }

    /// @brief Namespace that holds all parameters and functions to conduct the identification experiment with.
    namespace Map {
        double percentage_correct (const Identification::StarsEither &b, const Star::list &answers, double fov);

        template<class T>
        void trial (const std::shared_ptr<Chomp> &ch, const std::shared_ptr<Lumberjack> &lu,
                    const std::shared_ptr<Experiment::Parameters> &ep) {
            Benchmark be = Benchmark::Builder()
                    .using_chomp(ch)
                    .limited_by_m(ep->m_bar)
                    .limited_by_n_stars(ep->n_limit)
                    .limited_by_fov(ep->image_fov)
                    .build();
            std::shared_ptr<T> identifier = Identification::Builder<T>()
                    .using_chomp(ch)
                    .given_image(std::make_shared<Benchmark>(be))
                    .using_epsilon_1(ep->epsilon_1)
                    .using_epsilon_2(ep->epsilon_2)
                    .using_epsilon_3(ep->epsilon_3)
                    .using_epsilon_4(ep->epsilon_4)
                    .limit_n_comparisons(ep->nu_limit)
                    .identified_by(ep->identifier)
                    .with_table(ep->reference_table)
                    .build();
            cxxtimer::Timer t(false);

            std::array<unsigned int, 3> iters = {ep->shift_star_iter, ep->extra_star_iter, ep->remove_star_iter};
            std::array<std::function<double(int j)>, 3> errors = {
                    [&] (int j) { return ((j == 0) ? 0 : j * ep->shift_star_step); },
                    [&] (int j) { return ep->extra_star_min + j * ep->extra_star_step; },
                    [&] (int j) { return ep->remove_star_step * j; }
            };
            std::array<std::function<void(double e)>, 3> error_consumers = {
                    [&] (double e) { be.shift_light(static_cast<signed>(be.get_image()->size()), e); },
                    [&] (double e) { be.add_extra_light(static_cast<signed>(e)); },
                    [&] (double e) { be.remove_light(static_cast<unsigned int>(e), ep->remove_star_sigma); }
            };

            for (int i = 0; i < 3; i++) {
                for (unsigned int j = 0; j < iters[i]; j++) {
                    double error = errors[i](j);

                    for (unsigned int k = 0; k < ep->samples; k++) {
                        be.generate_stars(ch, Benchmark::NO_N, ep->m_bar);
                        error_consumers[i](error);

                        t.start(); // Perform a single trial. Record it's duration.
                        Identification::StarsEither w = identifier->identify();
                        t.stop();

                        lu->log_trial({ep->epsilon_1, ep->epsilon_2, ep->epsilon_3, ep->epsilon_4,
                                       (i == 0) ? error : 0.0, (i == 1) ? error : 0.0, (i == 2) ? error : 0.0,
                                       static_cast<double>(identifier->get_nu()),
                                       static_cast<double>(t.count()),
                                       percentage_correct(w, *be.get_answers(), be.get_fov()),
                                       (w.error == Identification::NO_CONFIDENT_A_EITHER) ? 0.0 : 1.0
                        }), t.reset();
                    }
                }
            }
        }
    }
}

class Experiment::ParametersBuilder {
public:
    ParametersBuilder &prefixed_by (const std::string &name) {
        p.identifier = name;
        return *this;
    }
    ParametersBuilder &using_reference_table (const std::string &name) {
        p.reference_table = name;
        return *this;
    }
    ParametersBuilder &with_epsilon (const double e1, const double e2, const double e3, const double e4) {
        p.epsilon_1 = e1, p.epsilon_2 = e2, p.epsilon_3 = e3, p.epsilon_4 = e4;
        return *this;
    }
    ParametersBuilder &with_image_of_size (const double fov) {
        p.image_fov = fov;
        return *this;
    }
    ParametersBuilder &limited_by_n (const unsigned int n) {
        p.n_limit = n;
        return *this;
    }
    ParametersBuilder &limited_by_m (const double m) {
        p.m_bar = m;
        return *this;
    }
    ParametersBuilder &limited_by_nu (const unsigned int nu) {
        p.nu_limit = nu;
        return *this;
    }
    ParametersBuilder &repeated_for_n_times (const unsigned int samples) {
        p.samples = samples;
        return *this;
    }
    ParametersBuilder &with_n_shift_star_trials (const unsigned int ssi) {
        p.shift_star_iter = ssi;
        return *this;
    }
    ParametersBuilder &with_n_extra_star_trials (const unsigned int esi) {
        p.extra_star_iter = esi;
        return *this;
    }
    ParametersBuilder &with_n_remove_star_trials (const unsigned int rsi) {
        p.remove_star_iter = rsi;
        return *this;
    }
    ParametersBuilder &using_shift_star_parameters (const double step) {
        p.shift_star_step = step;
        return *this;
    }
    ParametersBuilder &using_extra_star_parameters (const unsigned int min, const unsigned int step) {
        p.extra_star_step = min, p.extra_star_step = step;
        return *this;
    }
    ParametersBuilder &using_remove_star_parameters (const unsigned int step, const double sigma) {
        p.remove_star_step = step, p.remove_star_sigma = sigma;
        return *this;
    }
    Parameters build () { return this->p; }

private:
    Parameters p;
};

#endif /* HOKU_EXPERIMENT_H */
