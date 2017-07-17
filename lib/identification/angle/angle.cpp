/*
 * @file: angle.cpp
 *
 * @brief: Source file for Angle class, which matches a set of body vectors (stars) to their
 * inertial counter-parts in the database.
 */

#include "angle.h"

/*
 * Constructor. Sets the benchmark data, fov, and focus. Sort the input set.
 *
 * @param input The set of benchmark data to work with.
 */
Angle::Angle(Benchmark input) {
    this->input = input.present_stars();
    this->fov = input.get_fov();
    this->focus = input.get_focus();
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
    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    SQLite::Transaction transaction(db);
    db.exec("CREATE TABLE " + table_name + "(hr_a INT, hr_b INT, theta FLOAT)");

    // (i, j) are distinct, where no (i, j) = (j, i)
    std::array<Star, 5029> all_stars = Nibble::all_bsc5_stars();
    for (unsigned int i = 0; i < all_stars.size() - 1; i++) {
        std::cout << "\r" << "Current *I* Star: " << all_stars[i].get_hr();
        for (unsigned int j = i + 1; j < all_stars.size(); j++) {
            double theta = Star::angle_between(all_stars[i], all_stars[j]);

            // only insert if angle between both stars is less than fov
            if (theta < fov) {
                Nibble::insert_into_table(db, table_name, "hr_a, hr_b, theta",
                                          {(double) all_stars[i].get_hr(),
                                           (double) all_stars[j].get_hr(), theta});
            }
        }
    }

    transaction.commit();
    return Nibble::polish_table(table_name, "theta");
}

/*
 * Find the best matching pair using the appropriate SEP table and by comparing separation
 * angles. Assumes noise is normally distributed, searches using epsilon (3 * query_sigma).
 * Limits the amount returned by the search using 'query_limit'.
 *
 * @param db Database object currently open.
 * @param theta Separation angle (degrees) to search with.
 * @return [-1][-1] if no candidates found. Two element array of the matching HR numbers
 * otherwise.
 */
std::array<int, 2> Angle::query_for_pair(SQLite::Database &db, const double theta) {
    // noise is normally distributed, angle within 3 sigma
    double epsilon = 3.0 * this->parameters.query_sigma, current_minimum = this->fov;
    std::vector<double> candidates;
    std::ostringstream condition;
    int minimum_index = 0;

    // query using theta with epsilon bounds, return [-1][-1] if nothing found
    condition << "theta BETWEEN " << std::setprecision(16) << std::fixed;
    condition << theta - epsilon << " AND " << theta + epsilon;
    candidates = Nibble::search_table(db, this->parameters.table_name, condition.str(),
                                      "hr_a, hr_b, theta",
                                      (unsigned int) this->parameters.query_limit * 3,
                                      this->parameters.query_limit);
    if (candidates.size() == 0) { return std::array<int, 2> {-1, -1}; }

    // select the candidate pair with the angle closest to theta
    for (unsigned int i = 0; i < candidates.size() / 3; i++) {
        std::vector<double> inertial = Nibble::table_results_at(candidates, 3, i);

        // update with correct minimum
        if (fabs(inertial[2] - theta) < current_minimum) {
            current_minimum = inertial[2];
            minimum_index = i;
        }
    }

    // return the set with the angle closest to theta
    return std::array<int, 2> {(int) Nibble::table_results_at(candidates, 3, minimum_index)[0],
                               (int) Nibble::table_results_at(candidates, 3, minimum_index)[1]};
}

/*
 * Given a set of body (frame A) stars, find the matching inertial (frame B) stars.
 *
 * @param db Database object currently open.
 * @param a_a Star A in frame A to find the match for.
 * @param a_b Star B in frame A to find the match for.
 * @return Array of vectors with 0 length if no matching pair is found. Otherwise, two inertial
 * stars that match the given body.
 */
std::array<Star, 2> Angle::find_candidate_pair(SQLite::Database &db, const Star &a_a,
                                               const Star &a_b) {
    double theta = Star::angle_between(a_a, a_b);
    std::array<int, 2> candidates;

    // greater than current field of view, must break
    if (theta > this->fov) { return {Star(), Star()}; }

    // no candidates found, must break
    candidates = this->query_for_pair(db, theta);
    if (candidates[0] == -1 && candidates[1] == -1) { return {Star(0, 0, 0), Star(0, 0, 0)}; }

    // obtain inertial vectors for given candidates
    return {Nibble::query_bsc5(db, candidates[0]), Nibble::query_bsc5(db, candidates[1])};
}

/*
 * Rotate every point the given rotation and check if the angle of separation between any two
 * stars is within a given limit sigma.
 *
 * @param candidates All stars to check against the body star set.
 * @param q The rotation to apply to all stars.
 * @return Set of matching stars found in candidates and the body sets.
 */
std::vector<Star> Angle::find_matches(const std::vector<Star> &candidates, const Rotation &q) {
    std::vector<Star> matches, non_matched = this->input;
    double epsilon = 3.0 * this->parameters.match_sigma;
    matches.reserve(this->input.size());

    for (const Star &candidate : candidates) {
        Star b_prime = Rotation::rotate(candidate, q);

        for (unsigned int i = 0; i < non_matched.size(); i++) {
            double theta = Star::angle_between(b_prime, non_matched[i]);

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
 * Find the best fitting match of input stars (frame A) to database stars (frame B) using the given
 * pair as reference. Both possible configurations are searched for:
 *
 * Assumption One: A_a = B_a, A_b = B_b
 * Assumption Two: A_a = B_b, A_b = B_a
 *
 * @param candidates All stars found near the inertial pair.
 * @param b Inertial (frame B) pair of stars that match the body pair.
 * @param a Body (frame A) pair of stars that match the inertial pair.
 * @return The largest set of matching stars across the body and inertial in both pairing
 * configurations.
 */
std::vector<Star> Angle::check_assumptions(const std::vector<Star> &candidates,
                                           const std::array<Star, 2> &b,
                                           const std::array<Star, 2> &a) {
    std::array<std::array<Star, 2>, 2> assumption_list = {b, {b[1], b[0]}};
    std::array<std::vector<Star>, 2> matches;
    int current_assumption = 0;

    // determine rotation to take frame B to A
    for (const std::array<Star, 2> &assumption : assumption_list) {
        Rotation q = Rotation::rotation_across_frames(a, assumption);
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
std::vector<Star> Angle::identify(const Benchmark &input, const AngleParameters &parameters) {
    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    std::vector<Star> matches;
    bool matched = false;
    Angle a(input);
    a.parameters = parameters;

    // |A_input| choose 2 possibilities, starts with closest stars to focus
    for (unsigned int i = 0; i < a.input.size() - 1; i++) {
        for (unsigned int j = i + 1; j < a.input.size(); j++) {
            std::vector<Star> candidates;

            // narrow down current pair to two stars in catalog, order currently unknown
            std::array<Star, 2> candidate_pair =
                    a.find_candidate_pair(db, a.input[i], a.input[j]);
            if (Star::is_equal(candidate_pair[0], Star()) &&
                Star::is_equal(candidate_pair[1], Star())) { break; }

            // find candidate stars around the candidate pair
            candidates = Nibble::nearby_stars(db, candidate_pair[0], a.fov,
                                              3 * (a.input.size()));

            // check both possible configurations, return the most likely
            matches = a.check_assumptions(candidates, candidate_pair,
                                           {a.input[i], a.input[j]});

            // definition of image match: |match| > match minimum
            if (matches.size() > a.parameters.match_minimum) {
                matched = true;
                break;
            }
        }

        // break early if matched is met
        if (matched) { break; }
    }

    return matches;
}
