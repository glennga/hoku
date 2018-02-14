/// @file identification.cpp
/// @author Glenn Galvizo
///
/// Source file for Identification class, which holds all common data between all identification processes.

#include <numeric>
#include "third-party/inih/INIReader.h"

#include "benchmark/benchmark.h"
#include "identification/identification.h"

/// Returned when no candidates are found from a query.
const Identification::labels_list Identification::EMPTY_BIG_R = {-1, -1};

/// Returned when there exists no confident identity from an identification trial.
const Star::list Identification::NO_CONFIDENT_A = {Star::define_label(Star::zero(), -1)};

/// Returned when we count past our defined max nu from a crown or identification trial.
const Star::list Identification::EXCEEDED_NU_MAX = {Star::zero()};

/// Default parameters for a general identification object.
const Identification::Parameters Identification::DEFAULT_PARAMETERS = {DEFAULT_SIGMA_QUERY, DEFAULT_SQL_LIMIT,
    DEFAULT_PASS_R_SET_CARDINALITY, DEFAULT_FAVOR_BRIGHT_STARS, DEFAULT_SIGMA_OVERLAY, DEFAULT_NU_MAX, DEFAULT_NU,
    DEFAULT_F, DEFAULT_TABLE_NAME};

/// Constructor. We set our field-of-view to the default here.
Identification::Identification () {
    this->parameters = DEFAULT_PARAMETERS;
    this->fov = Benchmark::NO_FOV;
}

/// Given the reference to a Parameter struct and a configuration file reader, insert all the appropriate parameters
/// into the Parameter struct.
///
/// @param p Reference to the Parameter struct to update.
/// @param cf Reference to the configuration file reader to collect parameters from.
void Identification::collect_parameters (Parameters &p, INIReader &cf) {
    p.sigma_query = cf.GetReal("id-parameters", "sq", DEFAULT_SIGMA_QUERY);
    p.sql_limit = static_cast<unsigned>(cf.GetInteger("id-parameters", "sl", DEFAULT_SQL_LIMIT));
    p.pass_r_set_cardinality = cf.GetBoolean("id-parameters", "prsc", DEFAULT_PASS_R_SET_CARDINALITY);
    p.favor_bright_stars = cf.GetBoolean("id-parameters", "fbr", DEFAULT_FAVOR_BRIGHT_STARS);
    p.sigma_overlay = cf.GetReal("id-parameters", "so", DEFAULT_SIGMA_OVERLAY);
    p.nu_max = static_cast<unsigned>(cf.GetInteger("id-parameters", "nu-m", DEFAULT_NU_MAX));
    
    const std::array<std::string, 3> ws_id = {"TRIAD", "QUEST", "Q"};
    const std::array<Rotation::wahba_function, 3> ws = {Rotation::triad, Rotation::quest, Rotation::q_exact};
    
    // Determine the Wahba solver.
    std::string wabha_solver = cf.Get("id-parameters", "wbs", "TRIAD");
    for (unsigned int i = 0; i < ws_id.size(); i++) {
        if (ws_id.size() == wabha_solver.size() && std::equal(ws_id[i].begin(), ws_id[i].end(), wabha_solver.begin(),
                                                              [] (unsigned char a, unsigned char b) {
                                                                  return std::tolower(a) == std::tolower(b);
                                                              })) {
            p.f = ws[i];
            return;
        }
    }
}

/// Rotate every point the given rotation and check if the angle of separation between any two stars is within a
/// given limit sigma.
///
/// @param big_p All stars to check against the existing image body star set.
/// @param q The rotation to apply to all stars.
/// @return Set of matching stars found in candidates and the body sets.
Star::list Identification::find_positive_overlay (const Star::list &big_p, const Rotation &q) {
    double epsilon = 3.0 * this->parameters.sigma_overlay;
    Star::list m;
    m.reserve(big_i.size());
    
    for (const Star &p_i : big_p) {
        Star r_prime = Rotation::rotate(p_i, q);
        for (unsigned int i = 0; i < big_i.size(); i++) {
            if (Star::angle_between(r_prime, big_i[i]) < epsilon) {
                // Add this match to the list by noting the candidate star's catalog ID.
                m.emplace_back(Star(big_i[i][0], big_i[i][1], big_i[i][2], p_i.get_label()));
                
                // Remove the current star from the searching set. End the search for this star.
                big_i.erase(big_i.begin() + i);
                break;
            }
        }
    }
    
    return m;
}

/// Sort the given list of candidate label lists by their average brightness. Note: apparent magnitude has decreasing
/// scale, smaller = brighter.
///
/// @param big_r_ell Reference to a list of lists of catalog labels to sort.
void Identification::sort_brightness (std::vector<labels_list> &big_r_ell) {
    auto sum_m = [this] (const labels_list &k) -> double {
        return std::accumulate(k.begin(), k.end(), 0.0, [this] (const double &m_prev, const int &r_ell) -> double {
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
    Star::list b_d = identify(), inertial;
    
    // 'b_d' contains a list of labeled image stars. Determine the matching catalog stars.
    inertial.reserve(b_d.size());
    for (const Star &s : b_d) {
        inertial.push_back(ch.query_hip(s.get_label()));
    }
    
    // Return the result of applying the Wahba solver with the image and catalog stars.
    return parameters.f(b_d, inertial);
}

/// Extends the identification process by attempting to identify all stars in the image.
///
/// @return List of all identified stars.
Star::list Identification::identify_all () {
    // Perform the identification procedure.
    Star::list b_d = identify(), inertial;
    
    // 'b_d' contains a list of labeled image stars. Determine the matching catalog stars.
    inertial.reserve(b_d.size());
    for (const Star &s : b_d) {
        inertial.push_back(ch.query_hip(s.get_label()));
    }
    
    // Return the result of applying the Wahba solver and finding all matches with this.
    return find_positive_overlay(inertial, parameters.f(b_d, inertial));
}