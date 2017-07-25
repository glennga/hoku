/*
 * @file: angle.cpp
 *
 * @brief: Source file for Angle class, which matches a set of body vectors (stars) to their
 * inertial counter-parts in the database.
 */

#include "angle.h"

/*
 * Constructor. Sets the benchmark data, fov, and focus. Set the current working table to the
 * default 'SEP20'.
 *
 * @param input The set of benchmark data to work with.
 */
Angle::Angle(Benchmark input) {
    input.present_image(this->input, this->focus, this->fov);
    this->nb.select_table(AngleParameters().table_name);
}

/*
 * Generate the separation table given the specified FOV and table name. This finds the angle of
 * separation between each distinct permutation of stars, and only stores them if they fall within
 * the corresponding field-of-view.
 *
 * @param fov Field of view limit (degrees) that all pairs must be within.
 * @param table_name Name of the table to generate.
 * @return 0 when finished.
 */
int Angle::generate_sep_table(const int fov, const std::string &table_name) {
    Nibble nb;
    SQLite::Transaction transaction(*nb.db);
    nb.create_table(table_name, "hr_a INT, hr_b INT, theta FLOAT");
    nb.select_table(table_name);

    // (i, j) are distinct, where no (i, j) = (j, i)
    Nibble::bsc5_star_list all_stars = nb.all_bsc5_stars();
    for (unsigned int i = 0; i < all_stars.size() - 1; i++) {
        std::cout << "\r" << "Current *I* Star: " << all_stars[i].get_hr();
        for (unsigned int j = i + 1; j < all_stars.size(); j++) {
            double theta = Star::angle_between(all_stars[i], all_stars[j]);

            // only insert if angle between both stars is less than fov
            if (theta < fov) {
                nb.insert_into_table("hr_a, hr_b, theta", {(double) all_stars[i].get_hr(),
                                                           (double) all_stars[j].get_hr(), theta});
            }
        }
    }

    transaction.commit();
    return nb.polish_table("theta");
}

/*
 * Find the best matching pair using the appropriate SEP table and by comparing separation
 * angles. Assumes noise is normally distributed, searches using epsilon (3 * query_sigma).
 * Limits the amount returned by the search using 'query_limit'.
 *
 * @param theta Separation angle (degrees) to search with.
 * @return [-1][-1] if no candidates found. Two element array of the matching HR numbers
 * otherwise.
 */
Angle::hr_pair Angle::query_for_pair(const double theta) {
    // noise is normally distributed, angle within 3 sigma
    double epsilon = 3.0 * this->parameters.query_sigma, current_minimum = this->fov;
    unsigned int minimum_index = 0, limit = this->parameters.query_limit;
    std::ostringstream condition;
    hr_list candidates;

    // query using theta with epsilon bounds, return [-1][-1] if nothing found
    condition << "theta BETWEEN " << std::setprecision(16) << std::fixed;
    condition << theta - epsilon << " AND " << theta + epsilon;
    candidates = nb.search_table(condition.str(), "hr_a, hr_b, theta", limit * 3, limit);
    if (candidates.size() == 0) { return hr_pair {-1, -1}; }

    // select the candidate pair with the angle closest to theta
    for (unsigned int i = 0; i < candidates.size() / 3; i++) {
        hr_list inertial = nb.table_results_at(candidates, 3, i);

        // update with correct minimum
        if (fabs(inertial[2] - theta) < current_minimum) {
            current_minimum = inertial[2];
            minimum_index = i;
        }
    }

    // return the set with the angle closest to theta
    return hr_pair {(int) nb.table_results_at(candidates, 3, minimum_index)[0],
                    (int) nb.table_results_at(candidates, 3, minimum_index)[1]};
}

/*
 * Given a set of body (frame B) stars, find the matching inertial (frame R) stars.
 *
 * @param b_a Star A in frame B to find the match for.
 * @param b_b Star B in frame B to find the match for.
 * @return Array of vectors with 0 length if no matching pair is found. Otherwise, two inertial
 * stars that match the given body.
 */
Angle::star_pair Angle::find_candidate_pair(const Star &b_a, const Star &b_b) {
    double theta = Star::angle_between(b_a, b_b);
    hr_pair candidates;

    // greater than current field of view, must break
    if (theta > this->fov) { return {Star(), Star()}; }

    // no candidates found, must break
    candidates = this->query_for_pair(theta);
    if (candidates[0] == -1 && candidates[1] == -1) { return {Star(), Star()}; }

    // obtain inertial vectors for given candidates
    return {nb.query_bsc5(candidates[0]), nb.query_bsc5(candidates[1])};
}

/*
 * Rotate every point the given rotation and check if the angle of separation between any two
 * stars is within a given limit sigma.
 *
 * @param candidates All stars to check against the body star set.
 * @param q The rotation to apply to all stars.
 * @return Set of matching stars found in candidates and the body sets.
 */
Angle::star_list Angle::find_matches(const star_list &candidates, const Rotation &q) {
    star_list matches, non_matched = this->input;
    double epsilon = 3.0 * this->parameters.match_sigma;
    matches.reserve(this->input.size());

    for (const Star &candidate : candidates) {
        Star r_prime = Rotation::rotate(candidate, q);

        for (unsigned int i = 0; i < non_matched.size(); i++) {
            double theta = Star::angle_between(r_prime, non_matched[i]);

            if (theta < epsilon) {
                // add to list the body star with the candidate star's HR number
                matches.push_back(Star(non_matched[i][0], non_matched[i][1],
                                       non_matched[i][2], candidate.get_hr()));

                // remove the current star from the searching set, break early
                non_matched.erase(non_matched.begin() + i);
                break;
            }
        }
    }

    return matches;
}

/*
 * Find the best fitting match of input stars (frame B) to database stars (frame R) using the given
 * pair as reference. Both possible configurations are searched for:
 *
 * Assumption One: B_a = R_a, B_b = R_b
 * Assumption Two: B_a = R_b, B_b = R_a
 *
 * @param candidates All stars found near the inertial pair.
 * @param r Inertial (frame R) pair of stars that match the body pair.
 * @param b Body (frame B) pair of stars that match the inertial pair.
 * @return The largest set of matching stars across the body and inertial in both pairing
 * configurations.
 */
Angle::star_list Angle::check_assumptions(const std::vector<Star> &candidates,
                                          const star_pair &r, const star_pair &b) {
    std::array<star_pair, 2> assumption_list = {r, {r[1], r[0]}};
    std::array<star_list, 2> matches;
    int current_assumption = 0;

    // determine rotation to take frame B to A
    for (const star_pair &assumption : assumption_list) {
        Rotation q = Rotation::rotation_across_frames(b, assumption);
        matches[current_assumption++] = this->find_matches(candidates, q);
    }

    // return the larger of the two matches
    return matches[0].size() > matches[1].size() ? matches[0] : matches[1];
}

/*
 * Match the stars found in the given benchmark to those in the Nibble database.
 *
 * @param input The set of benchmark data to work with.
 * @param parameters Adjustments to the identification process.
 * @return Vector of body stars with their inertial BSC IDs that qualify as matches.
 */
Benchmark::star_list Angle::identify(const Benchmark &input, const AngleParameters &parameters) {
    bool matched = false;
    star_list matches;
    Angle a(input);

    a.parameters = parameters;
    a.nb.select_table(a.parameters.table_name);

    // |A_input| choose 2 possibilities, starts with closest stars to focus
    for (unsigned int i = 0; i < a.input.size() - 1; i++) {
        for (unsigned int j = i + 1; j < a.input.size(); j++) {
            star_list candidates;

            // narrow down current pair to two stars in catalog, order currently unknown
            star_pair candidate_pair = a.find_candidate_pair(a.input[i], a.input[j]);
            if (Star::is_equal(candidate_pair[0], Star()) &&
                Star::is_equal(candidate_pair[1], Star())) { break; }

            // find candidate stars around the candidate pair
            candidates = a.nb.nearby_stars(candidate_pair[0], a.fov, 3 * (a.input.size()));

            // check both possible configurations, return the most likely
            matches = a.check_assumptions(candidates, candidate_pair, {a.input[i], a.input[j]});

            // definition of image match: |match| > match minimum
            if (matches.size() > a.parameters.match_minimum) {
                matched = true;
                break;
            }
        }

        // break early if matched is met
        if (matched) break;
    }

    return matches;
}
