/// @file identification.cpp
/// @author Glenn Galvizo
///
/// Source file for Identification class, which holds all common data between all identification processes.

#include <numeric>

#include "benchmark/benchmark.h"
#include "identification/identification.h"

/// Default sigma query for all identification methods.
const double Identification::DEFAULT_SIGMA_QUERY = std::numeric_limits<double>::epsilon() * 10000;

/// Default SQL limit for all identification methods.
const unsigned int Identification::DEFAULT_SQL_LIMIT = 500;

/// Default no reduction flag for all identification methods.
const bool Identification::DEFAULT_NO_REDUCTION = false;

/// Default favor bright stars flag for all identification methods.
const bool Identification::DEFAULT_FAVOR_BRIGHT_STARS = false;

/// Default sigma overlay (for matching) for all identification methods.
const double Identification::DEFAULT_SIGMA_4 = std::numeric_limits<double>::epsilon() * 10000;

/// Default nu max (comparison counts) for all identification methods.
const unsigned int Identification::DEFAULT_NU_MAX = 50000;

/// Default pointer to nu (comparison count) for all identification methods.
const std::shared_ptr<unsigned int> Identification::DEFAULT_NU = nullptr;

/// Default solution to Wahba's problem for all identification methods.
const Rotation::wahba_function Identification::DEFAULT_F = Rotation::triad;

/// Default table name for all identification methods.
const char *Identification::DEFAULT_TABLE_NAME = "NO_TABLE";

/// Returned when no candidates are found from a query.
const Star::list Identification::NO_CONFIDENT_R = {Star::wrap(Vector3::Zero(), -2)};

/// Returned when there exists no confident identity from an identification trial.
const Star::list Identification::NO_CONFIDENT_A = {Star::wrap(Vector3::Zero(), -1)};

/// Returned when we count past our defined max nu from a crown or identification trial.
const Star::list Identification::EXCEEDED_NU_MAX = {Star::wrap(Vector3::Zero())};

/// Indicates that a table already exists upon a table creation attempt.
const int Identification::TABLE_ALREADY_EXISTS = -1;

/// Default parameters for a general identification object.
const Identification::Parameters Identification::DEFAULT_PARAMETERS = {DEFAULT_SIGMA_QUERY, DEFAULT_SIGMA_QUERY,
    DEFAULT_SIGMA_QUERY, DEFAULT_SIGMA_4, DEFAULT_SQL_LIMIT, DEFAULT_NO_REDUCTION, DEFAULT_FAVOR_BRIGHT_STARS,
    DEFAULT_NU_MAX, DEFAULT_NU, DEFAULT_F, DEFAULT_TABLE_NAME};

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
/// @param identifier Name of the identifier to collect the query sigmas from.
void Identification::collect_parameters (Parameters &p, INIReader &cf, const std::string &identifier) {
    p.sigma_1 = cf.GetReal("query-sigma", identifier + "-1", 0);
    p.sigma_2 = cf.GetReal("query-sigma", identifier + "-2", 0);
    p.sigma_3 = cf.GetReal("query-sigma", identifier + "-3", 0);
    p.sigma_4 = cf.GetReal("id-parameters", "so", DEFAULT_SIGMA_4);
    p.table_name = cf.Get("table-names", identifier, DEFAULT_TABLE_NAME);
    p.sql_limit = static_cast<unsigned>(cf.GetInteger("id-parameters", "sl", static_cast<long>(DEFAULT_SQL_LIMIT)));
    p.no_reduction = cf.GetBoolean("id-parameters", "nr", DEFAULT_NO_REDUCTION);
    p.favor_bright_stars = cf.GetBoolean("id-parameters", "fbr", DEFAULT_FAVOR_BRIGHT_STARS);
    p.nu_max = static_cast<unsigned>(cf.GetInteger("id-parameters", "nu-m", static_cast<long> (DEFAULT_NU_MAX)));
    
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
            return;
        }
    }
}

/// Overloaded FPO implementation. Rotate every point with the given rotation and check if the angle of separation
/// between any two stars is within a given limit sigma. Record which stars (by index) pass the test in the I vector.
/// Note that this assumes that the shuffling on the other is **disabled**.
///
/// @param big_p All stars to check against the existing image body star set.
/// @param q The rotation to apply to all stars.
/// @param i Reference to the index vector to record which stars were saved.
/// @return Set of matching stars found in candidates and the body sets.
Star::list Identification::find_positive_overlay(const Star::list &big_p, const Rotation &q, std::vector<int> &i) {
    double epsilon = 3.0 * this->parameters.sigma_4;
    Star::list m;
    
    // Clear our index vector.
    i.clear();
    i.reserve(this->big_i.size()), m.reserve(big_i.size());
    
    std::for_each(big_p.begin(), big_p.end(), [&] (const Star &p_i) -> void {
        Star r_prime = Rotation::rotate(p_i, q);
    
        for (unsigned int j = 0; j < big_i.size(); j++) {
            // Avoid stars that have been added.
            if (std::find(i.begin(), i.end(), j) == i.end() && Star::within_angle(r_prime, big_i[j], epsilon)) {
                // Add this match to the list by noting the candidate star's catalog ID.
                m.emplace_back(Star(big_i[j][0], big_i[j][1], big_i[j][2], p_i.get_label())), i.emplace_back(j);
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
    double epsilon = 3.0 * this->parameters.sigma_4;
    Star::list m, big_i_c = this->big_i;
    m.reserve(big_i.size());
    
    for (const Star &p_i : big_p) {
        Star r_prime = Rotation::rotate(p_i, q);
        
        for (unsigned int i = 0; i < big_i_c.size(); i++) {
            if (Star::within_angle(r_prime, big_i_c[i], epsilon)) {
                // Add this match to the list by noting the candidate star's catalog ID.
                m.emplace_back(Star(big_i_c[i][0], big_i_c[i][1], big_i_c[i][2], p_i.get_label()));
                
                // Remove the current star from the searching set. End the search for this star.
                big_i_c.erase(big_i_c.begin() + i);
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