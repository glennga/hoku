/// @file angle.cpp
/// @author Glenn Galvizo
///
/// Source file for Angle class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#include <iostream>

#include "identification/angle.h"

/// Default parameters for the angle identification method.
const Identification::Parameters Angle::DEFAULT_PARAMETERS = {DEFAULT_SIGMA_QUERY, DEFAULT_SQL_LIMIT,
    DEFAULT_PASS_R_SET_CARDINALITY, DEFAULT_FAVOR_BRIGHT_STARS, DEFAULT_SIGMA_OVERLAY, DEFAULT_NU_MAX, DEFAULT_NU,
    DEFAULT_F, "ANGLE_20"};

/// Returned when no candidate pair is found from a query.
const Star::pair Angle::NO_CANDIDATE_PAIR_FOUND = {Star::wrap(Vector3::Zero()), Star::wrap(Vector3::Zero())};

/// Constructor. Sets the benchmark data, fov, parameters, and current working table.
///
/// @param input Working Benchmark instance. We are **only** copying the star set and the fov.
Angle::Angle (const Benchmark &input, const Parameters &p) : Identification() {
    input.present_image(this->big_i, this->fov);
    this->parameters = p;
    
    this->ch.select_table(parameters.table_name);
}

/// Generate the separation table given the specified FOV and table name. This finds the angle of separation between
/// each distinct permutation of stars, and only stores them if they fall within the corresponding field-of-view.
///
/// @param cf Configuration reader holding all parameters to use.
/// @param id_name If specified, use the table name here instead of the default "angle".
/// @return TABLE_ALREADY_EXISTS if the table already exists. Otherwise, 0 when finished.
int Angle::generate_table (INIReader &cf, const std::string &id_name) {
    Chomp ch;
    SQLite::Transaction transaction(*ch.conn);
    double fov = cf.GetReal("hardware", "fov", 0);
    
    // Exit early if the table already exists.
    std::string table_name = cf.Get("table-names", id_name, "");
    if (ch.create_table(table_name, "label_a INT, label_b INT, theta FLOAT") == Nibble::TABLE_NOT_CREATED) {
        return TABLE_ALREADY_EXISTS;
    }
    ch.select_table(table_name);
    
    // (i, j) are distinct, where no (i, j) = (j, i).
    Star::list all_stars = ch.bright_as_list();
    for (unsigned int i = 0; i < all_stars.size() - 1; i++) {
        std::cout << "\r" << "Current *I* Star: " << all_stars[i].get_label();
        for (unsigned int j = i + 1; j < all_stars.size(); j++) {
            double theta = (180.0 / M_PI) * Vector3::Angle(all_stars[i], all_stars[j]);
            
            // Only insert if the angle between both stars is less than fov.
            if (theta < fov) {
                ch.insert_into_table("label_a, label_b, theta",
                                     Nibble::tuple_d {static_cast<double> (all_stars[i].get_label()),
                                         static_cast<double>(all_stars[j].get_label()), theta});
            }
        }
    }
    
    transaction.commit();
    return ch.polish_table(cf.Get("table-focus", id_name, ""));
}

/// Find **a** matching pair using the appropriate SEP table and by comparing separation angles. Assumes noise is
/// normally distributed, searches using epsilon (3 * query_sigma). Limits the amount returned by the search using
/// 'sql_limit'.
///
/// @param theta Separation angle (degrees) to search with.
/// @return NO_CANDIDATES_FOUND if no candidates found. Label list of the matching catalog IDs otherwise.
Identification::labels_list Angle::query_for_pair (const double theta) {
    // Noise is normally distributed. Angle within 3 sigma of theta.
    double epsilon = 3.0 * this->parameters.sigma_query;
    std::vector<labels_list> big_r_ell;
    Nibble::tuples_d big_r_ell_tuples;
    
    // Query using theta with epsilon bounds. Return EMPTY_BIG_R_ELL if nothing is found.
    big_r_ell_tuples = ch.simple_bound_query("theta", "label_a, label_b, theta", theta - epsilon, theta + epsilon,
                                             this->parameters.sql_limit);
    
    // |R| = 1 restriction. Applied with the PASS_R_SET_CARDINALITY flag.
    if (big_r_ell_tuples.empty() || (this->parameters.pass_r_set_cardinality && big_r_ell_tuples.size() > 1)) {
        return EMPTY_BIG_R_ELL;
    }
    
    // Create the candidate label list.
    for (const Nibble::tuple_d &candidate : big_r_ell_tuples) {
        big_r_ell.push_back({static_cast<int>(candidate[0]), static_cast<int>(candidate[1])});
    }
    
    // Favor bright stars if specified. Applied with the FAVOR_BRIGHT_STARS flag.
    if (this->parameters.favor_bright_stars) {
        sort_brightness(big_r_ell);
    }
    
    return big_r_ell[0];
}

/// Given a set of body (frame B) stars, find the matching inertial (frame R) stars.
///
/// @param b_i Star I in frame B to find the match for.
/// @param b_j Star J in frame B to find the match for.
/// @return NO_CANDIDATE_PAIR_FOUND if no matching pair is found. Otherwise, two inertial stars that match
/// the given body.
Star::pair Angle::find_candidate_pair (const Star &b_i, const Star &b_j) {
    double theta = (180.0 / M_PI) * Vector3::Angle(b_i, b_j);
    
    // If the current angle is greater than the current fov, break early.
    if (theta > this->fov) {
        return NO_CANDIDATE_PAIR_FOUND;
    }
    
    // If no candidate is found, break early.
    labels_list big_r_ell = this->query_for_pair(theta);
    if (std::equal(big_r_ell.begin(), big_r_ell.end(), EMPTY_BIG_R_ELL.begin())) {
        return NO_CANDIDATE_PAIR_FOUND;
    }
    
    // Otherwise, obtain and return the inertial vectors for the given candidates.
    return {ch.query_hip(big_r_ell[0]), ch.query_hip(big_r_ell[1])};
}

/// Find the best fitting match of input stars (body) to database stars (catalog) using the given pair as reference.
////
/// Assumption One: b_1 = r_1, b_2 = r_2
/// Assumption Two: b_1 = r_2, b_2 = r_1
///
/// @param big_p All stars found near the inertial pair.
/// @param r Inertial (catalog frame) pair of stars that match body pair. This must be length = 2.
/// @param b Body (body frame) pair of stars that match inertial pair. This must be length = 2.
/// @return Body stars b with the attached labels of the inertial pair r.
Star::list Angle::direct_match_test (const Star::list &big_p, const Star::list &r, const Star::list &b) {
    if (r.size() != 2 || b.size() != 2) {
        throw std::runtime_error(std::string("Input lists does not have exactly two b."));
    }
    std::array<Star::list, 2> big_m = {}, big_a = {};
    
    // Determine the rotation to take frame B to A, find all matches with this rotation.
    for (unsigned int i = 0; i < 2; i++) {
        // We define our identity 'a' below.
        std::array<int, 2> a = {(i == 0) ? 0 : 1, (i == 0) ? 1 : 0};
        
        big_m[i] = find_positive_overlay(big_p, parameters.f({b[0], b[1]}, {r[a[0]], r[a[1]]}));
        big_a[i] = {Star::define_label(b[0], r[a[0]].get_label()), Star::define_label(b[1], r[a[1]].get_label())};
    }
    
    // Return the body pair with the appropriate labels.
    return (big_m[0].size() > big_m[1].size()) ? big_a[0] : big_a[1];
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
std::vector<Identification::labels_list> Angle::query (const Star::list &s) {
    if (s.size() != QUERY_STAR_SET_SIZE) {
        throw std::runtime_error(std::string("Input list does not have exactly two b."));
    }
    double epsilon = 3.0 * this->parameters.sigma_query, theta = (180.0 / M_PI) * Vector3::Angle(s[0], s[1]);
    std::vector<labels_list> big_r_ell;
    
    // Query using theta with epsilon bounds.
    ch.select_table(this->parameters.table_name);
    Nibble::tuples_d big_r_tuples = ch.simple_bound_query("theta", "label_a, label_b", theta - epsilon, theta + epsilon,
                                                          this->parameters.sql_limit);
    
    // Sort r into list of catalog ID pairs.
    big_r_ell.reserve(big_r_tuples.size() / 2);
    for (const Nibble::tuple_d &r_t : big_r_tuples) {
        big_r_ell.emplace_back(labels_list {static_cast<int>(r_t[0]), static_cast<int>(r_t[1])});
    }
    
    return big_r_ell;
}

/// Reproduction of the Angle method's querying to candidate reduction step (i.e. none). Input image is used.
/// We require the following be defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_query
///     - sql_limit
/// @endcode
///
/// @return EMPTY_BIG_R_ELL if a single match configuration cannot be found. Otherwise, a single match configuration.
Angle::labels_list Angle::reduce () {
    ch.select_table(parameters.table_name);
    
    for (unsigned int i = 0; i < big_i.size() - 1; i++) {
        for (unsigned int j = i + 1; j < big_i.size(); j++) {
            Star::pair r = find_candidate_pair(big_i[i], big_i[j]);
            
            // The reduction step: |R| = 1.
            if (std::equal(r.begin(), r.end(), NO_CANDIDATE_PAIR_FOUND.begin())) {
                continue;
            }
            
            return {r[0].get_label(), r[1].get_label()};
        }
    }
    return EMPTY_BIG_R_ELL;
}

/// Reproduction of the Angle method's process from beginning to the orientation determination. Input image is used.
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
Star::list Angle::identify () {
    *parameters.nu = 0;
    
    // There exists |big_i| choose 2 possibilities.
    for (unsigned int i = 0; i < big_i.size() - 1; i++) {
        for (unsigned int j = i + 1; j < big_i.size(); j++) {
            Star::list big_p;
            (*parameters.nu)++;
            
            // Practical limit: exit early if we have iterated through too many comparisons without match.
            if (*parameters.nu > parameters.nu_max) {
                return EXCEEDED_NU_MAX;
            }
            
            // Narrow down current pair to two stars in catalog. The order is currently unknown.
            Star::pair r = find_candidate_pair(big_i[i], big_i[j]);
            if (std::equal(r.begin(), r.end(), NO_CANDIDATE_PAIR_FOUND.begin())) {
                continue;
            }
            
            // Find candidate stars around the candidate pair.
            big_p = ch.nearby_hip_stars(r[0], fov, static_cast<unsigned int>(3 * big_i.size()));
            
            // Find the most likely pair combination given the two pairs.
            return direct_match_test(big_p, {r[0], r[1]}, {big_i[i], big_i[j]});
        }
    }
    
    return NO_CONFIDENT_A;
}
