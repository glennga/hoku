/// @file spherical-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for SphericalTriangle class, which matches a set of body vectors (stars) to their inertial counter-parts
/// in the database.

#include "spherical-triangle.h"

/// Constructor. Sets the benchmark data, fov, and focus. Sets the parameters and working table. Constructs the
/// quadtree and saves the root.
///
/// @param input Working Benchmark instance. We are **only** copying the star set, focus star, and the fov.
/// @param parameters Parameters to use for identification.
SphericalTriangle::SphericalTriangle (const Benchmark &input, const Parameters &parameters) {
    input.present_image(this->input, this->focus, this->fov);
    this->parameters = parameters;
    
    ch.select_table(this->parameters.table_name);
    q_root = std::make_shared<QuadNode>(QuadNode::load_tree(this->parameters.bsc5_quadtree_w));
}

/// Generate the triangle table given the specified FOV and table name. This find the spherical area and polar moment
/// between each distinct permutation of trios, and only stores them if they fall within the corresponding
/// field-of-view.
///
/// @param fov Field of view limit (degrees) that all pairs must be within.
/// @param table_name Name of the table to generate.
/// @return 0 when finished.
int Sphere::generate_triangle_table (const int fov, const std::string &table_name) {
    Nibble nb;
    SQLite::Transaction initial_transaction(*nb.db);
    
    nb.create_table(table_name, "hr_a INT, hr_b INT, hr_c INT, a FLOAT, i FLOAT");
    initial_transaction.commit();
    nb.select_table(table_name);
    
    // (i, j, k) are distinct, where no (i, j, k) = (j, k, i), (j, i, k), ....
    Star::list all_stars = nb.all_bsc5_stars();
    for (unsigned int i = 0; i < all_stars.size() - 2; i++) {
        SQLite::Transaction transaction(*nb.db);
        std::cout << "\r" << "Current *A* Star: " << all_stars[i].get_hr();
        for (unsigned int j = i + 1; j < all_stars.size() - 1; j++) {
            for (unsigned int k = j + 1; k < all_stars.size(); k++) {
                
                // Only insert if the angle between all stars are separated by fov degrees or less.
                if (Star::angle_between(all_stars[i], all_stars[j]) < fov
                    && Star::angle_between(all_stars[j], all_stars[k]) < fov
                    && Star::angle_between(all_stars[k], all_stars[i]) < fov) {
                    double a_t = Trio::spherical_area(all_stars[i], all_stars[j], all_stars[k]);
                    double i_t = Trio::spherical_moment(all_stars[i], all_stars[j], all_stars[k],
                                                        Parameters().moment_td_h);
                    
                    nb.insert_into_table("hr_a, hr_b, hr_c, a, i",
                                         {(double) all_stars[i].get_hr(), (double) all_stars[j].get_hr(),
                                             (double) all_stars[k].get_hr(), a_t, i_t});
                }
            }
        }
        // Commit every star I change.
        transaction.commit();
    }
    
    // Create an index for area searches. We aren't searching for polar moments.
    return nb.polish_table("a");
}

/// Find the best matching pair using the appropriate SPHERE table and by comparing spherical areas and moments. Assumes
/// noise is normally distributed, searches using epsilon (3 * sigma_a) and K-Vector query.
///
/// **REQUIRES create_k_vector TO HAVE BEEN RUN PRIOR TO THIS METHOD**
///
/// @param a Spherical area to search with.
/// @param i_t Spherical polar moment to search with.
/// @return [-1][-1][-1] if no candidates found. Otherwise, all elements that met the criteria.
std::vector<Sphere::hr_trio> Sphere::query_for_trio (const double a, const double i) {
    double epsilon_a = 3.0 * this->parameters.sigma_a;
    double epsilon_i = 3.0 * this->parameters.sigma_i;
    std::vector<hr_trio> area_moment_match = {{-1, -1, -1}};
    hr_list area_match;
    
    // First, search for trio of stars matching area condition.
    ch.select_table(this->parameters.table_name);
    area_match = ch.k_vector_query("a", "hr_a, hr_b, hr_c, i", a - epsilon_a, a + epsilon_a, parameters.query_expected);
    
    // Next, search this trio for stars matching the moment condition.
    area_moment_match.reserve(area_match.size() / 4);
    for (unsigned int m = 0; m < area_match.size() / 4; m++) {
        Chomp::sql_row t = ch.table_results_at(area_match, 4, m);
        
        if (t[3] <= i - epsilon_i && t[3] > i + epsilon_i) {
            area_moment_match.push_back({t[0], t[1], t[2]});
        }
    }
    
    // If results are found, remove the initialized value of [-1][-1][-1].
    if (area_moment_match.size() > 1) {
        area_moment_match.erase(area_moment_match.begin());
    }
    return area_moment_match;
}

/// Given a trio of body stars, find matching trios of inertial stars using their respective spherical areas and polar
/// moments.
///
/// @param hr_b Index trio of stars in body (B) frame.
/// @return 1D vector of a trio of Star(0, 0, 0) if stars are not within the fov or if no matches currently exist.
/// Otherwise, vector of trios whose areas and moments are close.
std::vector<Trio::stars> Sphere::match_stars (const index_trio &hr_b) {
    std::vector<hr_trio> match_hr;
    std::vector<Trio::stars> matched_stars;
    Trio::stars b_stars{this->input[hr_b[0]], this->input[hr_b[1]], this->input[hr_b[2]]};
    
    // Do not attempt to find mathes if all stars are not within fov.
    if (Star::angle_between(b_stars[0], b_stars[1]) > this->fov
        || Star::angle_between(b_stars[1], b_stars[2]) > this->fov
        || Star::angle_between(b_stars[2], b_stars[0]) > this->fov) {
        return {{Star::zero(), Star::zero(), Star::zero()}};
    }
    
    // Search for the current trio. If this is empty, then break early.
    match_hr = this->query_for_trio(Trio::spherical_area(b_stars[0], b_stars[1], b_stars[2]),
                                    Trio::spherical_moment(b_stars[0], b_stars[1], b_stars[2], parameters.moment_td_h));
    if (match_hr[0][0] == -1 && match_hr[0][1] == -1 && match_hr[0][2] == -1) {
        return {{Star::zero(), Star::zero(), Star::zero()}};
    }
    
    // Grab stars themselves from HR numbers found in matches. Return these matches.
    matched_stars.reserve(match_hr.size());
    for (const hr_trio &t : match_hr) {
        matched_stars.push_back({ch.query_bsc5((int) t[0]), ch.query_bsc5((int) t[1]), ch.query_bsc5((int) t[2])});
    }
    
    return matched_stars;
}

/// Match the stars in the given set {B_1, B_2, B_3} to a trio in the database. If a past_set is given, then remove
/// all stars found matching the B trio that aren't found in the past set. Recurse until one definitive trio exists.
///
/// @param hr_b Index trio of stars in body (B) frame.
/// @param past_set Matches found in a previous search.
/// @return A trio of stars that match the given B stars to R stars.
Trio::stars Sphere::pivot (const index_trio &hr_b, const std::vector<Trio::stars> &past_set) {
    std::vector<Trio::stars> matches = this->match_stars(hr_b);
    
    // Function to increment hr_b3 first, then hr_b2, then hr_b1 last. This is the "pivoting".
    auto increment_hr = [matches] (const index_trio &hr_t) {
        if (hr_t[2] != matches.size() - 1) {
            return index_trio {hr_t[0], hr_t[1], hr_t[2] + 1};
        }
        else if (hr_t[1] != matches.size() - 1) {
            return index_trio {hr_t[0], hr_t[1] + 1, 0};
        }
        else {
            return index_trio {hr_t[0] + 1, 0, 0};
        }
    };
    
    // Remove all trios from matches that don't exist in the past set.
    if (past_set.size() > 0) {
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
        case 0: return pivot(increment_hr(hr_b)); // No trios exists. Rerun with different trio.
        default: return pivot(increment_hr(hr_b), matches); // 2+ trios exists. Run with different trio and past search.
    }
}

/// Rotate every point the given rotation and check if the angle of separation between any two stars is within a
/// given limit sigma.
///
/// @param candidates All stars to check against the body star set.
/// @param q The rotation to apply to all stars.
/// @return Set of matching stars found in candidates and the body sets.
Star::list Sphere::find_matches (const Star::list &candidates, const Rotation &q) {
    Star::list matches, non_matched = this->input;
    double epsilon = 3.0 * this->parameters.match_sigma;
    matches.reserve(this->input.size());
    
    for (const Star &candidate : candidates) {
        Star r_prime = Rotation::rotate(candidate, q);
        for (unsigned int i = 0; i < non_matched.size(); i++) {
            if (Star::angle_between(r_prime, non_matched[i]) < epsilon) {
                // Add this match to the list by noting the candidate star's HR number.
                matches.push_back(Star(non_matched[i][0], non_matched[i][1], non_matched[i][2], candidate.get_hr()));
                
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
/// @param hr_b Body (frame B) HR numbers for the trio of stars to check against the inertial trio.
/// @return The largest set of matching stars across the body and inertial in all pairing configurations.
Star::list Sphere::check_assumptions (const Star::list &candidates, const Trio::stars &r, const index_trio &hr_b) {
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
        matches[current_assumption++] = find_matches(candidates, q);
    }
    
    // Return the larger of the six matches.
    return std::max_element(matches.begin(), matches.end(), [] (const Star::list &lhs, const Star::list &rhs) {
        return lhs.size() < rhs.size();
    })[0];
}

/// Match the stars found in the given benchmark to those in the Nibble database.
///
/// @param input The set of benchmark data to work with.
/// @param parameters Adjustments to the identification process.
/// @return Vector of body stars with their inertial BSC IDs that qualify as matches.
Star::list Sphere::identify (const Benchmark &input, const Parameters &parameters) {
    Star::list matches;
    bool matched = false;
    Sphere s(input, parameters);
    s.parameters = parameters;
    
    // There exists |S_input| choose 3 possibilities.
    for (unsigned int i = 0; i < s.input.size() - 2; i++) {
        for (unsigned int j = i + 1; j < s.input.size() - 1; j++) {
            for (unsigned int k = j + 1; k < s.input.size(); k++) {
                std::vector<Trio::stars> candidate_trios;
                Trio::stars candidate_trio;
                Star::list candidates;
                
                // Find matches of current body trio to catalog. Pivot if necessary.
                candidate_trios = s.match_stars({(double) i, (double) j, (double) k});
                candidate_trio = s.pivot({(double) i, (double) j, (double) k}, candidate_trios);
                if (candidate_trio[0] == Star() && candidate_trio[1] == Star() && candidate_trio[2] == Star()) {
                    break;
                }
                
                // Find candidate stars around the candidate trio.
                candidates = (*s.q_root).nearby_stars(candidate_trio[0], s.fov, 3 * s.input.size());
                
                // Check all possible configurations. Return the most likely.
                matches = s.check_assumptions(candidates, candidate_trio, {(double) i, (double) j, (double) k});
                
                // Definition of image match: |match| > match minimum.
                if (matches.size() > s.parameters.match_minimum) {
                    matched = true;
                    break;
                }
            }
        }
        
        // Break early if the matched condition is met.
        if (matched) {
            break;
        }
    }
    
    return matches;
}