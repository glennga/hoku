/// @file identification.cpp
/// @author Glenn Galvizo
///
/// Source file for Identification class, which holds all common data between all identification processes.

#include <numeric>

#include "math/random-draw.h"
#include "benchmark/benchmark.h"
#include "identification/identification.h"

/// Error returned when no candidates are found from a query.
const int Identification::NO_CONFIDENT_R_EITHER = -3;

/// Error returned when there exists no confident identity from an identification trial.
const int Identification::NO_CONFIDENT_A_EITHER = -2;

/// Error returned when we count past our defined max nu from an identification trial.
const int Identification::EXCEEDED_NU_MAX_EITHER = -1;

/// Indicates that a table already exists upon a table creation attempt.
const int Identification::TABLE_ALREADY_EXISTS = -1;

/// Constructor. We set our field-of-view to the default here.
Identification::Identification () {
    this->parameters = std::make_unique<Identification::Parameters>(Identification::Parameters());

    this->fov = Benchmark::NO_FOV;
}

/// Given a configuration file reader, return all the appropriate parameters into a Parameter struct.
///
/// @param cf Reference to the configuration file reader to use.
/// @param identifier Name of the identifier to collect the query sigmas from.
Identification::Parameters Identification::collect_parameters (INIReader &cf, const std::string &identifier) {
    Parameters p;
    p.sigma_1 = cf.GetReal("query-sigma", identifier + "-1", 0);
    p.sigma_2 = cf.GetReal("query-sigma", identifier + "-2", 0);
    p.sigma_3 = cf.GetReal("query-sigma", identifier + "-3", 0);
    p.sigma_4 = cf.GetReal("id-parameters", "so", std::numeric_limits<double>::epsilon() * 10000);
    p.table_name = cf.Get("table-names", identifier, "DEFAULT_TABLE_NAME");
    p.sql_limit = static_cast<unsigned>(cf.GetInteger("id-parameters", "sl", static_cast<long>(500)));
    p.no_reduction = cf.GetBoolean("id-parameters", "nr", false);
    p.favor_bright_stars = cf.GetBoolean("id-parameters", "fbr", false);
    p.nu_max = static_cast<unsigned>(cf.GetInteger("id-parameters", "nu-m", static_cast<long> (50000)));
    p.nu = std::make_shared<unsigned> (0);

    const std::array<std::string, 3> ws_id = {"TRIAD", "Q", "SVD"};
    const std::array<Rotation::wahba_function, 3> ws = {Rotation::triad, Rotation::q_method, Rotation::svd};

    // Determine the Wahba solver. This is case-insensitive.
    std::string wabha_solver = cf.Get("id-parameters", "wbs", "TRIAD");
    for (unsigned int i = 0; i < ws_id.size(); i++) {
        if (ws_id.size() == wabha_solver.size() && std::equal(ws_id[i].begin(), ws_id[i].end(), wabha_solver.begin(),
                                                              [] (unsigned char a, unsigned char b) {
                                                                  return std::tolower(a) == std::tolower(b);
                                                              })) {
            p.f = ws[i];
            break;
        }
    }

    return p;
}

/// Overloaded FPO implementation. Rotate every point with the given rotation and check if the angle of separation
/// between any two stars is within a given limit sigma. Record which stars (by index) pass the test in the I vector.
/// Note that this assumes that the shuffling on the other is **disabled**.
///
/// @param big_p All stars to check against the existing image body star set.
/// @param q The rotation to apply to all stars.
/// @param i Reference to the index vector to record which stars were saved.
/// @return Set of matching stars found in candidates and the body sets.
Star::list Identification::find_positive_overlay (const Star::list &big_p, const Rotation &q, std::vector<int> &i) {
    double epsilon = 3.0 * this->parameters->sigma_4;
    Star::list big_p_c = big_p;
    Star::list m;

    // Clear our index vector.
    i.clear();
    i.reserve(this->big_i->size()), m.reserve(big_i->size());

    std::shuffle(big_p_c.begin(), big_p_c.end(), RandomDraw::mersenne_twister);
    std::for_each(big_p_c.begin(), big_p_c.end(), [&] (const Star &p_i) -> void {
        Star r_prime = Rotation::rotate(p_i, q);

        for (unsigned int j = 0; j < big_i->size(); j++) {
            if (Star::within_angle(r_prime, (*big_i)[j], epsilon)) {
                // Add this match to the list by noting the candidate star's catalog ID.
                m.emplace_back(Star((*big_i)[j][0], (*big_i)[j][1], (*big_i)[j][2], p_i.get_label())),
                        i.emplace_back(j);

                break;
            }
        }
    });

    return m;
}

/// Rotate every point with the given rotation and check if the angle of separation between any two stars is within a
/// given limit sigma.
///
/// @param big_p All stars to check against the existing image body star set.
/// @param q The rotation to apply to all stars.
/// @return Set of matching stars found in candidates and the body sets.
Star::list Identification::find_positive_overlay (const Star::list &big_p, const Rotation &q) {
    double epsilon = 3.0 * this->parameters->sigma_4;
    Star::list big_p_c = big_p;
    Star::list m;

    std::shuffle(big_p_c.begin(), big_p_c.end(), RandomDraw::mersenne_twister);
    std::for_each(big_p_c.begin(), big_p_c.end(), [&] (const Star &p_i) -> void {
        Star r_prime = Rotation::rotate(p_i, q);

        for (const Star &i_i : *big_i) {
            if (Star::within_angle(r_prime, i_i, epsilon)) {
                m.emplace_back(Star(i_i[0], i_i[1], i_i[2], r_prime.get_label()));

                // Break early, we have found our match.
                break;
            }
        }
    });

    return m;
}

/// Sort the given list of candidate label lists by their average brightness. Note: apparent magnitude has decreasing
/// scale, smaller = brighter.
///
/// @param big_r_ell Reference to a list of lists of catalog labels to sort.
void Identification::sort_brightness (std::vector<labels_list> &big_r_ell) {
    auto sum_m = [this] (const labels_list &k) -> double {
        return std::accumulate(k.begin(), k.end(), 0.0f, [this] (const double &m_prev, const int &r_ell) -> double {
            return m_prev + this->ch.query_hip(r_ell).get_magnitude();
        });
    };

    std::sort(big_r_ell.begin(), big_r_ell.end(), [&sum_m] (const labels_list &r_ell_i, const labels_list &r_ell_j) {
        return sum_m(r_ell_i) / r_ell_i.size() < sum_m(r_ell_j) / r_ell_j.size();
    });
}

/// Extends the identification process by determining an attitude given the identity results, and a solution to Wahba's
/// problem.
///
/// @return The rotation from the R set to the B set.
Rotation Identification::align () {
    // Perform the identification procedure.
    Star::list b_d = identify().result, inertial;

    // 'b_d' contains a list of labeled image stars. Determine the matching catalog stars.
    inertial.reserve(b_d.size());
    for (const Star &s : b_d) {
        inertial.push_back(ch.query_hip(s.get_label()));
    }

    // Return the result of applying the Wahba solver with the image and catalog stars.
    return parameters->f(b_d, inertial);
}

/// Extends the identification process by attempting to identify all stars in the image.
///
/// @return List of all identified stars.
Star::list Identification::identify_all () {
    // Perform the identification procedure.
    Star::list b_d = identify().result, inertial;

    // 'b_d' contains a list of labeled image stars. Determine the matching catalog stars.
    inertial.reserve(b_d.size());
    for (const Star &s : b_d) {
        inertial.push_back(ch.query_hip(s.get_label()));
    }

    // Return the result of applying the Wahba solver and finding all matches with this.
    return find_positive_overlay(inertial, parameters->f(b_d, inertial));
}
