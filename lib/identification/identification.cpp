/// @file identification.cpp
/// @author Glenn Galvizo
///
/// Source file for Identification class, which holds all common data between all identification processes.

#include "third-party/inih/INIReader.h"

#include "benchmark/benchmark.h"
#include "identification/identification.h"

/// Returned when no candidates are found from a query.
const Identification::labels_list Identification::NO_CANDIDATES_FOUND = {-1, -1};

/// Returned when there exists no confident identity from an identification trial.
const Star::list Identification::NO_CONFIDENT_IDENTITY = {Star::define_label(Star::zero(), -1)};

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
    std::string wabha_solver = cf.Get("id-parameters", "wbs", "");
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
/// @param candidates All stars to check against the body star set.
/// @param q The rotation to apply to all stars.
/// @return Set of matching stars found in candidates and the body sets.
Star::list Identification::find_matches (const Star::list &candidates, const Rotation &q) {
    Star::list matches, non_matched = this->input;
    double epsilon = 3.0 * this->parameters.sigma_overlay;
    matches.reserve(this->input.size());
    
    for (const Star &candidate : candidates) {
        Star r_prime = Rotation::rotate(candidate, q);
        for (unsigned int i = 0; i < non_matched.size(); i++) {
            if (Star::angle_between(r_prime, non_matched[i]) < epsilon) {
                // Add this match to the list by noting the candidate star's catalog ID.
                matches.emplace_back(
                    Star(non_matched[i][0], non_matched[i][1], non_matched[i][2], candidate.get_label()));
                
                // Remove the current star from the searching set. End the search for this star.
                non_matched.erase(non_matched.begin() + i);
                break;
            }
        }
    }
    
    return matches;
}

/// Sort the given list of candidate label lists by their average brightness.
///
/// @param candidates Reference to a list of lists of catalog labels to sort.
void Identification::sort_brightness (std::vector<labels_list> &candidates) {
    auto grab_b = [this] (const int ell) -> double {
        return this->ch.query_hip(ell).get_magnitude();
    };
    
    std::sort(candidates.begin(), candidates.end(), [&grab_b] (const labels_list &i, const labels_list &j) {
        return (grab_b(i[0]) + grab_b(i[1]) + grab_b(i[2])) > (grab_b(j[0]) + grab_b(j[1]) + grab_b(j[2]));
    });
}

/// Extends the identification process by determining an attitude given the identity results, and a solution to Wahba's
/// problem.
///
/// @return The rotation from the R set to the B set.
Rotation Identification::align () {
    // Perform the identification procedure.
    Star::list a = identify(), inertial;
    
    // 'a' contains a list of labeled image stars. Determine the matching catalog stars.
    inertial.reserve(a.size());
    for (const Star &s : a) {
        inertial.push_back(ch.query_hip(s.get_label()));
    }
    
    // Return the result of applying the Wahba solver with the image and catalog stars.
    return parameters.f(inertial, a);
}

/// Extends the identification process by attempting to identify all stars in the image.
///
/// @return List of all identified stars.
Star::list Identification::identify_all () {
    // Perform the identification procedure.
    Star::list a = identify(), inertial;
    
    // 'a' contains a list of labeled image stars. Determine the matching catalog stars.
    inertial.reserve(a.size());
    for (const Star &s : a) {
        inertial.push_back(ch.query_hip(s.get_label()));
    }
    
    // Return the result of applying the Wahba solver and finding all matches with this.
    return find_matches(inertial, parameters.f(inertial, a));
}