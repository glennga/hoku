/// @file base-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for BaseTriangle class, which holds all common methods between the Planar and Spherical triangle
/// methods (they are very similar).

#include <iostream>
#include <cmath>
#include <algorithm>

#include "identification/base-triangle.h"

/// Returned when a query does not return any results.
const std::vector<BaseTriangle::label_trio> BaseTriangle::NO_CANDIDATE_TRIOS_FOUND = {{-1, -1, -1}};

/// Returned when no candidates can be found from a match step.
const std::vector<Star::trio> BaseTriangle::NO_CANDIDATE_STARS_FOUND = {{Star::zero(), Star::zero(), Star::zero()}};

/// Returned with an unsuccessful pivoting step.
const Star::trio BaseTriangle::NO_CANDIDATE_STAR_SET_FOUND = {Star::zero(), Star::zero(), Star::zero()};

/// Starting index trio to perform pivot with.
const BaseTriangle::index_trio BaseTriangle::STARTING_INDEX_TRIO = {0, 1, 2};

/// Constructor. Initializes our permutation stack.
BaseTriangle::BaseTriangle () : Identification(), p({}) {
}

/// Generate the triangle table given the specified FOV and table name. This find the area and polar moment
/// between each distinct permutation of trios, and only stores them if they fall within the corresponding
/// field-of-view.
///
/// @param fov Field of view limit (degrees) that all pairs must be within.
/// @param table_name Name of the table to generate.
/// @param compute_area Area function to compute area with.
/// @param compute_moment Polar moment function to compute moment with.
/// @return TABLE_ALREADY_EXISTS if the table already exists. Otherwise, 0 when finished.
int BaseTriangle::generate_triangle_table (const double fov, const std::string &table_name, area_function compute_area,
                                           moment_function compute_moment) {
    Chomp ch;
    SQLite::Transaction initial_transaction(*ch.conn);
    
    // Exit early if the table already exists.
    if (ch.create_table(table_name, "label_a INT, label_b INT, label_c INT, a FLOAT, i FLOAT")
        == Nibble::TABLE_NOT_CREATED) {
        return TABLE_ALREADY_EXISTS;
    }
    initial_transaction.commit();
    ch.select_table(table_name);
    
    // (i, j, k) are distinct, where no (i, j, k) = (j, k, i), (j, i, k), ....
    Star::list all_stars = ch.bright_as_list();
    for (unsigned int i = 0; i < all_stars.size() - 2; i++) {
        SQLite::Transaction transaction(*ch.conn);
        std::cout << "\r" << "Current *I* Star: " << all_stars[i].get_label();
        for (unsigned int j = i + 1; j < all_stars.size() - 1; j++) {
            for (unsigned int k = j + 1; k < all_stars.size(); k++) {
                
                // Only insert if the angle between all stars are separated by fov degrees or less.
                if (Star::within_angle({all_stars[i], all_stars[j], all_stars[k]}, fov)) {
                    double a_t = compute_area(all_stars[i], all_stars[j], all_stars[k]);
                    double i_t = compute_moment(all_stars[i], all_stars[j], all_stars[k]);
                    
                    // Prevent insertion of trios with non realistic moments/areas.
                    if (a_t > 0 && !std::isnan(i_t)) {
                        ch.insert_into_table("label_a, label_b, label_c, a, i",
                                             Nibble::tuple_d {static_cast<double>(all_stars[i].get_label()),
                                                 static_cast<double>(all_stars[j].get_label()),
                                                 static_cast<double>(all_stars[k].get_label()), a_t, i_t});
                    }
                }
            }
        }
        // Commit every star I change.
        transaction.commit();
    }
    
    // Create an index for area searches. We aren't searching for polar moments.
    return ch.polish_table("a");
}

/// Find the matching pairs using the appropriate triangle table and by comparing areas and polar moments. Assumes
/// noise is normally distributed, searches using epsilon (3 * sigma_a) and a basic bounded query.
///
/// @param a Area (planar or spherical) to search with.
/// @param i_t Polar moment (planar or spherical) to search with.
/// @return NO_CANDIDATE_TRIOS_FOUND if no candidates found. Otherwise, all elements that met the criteria.
std::vector<BaseTriangle::label_trio> BaseTriangle::query_for_trio (const double a, const double i) {
    double epsilon = 3.0 * this->parameters.sigma_query;
    std::vector<label_trio> area_moment_match = NO_CANDIDATE_TRIOS_FOUND;
    Nibble::tuples_d area_match;
    
    // First, search for trio of stars matching area condition.
    area_match = ch.simple_bound_query("a", "label_a, label_b, label_c, i", a - epsilon, a + epsilon,
                                       this->parameters.sql_limit);
    
    // Next, search this trio for stars matching the moment condition.
    area_moment_match.reserve(area_match.size() / 4);
    for (Chomp::tuple_d t : area_match) {
        if (t[3] >= i - epsilon && t[3] < i + epsilon) {
            area_moment_match.push_back(
                label_trio {static_cast<int> (t[0]), static_cast<int> (t[1]), static_cast<int>(t[2])});
        }
    }
    
    // If results are found, remove the initialized value of NO_CANDIDATE_TRIOS_FOUND.
    if (area_moment_match.size() > 1) {
        area_moment_match.erase(area_moment_match.begin());
    }
    return area_moment_match;
}

/// Given a trio of body stars, find matching trios of inertial stars using their respective planar areas and polar
/// moments.
///
/// @param i_b Index trio of stars in body (B) frame.
/// @param compute_area Area function to compute area with.
/// @param compute_moment Polar moment function to compute moment with.
/// @return NO_CANDIDATE_STARS_FOUND if stars are not within the fov or if no matches currently exist. Otherwise,
/// vector of trios whose areas and moments are close.
std::vector<Star::trio> BaseTriangle::m_stars (const index_trio &i_b, area_function compute_area,
                                               moment_function compute_moment) {
    Star::trio b_stars = {this->input[i_b[0]], this->input[i_b[1]], this->input[i_b[2]]};
    std::vector<label_trio> match_hr;
    std::vector<Star::trio> matched_stars;
    
    // Do not attempt to find matches if all stars are not within fov.
    if (!Star::within_angle({b_stars[0], b_stars[1], b_stars[2]}, this->fov)) {
        return NO_CANDIDATE_STARS_FOUND;
    }
    
    // Search for the current trio. If this is empty, then break early.
    match_hr = this->query_for_trio(compute_area(b_stars[0], b_stars[1], b_stars[2]),
                                    compute_moment(b_stars[0], b_stars[1], b_stars[2]));
    if (std::equal(match_hr[0].begin() + 1, match_hr[0].end(), match_hr[0].begin())) {
        return NO_CANDIDATE_STARS_FOUND;
    }
    
    // Grab stars themselves from catalog IDs found in matches. Return these matches.
    matched_stars.reserve(match_hr.size());
    for (const label_trio &t : match_hr) {
        matched_stars.push_back({ch.query_hip(static_cast<int> (t[0])), ch.query_hip(static_cast<int> (t[1])),
                                    ch.query_hip(static_cast<int> (t[2]))});
    }
    
    return matched_stars;
}

/// Generate a series of indices to iterate through as we perform the pivot operation. Store the results in the p stack.
///
/// @param i_b Index trio of stars in the body (B) frame that are 'removed' from this series.
void BaseTriangle::generate_pivot_list (const index_trio &i_b) {
    p.clear();
    
    for (unsigned int j = 0; j < input.size(); j++) {
        if (std::find(i_b.begin(), i_b.end(), j) == i_b.end()) {
            p.push_back(j);
        }
    }
}

/// Match the stars in the given set {B_1, B_2, B_3} to a trio in the database. If a past_set is given, then remove
/// all stars found matching the B trio that aren't found in the past set. Recurse until one definitive trio exists.
///
/// @param i_b Index trio of stars in body (B) frame.
/// @param past_set Matches found in a previous search.
/// @return NO_CANDIDATE_STAR_SET_FOUND if pivoting is unsuccessful. Otherwise, a trio of stars that match the given B
/// stars to R stars.
Star::trio BaseTriangle::pivot (const index_trio &i_b, const std::vector<Star::trio> &past_set) {
    std::vector<Star::trio> matches = this->match_stars(i_b);
    if (std::equal(matches.begin(), matches.end(), NO_CANDIDATE_STARS_FOUND.begin())) {
        matches.clear();
    }
    
    // Remove all trios from matches that have at least two stars in the past set.
    if (!past_set.empty() && !(std::equal(past_set[0].begin() + 1, past_set[0].end(), past_set[0].begin()))) {
        for (unsigned int i = 0; i < matches.size(); i++) {
            bool match_found = false;
            
            for (const Star::trio &past : past_set) {
                // We do not need to check all permutations. Break early and advance to next star.
                if ((past[0] == matches[i][0] || past[0] == matches[i][1] || past[0] == matches[i][2])
                    && (past[1] == matches[i][0] || past[1] == matches[i][1] || past[1] == matches[i][2])) {
                    match_found = true;
                    break;
                }
            }
            
            // If a match is not found, remove this from the match set.
            if (!match_found) {
                matches.erase(matches.begin() + i);
            }
        }
    }
    
    switch (matches.size()) {
        case 1: return matches[0]; // Only 1 trio exists. This must be the matching trio.
        case 0: return NO_CANDIDATE_STAR_SET_FOUND; // No trios exist. Exit early.
            // 2+ trios exists. Run with different 3rd element and history.
        default: return pivot(index_trio {i_b[0], i_b[1], ptop(this->p)}, matches);
    }
}

/// Check all possible configuration of star trios and return quaternion corresponding to the set with the largest
/// number of reference to body matches.
///
/// @param candidates All stars to check against the body star set.
/// @param r Inertial (frame R) trio of stars to check against the body trio.
/// @param b Body (frame B) Trio of stars to check against the inertial trio.
/// @return The quaternion corresponding to largest set of matching stars across the body and inertial in all pairing
/// configurations.
Star::list BaseTriangle::singular_identification (const Star::list &candidates, const Star::trio &r,
                                                  const Star::trio &b) {
    std::array<index_trio, 6> order = {STARTING_INDEX_TRIO};
    std::array<Star::list, 6> matches = {}, identities = {};
    auto ell = [&r, &b, &order] (const int i, const int j) -> Star {
        return Star::define_label(b[j], r[order[i][j]].get_label());
    };
    
    // Generate unique permutations using previously generated trio.
    for (int i = 1; i < 6; i++) {
        // Given i, swap elements 2 and 3 if even, or 1 and 3 if odd.
        order[i] = (i % 2) == 0 ? index_trio {0, 2, 1} : index_trio {2, 1, 0};
    }
    
    // Determine the rotation to take frame R to B. Only use r_1 and r_2 to get rotation.
    for (unsigned int i = 0; i < 3; i++) {
        Rotation q = parameters.f({b[0], b[1]}, {r[order[i][0]], r[order[i][1]]});
        matches[i] = find_matches(candidates, q);
        identities[i] = {ell(i, 0), ell(i, 1), ell(i, 2)};
    }
    
    // Return map set corresponding to the largest match (messy lambda and iterator stuff below D:).
    return identities[
        std::max_element(matches.begin(), matches.end(), [] (const Star::list &lhs, const Star::list &rhs) {
            return lhs.size() < rhs.size();
        }) - matches.begin()];
}

/// Find the matching pairs using the appropriate triangle table and by comparing areas and polar moments. This is
/// just a wrapper for query_for_trio.
///
/// @param a Area (planar or spherical) to search with.
/// @param i_t Polar moment (planar or spherical) to search with.
/// @return NO_CANDIDATE_TRIOS_FOUND if no candidates found. Otherwise, all elements that met the criteria.
std::vector<BaseTriangle::label_trio> BaseTriangle::e_query (double a, double i) {
    return query_for_trio(a, i);
}

/// Find the **best** matching pair to the first three stars in our benchmark using the appropriate triangle table.
/// Assumes noise is normally distributed, searches using epsilon (3 * sigma_a) and a basic bounded query.
///
/// @return NO_CANDIDATES_FOUND if no candidates found. Otherwise, a single elements that best meets the criteria.
Identification::labels_list BaseTriangle::e_reduction () {
    generate_pivot_list(STARTING_INDEX_TRIO);
    Star::trio candidate_trio = pivot(STARTING_INDEX_TRIO);
    
    if (std::equal(candidate_trio.begin(), candidate_trio.end(), NO_CANDIDATE_STAR_SET_FOUND.begin())) {
        return NO_CANDIDATES_FOUND;
    }
    else {
        return {candidate_trio[0].get_label(), candidate_trio[1].get_label(), candidate_trio[2].get_label()};
    }
}

/// Find the rotation from the images in our current benchmark to our inertial frame (i.e. the catalog).
///
/// @return NO_CONFIDENT_IDENTITY if an identification cannot be found exhaustively. EXCEEDED_NU_MAX if an
/// identification cannot be found within a certain number of query picks. Otherwise, body stars b with the attached
/// labels of the inertial pair r.
Star::list BaseTriangle::e_identify () {
    *parameters.nu = 0;
    
    // There exists |input| choose 3 possibilities.
    for (int i = 0; i < static_cast<signed> (input.size() - 2); i++) {
        for (int j = i + 1; j < static_cast<signed> (input.size() - 1); j++) {
            for (int k = j + 1; k < static_cast<signed> (input.size()); k++) {
                std::vector<Star::trio> candidate_trios;
                Star::trio candidate_trio;
                Star::list candidates;
                (*parameters.nu)++;
                
                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (*parameters.nu > parameters.nu_max) {
                    return EXCEEDED_NU_MAX;
                }
                
                // Find matches of current body trio to catalog. Pivot if necessary.
                generate_pivot_list({i, j, k});
                candidate_trio = pivot({i, j, k});
                if (std::equal(candidate_trio.begin(), candidate_trio.end(), NO_CANDIDATE_STAR_SET_FOUND.begin())) {
                    continue;
                }
                
                // Find candidate stars around the candidate trio.
                candidates = ch.nearby_hip_stars(candidate_trio[0], fov,
                                                 static_cast<unsigned int> (3.0 * input.size()));
                
                // Find the most likely map given the two pairs.
                return singular_identification(candidates, candidate_trio, {input[i], input[j], input[k]});
            }
        }
    }
    return NO_CONFIDENT_IDENTITY;
}