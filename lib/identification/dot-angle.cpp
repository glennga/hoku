/// @file dot-angle.cpp
/// @author Glenn Galvizo
///
/// Source file for DotAngle class, which matches a set of body vectors (stars) to their inertial counter-parts in
/// the database.

#include <iostream>

#include "math/trio.h"
#include "identification/dot-angle.h"

/// Exact number of query stars required for query experiment.
const unsigned int Dot::QUERY_STAR_SET_SIZE = 3;

/// Default parameters for the dot angle identification method.
const Identification::Parameters Dot::DEFAULT_PARAMETERS = {DEFAULT_SIGMA_QUERY, DEFAULT_SIGMA_QUERY,
    DEFAULT_SIGMA_QUERY, DEFAULT_SIGMA_4, DEFAULT_SQL_LIMIT, DEFAULT_NO_REDUCTION, DEFAULT_FAVOR_BRIGHT_STARS,
    DEFAULT_NU_MAX, DEFAULT_NU, DEFAULT_F, "DOT_20"};

/// Returned when the reduction step does not pass in the query function.
const Identification::labels_list Dot::NO_CANDIDATES_FOUND = {-1, -1};

/// Returned when no candidate pair is found from a query.
const Star::trio Dot::NO_CANDIDATE_TRIO_FOUND = {Star::wrap(Vector3::Zero()), Star::wrap(Vector3::Zero()),
    Star::wrap(Vector3::Zero())};

/// Constructor. Sets the benchmark data, fov, parameters, and current working table.
///
/// @param input Working Benchmark instance. We are **only** copying the star set and the fov.
DotAngle::DotAngle (const Benchmark &input, const Parameters &p) : Identification() {
    input.present_image(this->big_i, this->fov);
    this->parameters = p;
    
    this->ch.select_table(parameters.table_name);
}

/// Generate the separation table given the specified FOV and table name. This finds the angle of separation between
/// each distinct permutation of stars, and only stores them if they fall within the corresponding field-of-view.
///
/// @param cf Configuration reader holding all parameters to use.
/// @return TABLE_ALREADY_EXISTS if the table already exists. Otherwise, 0 when finished.
int Dot::generate_table (INIReader &cf) {
    double fov = cf.GetReal("hardware", "fov", 0);
    Chomp ch;
    
    // Exit early if the table already exists.
    std::string table_name = cf.Get("table-names", "dot", "");
    if (ch.create_table(table_name, "label_a INT, label_b INT, label_c INT, theta_1 FLOAT, theta_2 FLOAT, phi FLOAT")
        == Nibble::TABLE_NOT_CREATED) {
        return TABLE_ALREADY_EXISTS;
    }
    ch.select_table(table_name);
    
    // Iterate through all possible permutations of i, j, c...
    Star::list all_stars = ch.bright_as_list();
    for (unsigned int i = 0; i < all_stars.size(); i++) {
        SQLite::Transaction transaction(*ch.conn);
        std::cout << "\r" << "Current *I* Star: " << all_stars[i].get_label();
        
        for (unsigned int j = 0; j < all_stars.size(); j++) {
            for (unsigned int c = 0; c < all_stars.size(); c++) {
                if (i == j || i == c || j == c) {
                    continue;
                }
                
                // Compute each feature (theta^1, theta^2, phi).
                double theta_1 = (180.0 / M_PI) * Vector3::Angle(all_stars[c], all_stars[i]);
                double theta_2 = (180.0 / M_PI) * Vector3::Angle(all_stars[c], all_stars[j]);
                double phi = Trio::dot_angle(all_stars[i], all_stars[j], all_stars[c]);
                
                // Condition 6d: theta^1 < theta^2.
                if (theta_1 < theta_2 && Star::within_angle({all_stars[i], all_stars[j], all_stars[c]}, fov)) {
                    ch.insert_into_table("label_a, label_b, label_c, theta_1, theta_2, phi",
                                         Nibble::tuple_d {static_cast<double> (all_stars[i].get_label()),
                                             static_cast<double>(all_stars[j].get_label()),
                                             static_cast<double> (all_stars[c].get_label()), theta_1, theta_2, phi});
                }
            }
        }
        transaction.commit();
    }
    return ch.polish_table(cf.Get("table-focus", "dot", ""));
}

/// Find **a** matching pair using the appropriate SEP table and by comparing separation angles. Assumes noise is
/// normally distributed, searches using epsilon (3 * query_sigma). Limits the amount returned by the search using
/// 'sql_limit'.
///
/// @param theta Separation angle (degrees) to search with.
/// @return NO_CANDIDATES_FOUND if no candidates found. Label list of the matching catalog IDs otherwise.
Identification::labels_list Dot::query_for_trio (double theta_1, double theta_2, double phi) {
    // Noise is normally distributed. All queries within 3 sigma.
    double epsilon_1 = 3.0 * this->parameters.sigma_1, epsilon_2 = 3.0 * this->parameters.sigma_2;
    double epsilon_3 = 3.0 * parameters.sigma_3;
    std::vector<labels_list> big_r_ell;
    Nibble::tuples_d matches;
    
    // Query for candidates using all fields.
    matches = ch.simple_bound_query({"theta_1", "theta_2", "phi"}, "label_a, label_b, label_c",
                                    {theta_1 - epsilon_1, theta_2 - epsilon_2, phi - epsilon_3},
                                    {theta_1 + epsilon_1, theta_2 + epsilon_2, phi + epsilon_3},
                                    this->parameters.sql_limit);
    
    // Transform each tuple into a candidate list of labels.
    big_r_ell.reserve(matches.size());
    std::for_each(matches.begin(), matches.end(), [&big_r_ell] (const Chomp::tuple_d &t) -> void {
        big_r_ell.emplace_back(labels_list {static_cast<int> (t[0]), static_cast<int> (t[1]), static_cast<int>(t[2])});
    });
    
    // |R| = 1 restriction. Applied with the PASS_R_SET_CARDINALITY flag.
    if (big_r_ell.empty() || (!this->parameters.no_reduction && big_r_ell.size() > 1)) {
        return NO_CANDIDATES_FOUND;
    }
    
    // Favor bright stars if specified. Applied with the FAVOR_BRIGHT_STARS flag.
    if (this->parameters.favor_bright_stars) {
        sort_brightness(big_r_ell);
    }
    
    return big_r_ell[0];
}

/// Given a set of body (frame B) stars, find the matching inertial (frame R) stars and the correct mapping if one
/// exists.
///
/// @param b_i Star I in frame B to find the match for.
/// @param b_j Star J in frame B to find the match for.
/// @param b_c Central star C in frame B to find the match for.
/// @return NO_CANDIDATE_TRIO_FOUND if no matching pair is found. Otherwise, three inertial stars that match
/// the given body in order [b_i, b_j, b_c].
Star::trio Dot::find_candidate_trio (const Star &b_i, const Star &b_j, const Star &b_c) {
    double theta_1 = (180.0 / M_PI) * Vector3::Angle(b_c, b_i), theta_2 = (180.0 / M_PI) * Vector3::Angle(b_c, b_j);
    double phi = Trio::dot_angle(b_i, b_j, b_c);
    
    // Ensure that condition 6d holds, and that all stars are within fov. Exit early if this is not met.
    if (theta_1 > theta_2 || !Star::within_angle({b_i, b_j, b_c}, this->fov)) {
        return NO_CANDIDATE_TRIO_FOUND;
    }
    
    // If not candidate is found, break early.
    labels_list big_r_ell = this->query_for_trio(theta_1, theta_2, phi);
    if (std::equal(big_r_ell.begin(), big_r_ell.end(), NO_CONFIDENT_R.begin())) {
        return NO_CANDIDATE_TRIO_FOUND;
    }
    
    // Otherwise, obtain and return the inertial vectors for the given candidates.
    return {ch.query_hip(big_r_ell[0]), ch.query_hip(big_r_ell[1]), ch.query_hip(big_r_ell[2])};
}

/// Reproduction of the Angle method's database querying. Input image is not used. We require the following be defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_query
///     - sql_limit
/// @endcode
///
/// @param s Stars to query with. This must be of length = QUERY_STAR_SET_SIZE.
/// @return Vector of likely matches found by the angle method.
std::vector<Identification::labels_list> Dot::query (const Star::list &s) {
    if (s.size() != QUERY_STAR_SET_SIZE) {
        throw std::runtime_error(std::string("Input list does not have exactly three b."));
    }
    double theta_1 = (180.0 / M_PI) * Vector3::Angle(s[2], s[0]), theta_2 = (180.0 / M_PI) * Vector3::Angle(s[2], s[1]);
    double phi = Trio::dot_angle(s[0], s[1], s[2]);
    
    // Noise is normally distributed. All queries within 3 sigma.
    double epsilon_1 = 3.0 * this->parameters.sigma_1, epsilon_2 = 3.0 * this->parameters.sigma_2;
    double epsilon_3 = 3.0 * parameters.sigma_3;
    std::vector<labels_list> big_r_ell;
    Nibble::tuples_d matches;
    
    // Ensure that condition 6d holds: switch if not.
    if (theta_1 > theta_2) {
        double theta_t = theta_1;
        theta_1 = theta_2, theta_2 = theta_t;
    }
    
    // Query for our candidate set.
    matches = ch.simple_bound_query({"theta_1", "theta_2", "phi"}, "label_a, label_b, label_c",
                                    {theta_1 - epsilon_1, theta_2 - epsilon_2, phi - epsilon_3},
                                    {theta_1 + epsilon_1, theta_2 + epsilon_2, phi + epsilon_3},
                                    this->parameters.sql_limit);
    
    // Transform candidate set tuples into labels list.
    big_r_ell.reserve(matches.size());
    std::for_each(matches.begin(), matches.end(), [&big_r_ell] (const Chomp::tuple_d &t) -> void {
        big_r_ell.emplace_back(labels_list {static_cast<int> (t[0]), static_cast<int> (t[1]), static_cast<int>(t[2])});
    });
    
    return big_r_ell;
}

/// Reproduction of the Dot method's querying to candidate reduction step (i.e. none). Input image is used.
/// We require the following be defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_query
///     - sql_limit
///     - nu
///     - nu_max
/// @endcode
///
/// @return EMPTY_BIG_R_ELL if there does not exist exactly one image star set. Otherwise, a single match configuration
/// found by the query method.
Star::list Dot::reduce () {
    ch.select_table(parameters.table_name);
    *parameters.nu = 0;
    
    for (unsigned int i = 0; i < big_i.size() - 2; i++) {
        for (unsigned int j = i + 1; j < big_i.size() - 1; j++) {
            for (unsigned int c = j + 1; c < big_i.size(); c++) {
                Star::trio big_r = find_candidate_trio(big_i[i], big_i[j], big_i[c]);
                (*parameters.nu)++;
    
                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (*parameters.nu > parameters.nu_max) {
                    return NO_CONFIDENT_R;
                }
                
                // The reduction step: |R| = 1.
                if (std::equal(big_r.begin(), big_r.end(), NO_CANDIDATE_TRIO_FOUND.begin())) {
                    continue;
                }
                
                return {big_r[0], big_r[1], big_r[2]};
            }
        }
    }
    return NO_CONFIDENT_R;
}

/// Reproduction of the DotAngle method's process from beginning to the orientation determination. Input image is used.
/// We require the following be defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_overlay
///     - sql_limit
///     - sigma_query
///     - nu
///     - nu_max
/// @endcode
///
/// @param input The set of benchmark data to work with.
/// @param p Adjustments to the identification process.
/// @return NO_CONFIDENT_A if an identification cannot be found exhaustively. EXCEEDED_NU_MAX if an
/// identification cannot be found within a certain number of query picks. Otherwise, body stars b with the attached
/// labels of the inertial pair r.
Star::list Dot::identify () {
    *parameters.nu = 0;
    
    // There exists |big_i| choose 3 possibilities.
    for (unsigned int i = 0; i < big_i.size() - 2; i++) {
        for (unsigned int j = i + 1; j < big_i.size() - 1; j++) {
            for (unsigned int c = j + 1; c < big_i.size(); c++) {
                bool is_swapped = false;
                
                // Practical limit: exit early if we have iterated through too many comparisons without match.
                (*parameters.nu)++;
                if (*parameters.nu > parameters.nu_max) {
                    return EXCEEDED_NU_MAX;
                }
                
                // Determine which stars map to the current 'b'. If this fails, swap b_i and b_j.
                Star::trio r = find_candidate_trio(big_i[i], big_i[j], big_i[c]);
                if (std::equal(r.begin(), r.end(), NO_CANDIDATE_TRIO_FOUND.begin())) {
                    r = find_candidate_trio(big_i[j], big_i[i], big_i[c]), is_swapped = true;
                }
                
                // If there exist no matches at this point, then repeat for another pair.
                if (std::equal(r.begin(), r.end(), NO_CANDIDATE_TRIO_FOUND.begin())) {
                    continue;
                }
                
                // Otherwise, attach the labels to the body and return this set.
                Star::define_label(big_i[c], r[2].get_label());
                Star::define_label(big_i[(is_swapped) ? j : i], r[0].get_label());
                Star::define_label(big_i[(is_swapped) ? i : j], r[0].get_label());
                return {big_i[i], big_i[j], big_i[c]};
            }
        }
    }
    
    return NO_CONFIDENT_A;
}
