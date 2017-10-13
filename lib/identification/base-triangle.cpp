/// @file base-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for BaseTriangle class, which holds all common methods between the Planar and Spherical triangle
/// methods (they are very similar).

#include "identification/base-triangle.h"

/// Find the best matching pair using the appropriate PLAN table and by comparing areas and polar moments. Assumes
/// noise is normally distributed, searches using epsilon (3 * sigma_a) and K-Vector query.
///
/// **REQUIRES create_k_vector TO HAVE BEEN RUN PRIOR TO THIS METHOD**
///
/// @param a Area (planar or spherical) to search with.
/// @param i_t Polar moment (planar or spherical) to search with.
/// @return [-1][-1][-1] if no candidates found. Otherwise, all elements that met the criteria.
std::vector<BaseTriangle::label_trio> BaseTriangle::query_for_trio (const double a, const double i) {
    double epsilon_a = 3.0 * this->parameters.sigma_a;
    double epsilon_i = 3.0 * this->parameters.sigma_i;
    std::vector<label_trio> area_moment_match = {{-1, -1, -1}};
    hr_list area_match;
    
    // First, search for trio of stars matching area condition.
    ch.select_table(this->parameters.table_name);
    area_match = ch.k_vector_query("a", "hr_a, hr_b, hr_c, i", a - epsilon_a, a + epsilon_a,
                                   this->parameters.query_expected);
    
    // Next, search this trio for stars matching the moment condition.
    area_moment_match.reserve(area_match.size() / 4);
    for (unsigned int m = 0; m < area_match.size() / 4; m++) {
        Chomp::tuple t = ch.table_results_at(area_match, 4, m);
        
        if (t[3] >= i - epsilon_i && t[3] < i + epsilon_i) {
            area_moment_match.push_back({t[0], t[1], t[2]});
        }
    }
    
    // If results are found, remove the initialized value of [-1][-1][-1].
    if (area_moment_match.size() > 1) {
        area_moment_match.erase(area_moment_match.begin());
    }
    return area_moment_match;
}

/// Match the stars in the given set {B_1, B_2, B_3} to a trio in the database. If a past_set is given, then remove
/// all stars found matching the B trio that aren't found in the past set. Recurse until one definitive trio exists.
///
/// @param i_b Index trio of stars in body (B) frame.
/// @param past_set Matches found in a previous search.
/// @return A trio of stars that match the given B stars to R stars.
Trio::stars BaseTriangle::pivot (const index_trio &i_b, const std::vector<Trio::stars> &past_set) {
    std::vector<Trio::stars> matches = this->match_stars(i_b);
    
    // Function to increment hr_b3 first, then hr_b2, then hr_b1 last. This is the "pivoting".
    auto increment_hr = [&matches] (const index_trio &i_t) {
        if (i_t[2] != matches.size() - 1) {
            return index_trio {i_t[0], i_t[1], i_t[2] + 1};
        }
        else if (i_t[1] != matches.size() - 1) {
            return index_trio {i_t[0], i_t[1] + 1, 0};
        }
        else {
            return index_trio {i_t[0] + 1, 0, 0};
        }
    };
    
    // Remove all trios from matches that don't exist in the past set.
    if (!past_set.empty() && !(std::equal(past_set[0].begin() + 1, past_set[0].end(), past_set[0].begin()))) {
        for (unsigned int i = 0; i < matches.size(); i++) {
            bool match_found = false;
            for (const Trio::stars &past : past_set) {
                // We do not need to check all permutations. Break early and advance to next star.
                if (past[0] == matches[i][0] && past[1] == matches[i][1] && past[2] == matches[i][2]) {
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
        case 0: return pivot(increment_hr(i_b)); // No trios exists. Rerun with different trio.
        default: return pivot(increment_hr(i_b), matches); // 2+ trios exists. Run with different trio and past search.
    }
}

/// Rotate every point the given rotation and check if the angle of separation between any two stars is within a
/// given limit sigma.
///
/// @param candidates All stars to check against the body star set.
/// @param q The rotation to apply to all stars.
/// @return Set of matching stars found in candidates and the body sets.
Star::list BaseTriangle::rotate_stars (const Star::list &candidates, const Rotation &q) {
    Star::list matches, non_matched = this->input;
    double epsilon = 3.0 * this->parameters.match_sigma;
    matches.reserve(this->input.size());
    
    for (const Star &candidate : candidates) {
        Star r_prime = Rotation::rotate(candidate, q);
        for (unsigned int i = 0; i < non_matched.size(); i++) {
            if (Star::angle_between(r_prime, non_matched[i]) < epsilon) {
                // Add this match to the list by noting the candidate star's catalog ID number.
                matches.push_back(Star(non_matched[i][0], non_matched[i][1], non_matched[i][2], candidate.get_label()));
                
                // Remove the current star from the searching set. End the search for this star.
                non_matched.erase(non_matched.begin() + i);
                break;
            }
        }
    }
    
    return matches;
}

/// Check all possible configuration of star trios and return the set with the largest number of reference to body
/// matches.
///
/// @param candidates All stars to check against the body star set.
/// @param r Inertial (frame R) trio of stars to check against the body trio.
/// @param hr_b Body (frame B) Catalog IDs for the trio of stars to check against the inertial trio.
/// @return The largest set of matching stars across the body and inertial in all pairing configurations.
Star::list BaseTriangle::check_assumptions (const Star::list &candidates, const Trio::stars &r,
                                            const index_trio &hr_b) {
    index_trio current_order = {0, 1, 2};
    std::array<Trio::stars, 6> r_assumption_list;
    std::array<Star::list, 6> matches;
    int current_assumption = 0;
    
    // Generate unique permutations using previously generated trio.
    for (int i = 1; i < 6; i++) {
        r_assumption_list = {r[current_order[0]], r[current_order[1]], r[current_order[2]]};
        
        // Given i, swap elements 2 and 3 if even, or 1 and 3 if odd.
        current_order = (i % 2) == 0 ? index_trio {0, 2, 1} : index_trio {2, 1, 0};
    }
    
    // Determine the rotation to take frame R to B. Only use r_1 and r_2 to get rotation.
    for (const Trio::stars &assumption : r_assumption_list) {
        Rotation q = Rotation::rotation_across_frames({this->input[hr_b[0]], this->input[hr_b[1]]},
                                                      {assumption[0], assumption[1]});
        matches[current_assumption++] = rotate_stars(candidates, q);
    }
    
    // Return the larger of the six matches.
    return std::max_element(matches.begin(), matches.end(), [] (const Star::list &lhs, const Star::list &rhs) {
        return lhs.size() < rhs.size();
    })[0];
}

/// Check all possible configuration of star trios and return quaternion corresponding to the  set with the largest
/// number of reference to body matches. Unlike the method used in identification, this does not
/// return the larger star list, but rather the resulting attitude.
///
/// @param candidates All stars to check against the body star set.
/// @param r Inertial (frame R) trio of stars to check against the body trio.
/// @param hr_b Body (frame B) Catalog IDs for the trio of stars to check against the inertial trio.
/// @return The quaternion corresponding to largest set of matching stars across the body and inertial in all pairing
/// configurations.
Rotation BaseTriangle::trial_attitude_determine (const Star::list &candidates, const Trio::stars &r,
                                                 const Trio::stars &b) {
    index_trio current_order = {0, 1, 2};
    std::array<Trio::stars, 6> r_assumption_list;
    std::array<Star::list, 6> matches;
    std::array<Rotation, 6> q;
    
    // Generate unique permutations using previously generated trio.
    for (int i = 1; i < 6; i++) {
        r_assumption_list = {r[current_order[0]], r[current_order[1]], r[current_order[2]]};
        
        // Given i, swap elements 2 and 3 if even, or 1 and 3 if odd.
        current_order = (i % 2) == 0 ? index_trio {0, 2, 1} : index_trio {2, 1, 0};
    }
    
    // Determine the rotation to take frame R to B. Only use r_1 and r_2 to get rotation.
    for (unsigned int i = 0; i < r_assumption_list.size(); i++) {
        q[i] = Rotation::rotation_across_frames({b[0], b[1]}, {r_assumption_list[i][0], r_assumption_list[i][1]});
        matches[i] = rotate_stars(candidates, q[i]);
    }
    
    // Return quaternion corresponding to the largest match (messy lambda and iterator stuff below D:).
    return q[std::max_element(matches.begin(), matches.end(), [] (const Star::list &lhs, const Star::list &rhs) {
        return lhs.size() < rhs.size();
    }) - matches.begin()];
}

/// Match the stars found in the current benchmark to those in the Nibble database. The child class should wrap this
/// function as 'identify' to mimic the other methods.
///
/// @param z Reference to variable that will hold the input comparison count.
/// @return Vector of body stars with their inertial BSC IDs that qualify as matches.
Star::list BaseTriangle::identify_stars (unsigned int &z) {
    Star::list matches;
    z = 0;
    
    // There exists |input| choose 3 possibilities.
    for (int i = 0; i < (signed) input.size() - 2; i++) {
        for (int j = i + 1; j < (signed) input.size() - 1; j++) {
            for (int k = j + 1; k < (signed) input.size(); k++) {
                std::vector<Trio::stars> candidate_trios;
                Trio::stars candidate_trio;
                Star::list candidates;
                z++;
                
                // Find matches of current body trio to catalog.
                candidate_trios = match_stars({(double) i, (double) j, (double) k});
                if (candidate_trios[0][0] == Star::zero() && candidate_trios[0][0] == Star::zero()
                    && candidate_trios[0][2] == Star::zero()) {
                    break;
                }
                
                // Pivot if necessary.
                candidate_trio = pivot({(double) i, (double) j, (double) k}, candidate_trios);
                if (candidate_trio[0] == Star::zero() && candidate_trio[1] == Star::zero()
                    && candidate_trio[2] == Star::zero()) {
                    break;
                }
                
                // Find candidate stars around the candidate trio.
                candidates = (*q_root).nearby_stars(candidate_trio[0], fov, (unsigned int) (3.0 * input.size()));
                
                // Check all possible configurations. Return the most likely.
                matches = check_assumptions(candidates, candidate_trio, {(double) i, (double) j, (double) k});
                
                // Definition of image match: |match| > match minimum. Break early.
                if (matches.size() > parameters.match_minimum) {
                    return matches;
                }
            }
        }
    }
    
    // Return an empty list if nothing is found.
    return {};
}