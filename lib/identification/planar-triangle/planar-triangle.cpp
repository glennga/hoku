/*
 * @file: triangleplanar.cpp
 *
 * @brief: Source file for Angle class, which matches a set of body vectors (stars) to their
 * inertial counter-parts in the database.
 */

#include "planar-triangle.h"

/*
 * Constructor. Sets the benchmark data, fov, and focus. Sort the input set.
 *
 * @param input The set of benchmark data to work with.
 */
PlanarTriangle::PlanarTriangle(Benchmark input) {
    this->input = input.present_stars();
    this->fov = input.get_fov();
    this->focus = input.get_focus();
}

/*
 * Generate the triangle table given the specified FOV and table name. This find the planar area
 * and polar moment between each distinct permutation of trios, and only stores them if they
 * fall within the corresponding field-of-view.
 *
 * @param fov Field of view limit (degrees) that all pairs must be within.
 * @param table_name Name of the table to generate.
 * @return 0 when finished.
 */
int PlanarTriangle::generate_triangle_table(const int fov, const std::string &table_name) {
    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    SQLite::Transaction initial_transaction(db);
    std::string fields = "star_a_number, star_b_number, star_c_number, area, moment";
    std::string schema = "star_a_number INT, star_b_number INT, star_c_number INT, "
            "area FLOAT, moment FLOAT";

    db.exec("CREATE TABLE " + table_name + " (" + schema + ")");
    initial_transaction.commit();

    // (a, b, c) are distinct, where no (a, b, c) = (b, c, a), (c, b, a), ....
    std::array<Star, 5029> all_stars = Nibble::all_bsc5_stars();
    for (unsigned int a = 0; a < all_stars.size() - 2; a++) {
        SQLite::Transaction transaction(db);
        std::cout << "\r" << "Current *A* Star: " << all_stars[a].get_bsc_id();
        for (unsigned int b = a + 1; b < all_stars.size() - 1; b++) {
            for (unsigned int c = b + 1; c < all_stars.size(); c++) {

                // only insert if all stars are within fov
                if (Star::angle_between(all_stars[a], all_stars[b]) < fov &&
                    Star::angle_between(all_stars[b], all_stars[c]) < fov &&
                    Star::angle_between(all_stars[c], all_stars[a]) < fov) {
                    double area = Trio::planar_area(all_stars[a], all_stars[b], all_stars[c]);
                    double moment = Trio::planar_moment(all_stars[a], all_stars[b], all_stars[c]);

                    Nibble::insert_into_table(db, table_name, fields,
                                              {(double) all_stars[a].get_bsc_id(),
                                               (double) all_stars[b].get_bsc_id(),
                                               (double) all_stars[c].get_bsc_id(), area, moment});
                }
            }
        }
        // commit every star A change
        transaction.commit();
    }

    return Nibble::polish_table(table_name, "star_a_number, star_b_number, star_c_number, area, "
            "moment", schema, "area");
}
//
///*
// * Find the best matching pair using the appropriate SEP table and by comparing separation
// * angles. Assumes noise is normally distributed, searches using epsilon (3 * query_sigma).
// * Limits the amount returned by the search using 'query_limit'.
// *
// * @param theta Separation angle (degrees) to search with.
// * @return [-1][-1] if no candidates found. Two element array of the matching BSC IDs otherwise.
// */
//std::array<int, 2> Angle::query_for_pair(const double theta) {
//    // noise is normally distributed, angle within 3 sigma
//    double epsilon = 3.0 * this->parameters.query_sigma, current_minimum = this->fov;
//    std::vector<double> candidates;
//    std::ostringstream condition;
//    int minimum_index = 0;
//
//    // query using theta with epsilon bounds, return [-1][-1] if nothing found
//    condition << "theta BETWEEN " << std::setprecision(16) << std::fixed;
//    condition << theta - epsilon << " AND " << theta + epsilon;
//    candidates = Nibble::search_table(this->parameters.table_name, condition.str(),
//                                      "star_a_number, star_b_number, theta",
//                                      (unsigned int) this->parameters.query_limit * 3,
//                                      this->parameters.query_limit);
//    if (candidates.size() == 0) { return std::array<int, 2> {-1, -1}; }
//
//    // select the candidate pair with the angle closest to theta
//    for (unsigned int a = 0; a < candidates.size() / 3; a++) {
//        std::vector<double> inertial = Nibble::table_results_at(candidates, 3, a);
//
//        // update with correct minimum
//        if (fabs(inertial[2] - theta) < current_minimum) {
//            current_minimum = inertial[2];
//            minimum_index = a;
//        }
//    }
//
//    // return the set with the angle closest to theta
//    return std::array<int, 2> {(int) Nibble::table_results_at(candidates, 3, minimum_index)[0],
//                               (int) Nibble::table_results_at(candidates, 3, minimum_index)[1]};
//}
//
///*
// * Given a set of body stars, find the matching inertial stars.
// *
// * @param db Database object currently open.
// * @param body_a Star A to find the match for.
// * @param body_b Star B to find the match for.
// * @return Array of vectors with 0 length if no matching pair is found. Otherwise, two inertial
// * stars that match the given body.
// */
//std::array<Star, 2> Angle::find_candidate_pair(SQLite::Database &db, const Star &body_a,
//                                               const Star &body_b) {
//    double theta = Star::angle_between(body_a, body_b);
//    std::array<double, 4> components_a, components_b;
//    std::array<int, 2> candidates;
//
//    // greater than current field of view, must break
//    if (theta > this->fov) {
//        return {Star(0, 0, 0), Star(0, 0, 0)};
//    }
//
//    // no candidates found, must break
//    candidates = this->query_for_pair(theta);
//    if (candidates[0] == -1 && candidates[1] == -1) {
//        return {Star(0, 0, 0), Star(0, 0, 0)};
//    }
//
//    // obtain inertial vectors for given candidates
//    components_a = Nibble::query_bsc5(db, candidates[0]);
//    components_b = Nibble::query_bsc5(db, candidates[1]);
//    return {Star(components_a[0], components_a[1], components_a[2], candidates[0]),
//            Star(components_b[0], components_b[1], components_b[2], candidates[1])};
//}
//
///*
// * Rotate every point the given rotation and check if the angle of separation between any two
// * stars is within a given limit sigma.
// *
// * @param candidates All stars to check against the body star set.
// * @param psi The rotation to apply to all stars.
// * @return Set of matching stars found in candidates and the body sets.
// */
//std::vector<Star> Angle::find_matches(const std::vector<Star> &candidates, const Rotation &psi) {
//    std::vector<Star> matches, current_input = this->input;
//    double epsilon = 3.0 * this->parameters.match_sigma;
//    matches.reserve(this->input.size());
//
//    for (const Star &candidate : candidates) {
//        Star as_body = Rotation::rotate(candidate, psi);
//
//        for (unsigned int a = 0; a < current_input.size(); a++) {
//            double theta = Star::angle_between(as_body, current_input[a]);
//
//            if (theta < epsilon) {
//                std::array<double, 3> match = current_input[a].components_as_array();
//                matches.push_back(Star(match[0], match[1], match[2], candidate.get_bsc_id()));
//
//                // remove the current star from the searching set, break early
//                current_input.erase(current_input.begin() + a);
//                break;
//            }
//        }
//    }
//
//    return matches;
//}
//
///*
// * Find the best fitting match of body stars (input) to inertial stars (database) using the given
// * pair as reference. Both possible configurations are searched for:
// *
// * Assumption One: body_a = inertial_a, body_b = inertial_b
// * Assumption Two: body_a = inertial_b, body_b = inertial_a
// *
// * @param candidates All stars found near the inertial pair.
// * @param inertial Inertial pair of stars that match the body pair.
// * @param body Body pair of stars that match the inertial pair.
// * @return The largest set of matching stars across the body and inertial in both pairing
// * configurations.
// */
//std::vector<Star> Angle::check_assumptions(const std::vector<Star> &candidates,
//                                           const std::array<Star, 2> &inertial,
//                                           const std::array<Star, 2> &body) {
//    std::array<std::array<Star, 2>, 2> assumption_list = {inertial, {inertial[1], inertial[0]}};
//    std::array<std::vector<Star>, 2> matches;
//    int current_assumption = 0;
//
//    // determine rotation to take inertial to body
//    for (const std::array<Star, 2> &assumption : assumption_list) {
//        Rotation psi = Rotation::rotation_across_frames(body, assumption);
//        matches[current_assumption++] = this->find_matches(candidates, psi);
//    }
//
//    // return the larger of the two matches
//    return matches[0].size() > matches[1].size() ? matches[0] : matches[1];
//}
//
///*
// * Match the stars found in the given benchmark to those in the Nibble database.
// *
// * @param input The set of benchmark data to work with.
// * @param parameters Adjustments to the identification process.
// * @return Vector of body stars with their inertial BSC IDs that qualify as matches.
// */
//std::vector<Star> Angle::identify(const Benchmark &input, const AngleParameters &parameters) {
//    Angle mu(input);
//    std::vector<Star> matches;
//    SQLite::Database db(Nibble::database_location, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
//    bool matched = false;
//    mu.parameters = parameters;
//
//    // |mu.input| choose 2 possibilities, starts with closest stars to focus
//    for (unsigned int a = 0; a < mu.input.size() - 1; a++) {
//        for (unsigned int b = a + 1; b < mu.input.size(); b++) {
//            std::vector<Star> candidates;
//
//            // narrow down current pair to two stars in catalog, order currently unknown
//            std::array<Star, 2> candidate_pair =
//                    mu.find_candidate_pair(db, mu.input[a], mu.input[b]);
//            if (Star::is_equal(candidate_pair[0], Star(0, 0, 0)) &&
//                Star::is_equal(candidate_pair[1], Star(0, 0, 0))) { break; }
//
//            // find candidate stars around the candidate pair
//            candidates = Nibble::nearby_stars(db, candidate_pair[0], mu.fov,
//                                              3 * (mu.input.size()));
//
//            // check both possible configurations, return the most likely
//            matches = mu.check_assumptions(candidates, candidate_pair,
//                                           {mu.input[a], mu.input[b]});
//
//            // definition of image match: |match| > match minimum
//            if (matches.size() > mu.parameters.match_minimum) {
//                matched = true;
//                break;
//            }
//        }
//
//        // break early if matched is met
//        if (matched) { break; }
//    }
//
//    return matches;
//}
