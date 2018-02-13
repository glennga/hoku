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
const Star::pair Angle::NO_CANDIDATE_PAIR_FOUND = {Star::zero(), Star::zero()};

/// Constructor. Sets the benchmark data, fov, parameters, and current working table.
///
/// @param input Working Benchmark instance. We are **only** copying the star set and the fov.
Angle::Angle (const Benchmark &input, const Parameters &p) : Identification() {
    input.present_image(this->input, this->fov);
    this->parameters = p;
    
    this->ch.select_table(parameters.table_name);
}

/// Generate the separation table given the specified FOV and table name. This finds the angle of separation between
/// each distinct permutation of stars, and only stores them if they fall within the corresponding field-of-view.
///
/// @param fov Field of view limit (degrees) that all pairs must be within.
/// @param table_name Name of the table to generate.
/// @return TABLE_ALREADY_EXISTS if the table already exists. Otherwise, 0 when finished.
int Angle::generate_table (double fov, const std::string &table_name) {
    Chomp ch;
    SQLite::Transaction transaction(*ch.conn);
    
    // Exit early if the table already exists.
    if (ch.create_table(table_name, "label_a INT, label_b INT, theta FLOAT") == Nibble::TABLE_NOT_CREATED) {
        return TABLE_ALREADY_EXISTS;
    }
    ch.select_table(table_name);
    
    // (i, j) are distinct, where no (i, j) = (j, i).
    Star::list all_stars = ch.bright_as_list();
    for (unsigned int i = 0; i < all_stars.size() - 1; i++) {
        std::cout << "\r" << "Current *I* Star: " << all_stars[i].get_label();
        for (unsigned int j = i + 1; j < all_stars.size(); j++) {
            double theta = Star::angle_between(all_stars[i], all_stars[j]);
            
            // Only insert if the angle between both stars is less than fov.
            if (theta < fov) {
                ch.insert_into_table("label_a, label_b, theta",
                                     Nibble::tuple_d {static_cast<double> (all_stars[i].get_label()),
                                         static_cast<double>(all_stars[j].get_label()), theta});
            }
        }
    }
    
    transaction.commit();
    return ch.polish_table("theta");
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
    std::vector<labels_list> candidate_labels;
    Nibble::tuples_d candidates;
    
    // Query using theta with epsilon bounds. Return NO_CANDIDATES_FOUND if nothing is found.
    candidates = ch.simple_bound_query("theta", "label_a, label_b, theta", theta - epsilon, theta + epsilon,
                                       this->parameters.sql_limit);
    
    // |R| = 1 restriction. Applied with the PASS_R_SET_CARDINALITY flag.
    if (candidates.empty() || (this->parameters.pass_r_set_cardinality && candidates.size() > 1)) {
        return NO_CANDIDATES_FOUND;
    }
    
    // Create the candidate label list.
    for (const Nibble::tuple_d &candidate : candidates) {
        candidate_labels.push_back({static_cast<int>(candidate[0]), static_cast<int>(candidate[1])});
    }
    
    // Favor bright stars if specified. Applied with the FAVOR_BRIGHT_STARS flag.
    if (this->parameters.favor_bright_stars) {
        sort_brightness(candidate_labels);
    }
    
    return candidate_labels[0];
}

/// Given a set of body (frame B) stars, find the matching inertial (frame R) stars.
///
/// @param b_a Star A in frame B to find the match for.
/// @param b_b Star B in frame B to find the match for.
/// @return NO_CANDIDATE_PAIR_FOUND if no matching pair is found. Otherwise, two inertial stars that match
/// the given body.
Star::pair Angle::find_candidate_pair (const Star &b_a, const Star &b_b) {
    double theta = Star::angle_between(b_a, b_b);
    
    // If the current angle is greater than the current fov, break early.
    if (theta > this->fov) {
        return NO_CANDIDATE_PAIR_FOUND;
    }
    
    // If no candidate is found, break early.
    labels_list candidates = this->query_for_pair(theta);
    if (std::equal(candidates.begin(), candidates.end(), NO_CANDIDATES_FOUND.begin())) {
        return NO_CANDIDATE_PAIR_FOUND;
    }
    
    // Otherwise, obtain and return the inertial vectors for the given candidates.
    return {ch.query_hip(candidates[0]), ch.query_hip(candidates[1])};
}

/// Find the best fitting match of input stars (frame B) to database stars (frame R) using the given pair as reference.
////
/// Assumption One: B_a = R_a, B_b = R_b
/// Assumption Two: B_a = R_b, B_b = R_a
///
/// @param candidates All stars found near the inertial pair.
/// @param r Inertial (frame R) pair of stars that match body pair. This must be length = 2.
/// @param b Body (frame B) pair of stars that match inertial pair. This must be length = 2.
/// @return Body stars b with the attached labels of the inertial pair r.
Star::list Angle::singular_identification (const Star::list &candidates, const Star::list &r, const Star::list &b) {
    if (r.size() != 2 || b.size() != 2) {
        throw std::runtime_error(std::string("Input lists does not have exactly two stars."));
    }
    std::array<Star::list, 2> matches = {}, identities = {};
    
    // Determine the rotation to take frame B to A, find all matches with this rotation.
    for (unsigned int i = 0; i < 2; i++) {
        // We define our identity 'a' below.
        std::array<int, 2> a = {(i == 0) ? 0 : 1, (i == 0) ? 1 : 0};
        
        matches[i] = find_matches(candidates, parameters.f({b[0], b[1]}, {r[a[0]], r[a[1]]}));
        identities[i] = {Star::define_label(b[0], r[a[0]].get_label()), Star::define_label(b[1], r[a[1]].get_label())};
    }
    
    // Return the body pair with the appropriate labels.
    return (matches[0].size() > matches[1].size()) ? identities[0] : identities[1];
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
        throw std::runtime_error(std::string("Input list does not have exactly two stars."));
    }
    double epsilon = 3.0 * this->parameters.sigma_query, theta = Star::angle_between(s[0], s[1]);
    std::vector<labels_list> r_bar;
    
    // Query using theta with epsilon bounds.
    ch.select_table(this->parameters.table_name);
    Nibble::tuples_d r = ch.simple_bound_query("theta", "label_a, label_b", theta - epsilon, theta + epsilon,
                                               this->parameters.sql_limit);
    
    // Sort r into list of catalog ID pairs.
    r_bar.reserve(r.size() / 2);
    for (const Nibble::tuple_d &r_t : r) {
        r_bar.emplace_back(labels_list {static_cast<int>(r_t[0]), static_cast<int>(r_t[1])});
    }
    
    return r_bar;
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
/// @return NO_CANDIDATES_FOUND if there does not exist exactly one image star. Otherwise, a single match configuration
/// found by the angle method.
Angle::labels_list Angle::reduce () {
    ch.select_table(parameters.table_name);
    std::vector<labels_list> p = query({input[0], input[1]});
    return (p.size() != 1) ? NO_CANDIDATES_FOUND : p[0];
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
/// @return NO_CONFIDENT_IDENTITY if an identification cannot be found exhaustively. EXCEEDED_NU_MAX if an
/// identification cannot be found within a certain number of query picks. Otherwise, body stars b with the attached
/// labels of the inertial pair r.
Star::list Angle::identify () {
    *parameters.nu = 0;
    
    // There exists |input| choose 2 possibilities.
    for (unsigned int i = 0; i < input.size() - 1; i++) {
        for (unsigned int j = i + 1; j < input.size(); j++) {
            Star::list candidates;
            (*parameters.nu)++;
            
            // Practical limit: exit early if we have iterated through too many comparisons without match.
            if (*parameters.nu > parameters.nu_max) {
                return EXCEEDED_NU_MAX;
            }
            
            // Narrow down current pair to two stars in catalog. The order is currently unknown.
            Star::pair candidate_pair = find_candidate_pair(input[i], input[j]);
            if (std::equal(candidate_pair.begin(), candidate_pair.end(), NO_CANDIDATE_PAIR_FOUND.begin())) {
                continue;
            }
            
            // Find candidate stars around the candidate pair.
            candidates = ch.nearby_hip_stars(candidate_pair[0], fov, static_cast<unsigned int>(3 * input.size()));
            
            // Find the most likely pair combination given the two pairs.
            return singular_identification(candidates, {candidate_pair[0], candidate_pair[1]}, {input[i], input[j]});
        }
    }
    
    return NO_CONFIDENT_IDENTITY;
}
