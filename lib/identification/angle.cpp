/// @file angle.cpp
/// @author Glenn Galvizo
///
/// Source file for Angle class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#include "identification/angle.h"

/// Constructor. Sets the benchmark data, fov, parameters, and current working table.
///
/// @param input Working Benchmark instance. We are **only** copying the star set and the fov.
Angle::Angle (const Benchmark &input, const Parameters &p) {
    input.present_image(this->input, this->fov);
    this->parameters = p;
    
    this->ch.select_table(parameters.table_name);
}

/// Generate the separation table given the specified FOV and table name. This finds the angle of separation between
/// each distinct permutation of stars, and only stores them if they fall within the corresponding field-of-view.
///
/// @param fov Field of view limit (degrees) that all pairs must be within.
/// @param table_name Name of the table to generate.
/// @return 0 when finished.
int Angle::generate_sep_table (const double fov, const std::string &table_name) {
    Chomp ch;
    SQLite::Transaction transaction(*ch.db);
    ch.create_table(table_name, "label_a INT, label_b INT, theta FLOAT");
    ch.select_table(table_name);
    
    // (i, j) are distinct, where no (i, j) = (j, i).
    Star::list all_stars = ch.bright_as_list();
    for (unsigned int i = 0; i < all_stars.size() - 1; i++) {
        std::cout << "\r" << "Current *I* Star: " << all_stars[i].get_label();
        for (unsigned int j = i + 1; j < all_stars.size(); j++) {
            double theta = Star::angle_between(all_stars[i], all_stars[j]);
            
            // Only insert if the angle between both stars is less than fov.
            if (theta < fov) {
                ch.insert_into_table("label_a, label_b, theta", Nibble::tuple_d {(double) all_stars[i].get_label(),
                    (double) all_stars[j].get_label(), theta});
            }
        }
    }
    
    transaction.commit();
    return ch.polish_table("theta");
}

/// Find the best matching pair using the appropriate SEP table and by comparing separation angles. Assumes noise is
/// normally distributed, searches using epsilon (3 * query_sigma). Limits the amount returned by the  search using
/// 'query_limit'.
///
/// @param theta Separation angle (degrees) to search with.
/// @return [-1][-1] if no candidates found. Two element array of the matching catalog IDs otherwise.
Angle::label_pair Angle::query_for_pair (const double theta) {
    // Noise is normally distributed. Angle within 3 sigma of theta.
    double epsilon = 3.0 * this->parameters.query_sigma;
    unsigned int limit = this->parameters.query_limit;
    std::ostringstream condition;
    Nibble::tuples_d candidates;
    
    // Query using theta with epsilon bounds. Return [-1][-1] if nothing is found.
    ch.select_table(parameters.table_name);
    condition << "theta BETWEEN " << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed;
    condition << theta - epsilon << " AND " << theta + epsilon;
    candidates = ch.search_table("label_a, label_b, theta", condition.str(), limit * 3, limit);
    if (candidates.empty()) {
        return label_pair {-1, -1};
    }
    
    // Select the candidate pair with the angle closest to theta.
    Nibble::tuple_d minimum_tuple = {0, 0, this->fov};
    for (const Nibble::tuple_d &inertial : candidates) {
        
        // Update with the correct minimum.
        if (fabs(inertial[2] - theta) < minimum_tuple[2]) {
            minimum_tuple = inertial;
        }
    }
    
    // Return the set with the angle closest to theta.
    return label_pair {(int) minimum_tuple[0], (int) minimum_tuple[1]};
}

/// Given a set of body (frame B) stars, find the matching inertial (frame R) stars.
///
/// @param b_a Star A in frame B to find the match for.
/// @param b_b Star B in frame B to find the match for.
/// @return Array of vectors with 0 length if no matching pair is found. Otherwise, two inertial stars that match
/// the given body.
Star::pair Angle::find_candidate_pair (const Star &b_a, const Star &b_b) {
    double theta = Star::angle_between(b_a, b_b);
    
    // If the current angle is greater than the current fov, break early.
    if (theta > this->fov) {
        return {Star::zero(), Star::zero()};
    }
    
    // If no candidate is found, break early.
    label_pair candidates = this->query_for_pair(theta);
    if (candidates[0] == -1 && candidates[1] == -1) {
        return {Star::zero(), Star::zero()};
    }
    
    // Otherwise, obtain and return the inertial vectors for the given candidates.
    return {ch.query_hip(candidates[0]), ch.query_hip(candidates[1])};
}

/// Rotate every point the given rotation and check if the angle of separation between any two stars is within a
/// given limit sigma.
///
/// @param candidates All stars to check against the body star set.
/// @param q The rotation to apply to all stars.
/// @return Set of matching stars found in candidates and the body sets.
Star::list Angle::find_matches (const Star::list &candidates, const Rotation &q) {
    Star::list matches, non_matched = this->input;
    double epsilon = 3.0 * this->parameters.match_sigma;
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

/// Find the best fitting match of input stars (frame B) to database stars (frame R) using the given pair as reference.
/// Both possible configurations are searched for:
///
/// Assumption One: B_a = R_a, B_b = R_b
/// Assumption Two: B_a = R_b, B_b = R_a
///
/// @param candidates All stars found near the inertial pair.
/// @param r Inertial (frame R) pair of stars that match the body pair.
/// @param b Body (frame B) pair of stars that match the inertial pair.
/// @return The largest set of matching stars across the body and inertial in both pairing configurations.
Star::list Angle::check_assumptions (const Star::list &candidates, const Star::pair &r, const Star::pair &b) {
    std::array<Star::pair, 2> assumption_list = {r, {r[1], r[0]}};
    std::array<Star::list, 2> matches;
    int current_assumption = 0;
    
    // Determine the rotation to take frame B to A, find all matches with this rotation.
    for (const Star::pair &assumption : assumption_list) {
        Rotation q = Rotation::rotation_across_frames(b, assumption);
        matches[current_assumption++] = this->find_matches(candidates, q);
    }
    
    // Return the larger of the two matches.
    return matches[0].size() > matches[1].size() ? matches[0] : matches[1];
}

/// Match the stars found in the given benchmark to those in the Nibble database.
///
/// @param input The set of benchmark data to work with.
/// @param parameters Adjustments to the identification process.
/// @param z Reference to variable that will hold the input comparison count.
/// @return Empty list if an image match cannot be found in "time". Otherwise, a vector of body stars with their
/// inertial catalog IDs that qualify as matches.
Star::list Angle::identify (const Benchmark &input, const Parameters &parameters, unsigned int &z) {
    Star::list matches;
    Angle a(input, parameters);
    z = 0;
    
    // There exists |A_input| choose 2 possibilities.
    for (int i = 0; i < (signed) a.input.size() - 1; i++) {
        for (int j = i + 1; j < (signed) a.input.size(); j++) {
            Star::list candidates;
            z++;
            
            // Narrow down current pair to two stars in catalog. The order is currently unknown.
            Star::pair candidate_pair = a.find_candidate_pair(a.input[i], a.input[j]);
            if (Star::is_equal(candidate_pair[0], Star()) && Star::is_equal(candidate_pair[1], Star())) {
                break;
            }
            
            // Find candidate stars around the candidate pair.
            candidates = a.ch.nearby_hip_stars(candidate_pair[0], a.fov, 3 * ((unsigned int) a.input.size()));
            
            // Check both possible configurations. Return the most likely of the two.
            matches = a.check_assumptions(candidates, candidate_pair, {a.input[i], a.input[j]});
            
            // Practical limit: exit early if we have iterated through too many comparisons without match.
            if (z > a.parameters.z_max) {
                return {};
            }
            
            // Definition of image match: |match| > match minimum OR |match| == |input|.
            if (matches.size() > a.parameters.match_minimum || matches.size() == a.input.size()) {
                return matches;
            }
        }
    }
    
    // Return an empty list if no matches found.
    return matches;
}

/// Overloaded to not include the comparison count parameter. Match the stars found in the given benchmark to those in
/// the Nibble database.
///
/// @param input The set of benchmark data to work with.
/// @param parameters Adjustments to the identification process.
/// @return Vector of body stars with their inertial BSC IDs that qualify as matches.
Star::list Angle::identify (const Benchmark &input, const Parameters &parameters) {
    unsigned int z = 0;
    return Angle::identify(input, parameters, z);
}

/// Reproduction of the Angle method's Nibble querying. Unlike the one used in identification, this does not restrict
/// our results (no LIMIT in query) and does not return the most likely match. Messy, but gets the job done. (:
///
/// **Need to select the proper table before calling this method.**
///
/// @param ch Open Nibble connection with Chomp.
/// @param s_1 Star one to query with.
/// @param s_2 Star two to query with.
/// @param query_sigma Theta must be within 3 * query_sigma to appear in results.
/// @return Vector of likely matches found by the angle method.
std::vector<Angle::label_pair> Angle::trial_query (Chomp &ch, const Star &s_1, const Star &s_2,
                                                   const double query_sigma) {
    double epsilon = 3.0 * query_sigma, theta = Star::angle_between(s_1, s_2);
    std::ostringstream condition;
    std::vector<Angle::label_pair> r_bar;
    
    // Query using theta with epsilon bounds.
    condition << "theta BETWEEN " << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed;
    condition << theta - epsilon << " AND " << theta + epsilon;
    Nibble::tuples_d r = ch.search_table("label_a, label_b", condition.str(), 500);
    
    // Sort tuple_d into list of catalog ID pairs.
    r_bar.reserve(r.size() / 2);
    for (const Nibble::tuple_d &r_t : r) {
        r_bar.emplace_back(Angle::label_pair {(int) r_t[0], (int) r_t[1]});
    }
    
    return r_bar;
}

/// Reproduction of the Angle method's querying to candidate reduction step. This returns the first match found by our
/// trial_query method.
///
/// **Need to select the proper table before calling this method.**
///
/// @param ch Open Nibble connection with Chomp.
/// @param s_1 Star one to query with.
/// @param s_2 Star two to query with.
/// @param query_sigma Theta must be within 3 * query_sigma to appear in results.
/// @return [0, 0] if no match is found. Otherwise, a single match for the given stars s_1 and s_2.
Angle::label_pair Angle::trial_reduction (Chomp &ch, const Star &s_1, const Star &s_2, double query_sigma) {
    std::vector<label_pair> p = trial_query(ch, s_1, s_2, query_sigma);
    return p.empty() ? (label_pair) {0, 0} : p[0];
}

/// Reproduction of the Angle method's check_assumption. Unlike the method used in identification, this does not
/// return the larger star list, but rather the resulting attitude.
///
/// @param candidates All stars found near the inertial pair.
/// @param r Inertial (frame R) pair of stars that match the body pair.
/// @param b Body (frame B) pair of stars that match the inertial pair.
/// @return The quaternion associated with the largest set of matching stars across the body and inertial in both
/// pairing configurations.
Rotation Angle::trial_attitude_determine (const Star::list &candidates, const Star::pair &r, const Star::pair &b) {
    std::array<Star::pair, 2> assumption_list = {r, {r[1], r[0]}};
    std::array<Star::list, 2> matches;
    std::array<Rotation, 2> q;
    
    // Determine the rotation to take frame B to A, find all matches with this rotation.
    for (unsigned int i = 0; i < assumption_list.size(); i++) {
        q[i] = Rotation::rotation_across_frames(b, assumption_list[i]);
        matches[i] = this->find_matches(candidates, q[i]);
    }
    
    // Return the rotation associated with the largest set of matches.
    return matches[0].size() > matches[1].size() ? q[0] : q[1];
}

/// Reproduction of the Angle method's process from beginning to the orientation determination. Unlike our main
/// identify method, this does not perform our match step.
///
/// @param input The set of benchmark data to work with.
/// @param parameters Adjustments to the identification process.
/// @param z Reference to variable that will hold the input comparison count.
/// @return Identity quaternion if no rotation can be found. Otherwise, the resulting rotation after our attitude
/// determination step.
Rotation Angle::trial_semi_crown (const Benchmark &input, const Parameters &parameters, unsigned int &z) {
    Angle a(input, parameters);
    z = 0;
    
    // There exists |A_input| choose 2 possibilities.
    for (int i = 0; i < (signed) a.input.size() - 1; i++) {
        for (int j = i + 1; j < (signed) a.input.size(); j++) {
            Star::list candidates;
            z++;
            
            // Narrow down current pair to two stars in catalog. The order is currently unknown.
            Star::pair candidate_pair = a.find_candidate_pair(a.input[i], a.input[j]);
            if (Star::is_equal(candidate_pair[0], Star()) && Star::is_equal(candidate_pair[1], Star())) {
                break;
            }
            
            // Find candidate stars around the candidate pair.
            candidates = a.ch.nearby_hip_stars(candidate_pair[0], a.fov, 3 * ((unsigned int) a.input.size()));
    
            // Find the most likely rotation given the two pairs.
            return a.trial_attitude_determine(candidates, candidate_pair, {a.input[i], a.input[j]});
        }
    }
    
    return Rotation::identity();
}