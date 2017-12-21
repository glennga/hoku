/// @file base-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for BaseTriangle class, which holds all common methods between the Planar and Spherical triangle
/// methods (they are very similar).

#include "identification/base-triangle.h"

/// Find the matching pairs using the appropriate triangle table and by comparing areas and polar moments. Assumes
/// noise is normally distributed, searches using epsilon (3 * sigma_a) and a basic bounded query.
///
/// @param a Area (planar or spherical) to search with.
/// @param i_t Polar moment (planar or spherical) to search with.
/// @return [-1][-1][-1] if no candidates found. Otherwise, all elements that met the criteria.
std::vector<BaseTriangle::label_trio> BaseTriangle::query_for_trio (const double a, const double i) {
    double epsilon = 3.0 * this->parameters.sigma_query;
    std::vector<label_trio> area_moment_match = {{-1, -1, -1}};
    Nibble::tuples_d area_match;
    
    // First, search for trio of stars matching area condition.
    area_match = ch.simple_bound_query("theta", "label_a, label_b, label_c, i", a - epsilon, a + epsilon,
                                       this->parameters.sql_limit);
    
    // Next, search this trio for stars matching the moment condition.
    area_moment_match.reserve(area_match.size() / 4);
    for (Chomp::tuple_d t : area_match) {
        if (t[3] >= i - epsilon && t[3] < i + epsilon) {
            area_moment_match.push_back(label_trio {t[0], t[1], t[2]});
        }
    }
    
    // If results are found, remove the initialized value of [-1][-1][-1].
    if (area_moment_match.size() > 1) {
        area_moment_match.erase(area_moment_match.begin());
    }
    return area_moment_match;
}

/// Return the same index trio, with a new A value such that A exists in space [0, 1, ..., |input|] and A != B, A != C.
///
/// @param i_t Current index trio, generates another permutation based off this.
/// @return Same index trio, with a new A index.
BaseTriangle::index_trio BaseTriangle::permutate_index (const index_trio &i_t) {
    index_trio t = i_t;
    
    do {
        t[0]++;
    }
    while (t[0] == t[1] || t[0] == t[2]);
    
    return t;
}

/// Match the stars in the given set {B_1, B_2, B_3} to a trio in the database. If a past_set is given, then remove
/// all stars found matching the B trio that aren't found in the past set. Recurse until one definitive trio exists.
///
/// @param i_b Index trio of stars in body (B) frame.
/// @param past_set Matches found in a previous search.
/// @return A trio of stars that match the given B stars to R stars.
Trio::stars BaseTriangle::pivot (const index_trio &i_b, const std::vector<Trio::stars> &past_set) {
    std::vector<Trio::stars> matches = this->match_stars(i_b);
    if (matches[0][0] == Star::zero()) {
        matches.clear();
    }
    
    // Remove all trios from matches that have at least two stars in the past set.
    if (!past_set.empty() && !(std::equal(past_set[0].begin() + 1, past_set[0].end(), past_set[0].begin()))) {
        for (unsigned int i = 0; i < matches.size(); i++) {
            bool match_found = false;
            
            for (const Trio::stars &past : past_set) {
                // We do not need to check all permutations. Break early and advance to next star.
                if ((past[1] == matches[i][0] || past[1] == matches[i][1] || past[1] == matches[i][2])
                    && (past[2] == matches[i][0] || past[2] == matches[i][1] || past[2] == matches[i][2])) {
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
        case 0: return {Star::zero(), Star::zero(), Star::zero()}; // No trios exist. Exit early.
        default: return pivot(permutate_index(i_b), matches); // 2+ trios exists. Run with different trio and history.
    }
}

/// Check all possible configuration of star trios and return the set with the largest number of reference to body
/// matches.
///
/// @param candidates All stars to check against the body star set.
/// @param r Inertial (frame R) trio of stars to check against the body trio.
/// @param b_labels Body (frame B) Catalog IDs for the trio of stars to check against the inertial trio.
/// @return The largest set of matching stars across the body and inertial in all pairing configurations.
Star::list BaseTriangle::check_assumptions (const Star::list &candidates, const Trio::stars &r,
                                            const index_trio &b_labels) {
    index_trio current_order = {0, 1, 2};
    std::array<Trio::stars, 6> r_assumption_list;
    std::array<Star::list, 6> matches;
    int current_assumption = 0;
    
    // Generate unique permutations using previously generated trio.
    r_assumption_list[0] = {r[current_order[0]], r[current_order[1]], r[current_order[2]]};
    for (int i = 1; i < 6; i++) {
        // Given i, swap elements 2 and 3 if even, or 1 and 3 if odd.
        current_order = (i % 2) == 0 ? index_trio {0, 2, 1} : index_trio {2, 1, 0};
        r_assumption_list[i] = {r[current_order[0]], r[current_order[1]], r[current_order[2]]};
    }
    
    // Determine the rotation to take frame R to B. Only use r_1 and r_2 to get rotation.
    for (const Trio::stars &assumption : r_assumption_list) {
        Rotation q = Rotation::rotation_across_frames({this->input[b_labels[0]], this->input[b_labels[1]]},
                                                      {assumption[0], assumption[1]});
        matches[current_assumption++] = find_matches(candidates, q);
    }
    
    // Return the larger of the six matches.
    return std::max_element(matches.begin(), matches.end(), [] (const Star::list &lhs, const Star::list &rhs) {
        return lhs.size() < rhs.size();
    })[0];
}

std::vector<label_trio> BaseTriangle::e_query (double a, double i) {

}

/// Check all possible configuration of star trios and return quaternion corresponding to the  set with the largest
/// number of reference to body matches.
///
/// @param candidates All stars to check against the body star set.
/// @param r Inertial (frame R) trio of stars to check against the body trio.
/// @param b Body (frame B) Trio of stars to check against the inertial trio.
/// @return The quaternion corresponding to largest set of matching stars across the body and inertial in all pairing
/// configurations.
Rotation BaseTriangle::e_alignment (const Star::list &candidates, const Trio::stars &r, const Trio::stars &b) {
    index_trio current_order = {0, 1, 2};
    std::array<Trio::stars, 6> r_assumption_list;
    std::array<Star::list, 6> matches;
    std::array<Rotation, 6> q;
    
    // Generate unique permutations using previously generated trio.
    r_assumption_list[0] = {r[current_order[0]], r[current_order[1]], r[current_order[2]]};
    for (int i = 1; i < 6; i++) {
        // Given i, swap elements 2 and 3 if even, or 1 and 3 if odd.
        current_order = (i % 2) == 0 ? index_trio {0, 2, 1} : index_trio {2, 1, 0};
        r_assumption_list[i] = {r[current_order[0]], r[current_order[1]], r[current_order[2]]};
    }
    
    // Determine the rotation to take frame R to B. Only use r_1 and r_2 to get rotation.
    for (unsigned int i = 0; i < r_assumption_list.size(); i++) {
        q[i] = Rotation::rotation_across_frames({b[0], b[1]}, {r_assumption_list[i][0], r_assumption_list[i][1]});
        matches[i] = find_matches(candidates, q[i]);
    }
    
    // Return quaternion corresponding to the largest match (messy lambda and iterator stuff below D:).
    return q[std::max_element(matches.begin(), matches.end(), [] (const Star::list &lhs, const Star::list &rhs) {
        return lhs.size() < rhs.size();
    }) - matches.begin()];
}

/// Find the **best** matching pair to the first three stars in our benchmark using the appropriate triangle table.
/// Assumes noise is normally distributed, searches using epsilon (3 * sigma_a) and K-Vector query.
///
/// @return [-1][-1][-1] if no candidates found. Otherwise, a single elements that best meets the criteria.
BaseTriangle::label_trio BaseTriangle::e_reduction () {
    Trio::stars candidate_trio = pivot({0, 1, 2});
    
    if (candidate_trio[0] == Star::zero() && candidate_trio[1] == Star::zero() && candidate_trio[2] == Star::zero()) {
        return {-1, -1, -1};
    }
    else {
        return {candidate_trio[0].get_label(), candidate_trio[1].get_label(), candidate_trio[2].get_label()};
    }
}

/// Find the rotation from the images in our current benchmark to our inertial frame (i.e. the catalog).
///
/// @param z Reference to variable that will hold the input comparison count.
/// @return The identity rotation if no rotation can be found. Otherwise, the rotation from our current benchmark to
/// the catalog.
Rotation BaseTriangle::e_attitude () {
    *parameters.nu = 0;
    
    // There exists |input| choose 3 possibilities.
    for (int i = 0; i < (signed) input.size() - 2; i++) {
        for (int j = i + 1; j < (signed) input.size() - 1; j++) {
            for (int k = j + 1; k < (signed) input.size(); k++) {
                std::vector<Trio::stars> candidate_trios;
                Trio::stars candidate_trio;
                Star::list candidates;
                (*parameters.nu)++;
                
                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (*parameters.nu > parameters.nu_max) {
                    return {};
                }
                
                // Find matches of current body trio to catalog. Pivot if necessary.
                candidate_trio = pivot({(double) i, (double) j, (double) k});
                if (candidate_trio[0] == Star::zero() && candidate_trio[1] == Star::zero()
                    && candidate_trio[2] == Star::zero()) {
                    break;
                }
                
                // Find candidate stars around the candidate trio.
                candidates = ch.nearby_hip_stars(candidate_trio[0], fov, (unsigned int) (3.0 * input.size()));
                
                // Find the most likely rotation given the two pairs.
                return e_alignment(candidates, candidate_trio, {input[i], input[j], input[k]});
            }
        }
    }
    return Rotation::identity();
}

/// Match the stars found in the current benchmark to those in the Nibble database. The child class should wrap this
/// function as 'experiment_crown' to mimic the other methods.
///
/// @return Empty list if an image match cannot be found in "time". Otherwise, a vector of body stars with their
/// inertial catalog IDs that qualify as matches.
Star::list BaseTriangle::e_crown () {
    Star::list matches;
    *parameters.nu = 0;
    
    // There exists |input| choose 3 possibilities.
    for (int i = 0; i < (signed) input.size() - 2; i++) {
        for (int j = i + 1; j < (signed) input.size() - 1; j++) {
            for (int k = j + 1; k < (signed) input.size(); k++) {
                std::vector<Trio::stars> candidate_trios;
                Trio::stars candidate_trio;
                Star::list candidates;
                (*parameters.nu)++;
                
                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (*parameters.nu > parameters.nu_max) {
                    return {};
                }
                
                // Find matches of current body trio to catalog. Pivot if necessary.
                candidate_trio = pivot({(double) i, (double) j, (double) k});
                if (candidate_trio[0] == Star::zero() && candidate_trio[1] == Star::zero()
                    && candidate_trio[2] == Star::zero()) {
                    break;
                }
                
                // Find candidate stars around the candidate trio.
                candidates = ch.nearby_hip_stars(candidate_trio[0], fov, (unsigned int) (3.0 * input.size()));
                
                // Check all possible configurations. Return the most likely.
                matches = check_assumptions(candidates, candidate_trio, {(double) i, (double) j, (double) k});
                
                // Definition of image match: |match| > gamma. Break early.
                if (matches.size() > parameters.gamma) {
                    return matches;
                }
            }
        }
    }
    
    // Return an empty list if nothing is found.
    return {};
}