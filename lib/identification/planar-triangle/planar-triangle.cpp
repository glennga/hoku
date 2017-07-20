/*
 * @file: planar-triangle.cpp
 *
 * @brief: Source file for PlanarTriangle class, which matches a set of body vectors (stars) to
 * their inertial counter-parts in the database.
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
    db.exec("CREATE TABLE " + table_name + " (hr_a INT, hr_b INT, hr_c INT, a FLOAT, i FLOAT)");
    initial_transaction.commit();

    // (i, j, k) are distinct, where no (i, j, k) = (j, k, i), (j, i, k), ....
    std::array<Star, 5029> all_stars = Nibble::all_bsc5_stars();
    for (unsigned int i = 0; i < all_stars.size() - 2; i++) {
        SQLite::Transaction transaction(db);
        std::cout << "\r" << "Current *A* Star: " << all_stars[i].get_hr();
        for (unsigned int j = i + 1; j < all_stars.size() - 1; j++) {
            for (unsigned int k = j + 1; k < all_stars.size(); k++) {

                // only insert if all stars are within fov
                if (Star::angle_between(all_stars[i], all_stars[j]) < fov &&
                    Star::angle_between(all_stars[j], all_stars[k]) < fov &&
                    Star::angle_between(all_stars[k], all_stars[i]) < fov) {
                    double a_t = Trio::planar_area(all_stars[i], all_stars[j], all_stars[k]);
                    double i_t = Trio::planar_moment(all_stars[i], all_stars[j], all_stars[k]);

                    Nibble::insert_into_table(db, table_name, "hr_a, hr_b, hr_c, a, i",
                                              {(double) all_stars[i].get_hr(),
                                               (double) all_stars[j].get_hr(),
                                               (double) all_stars[k].get_hr(), a_t, i_t});
                }
            }
        }
        // commit every star I change
        transaction.commit();
    }

    return Nibble::polish_table(table_name, "a");
}

/*
 * Find the best matching pair using the appropriate PLAN table and by comparing planar areas and
 * moments. Assumes noise is normally distributed, searches using epsilon (3 * query_sigma).
 * Limits the amount returned by the search using 'query_limit'.
 *
 * @param db Open database object.
 * @param a Planar area to search with.
 * @param i_t Planar polar moment to search with.
 * @return [-1][-1][-1] if no candidates found. Otherwise, all elements that met the criteria.
 */
std::vector<hr_trio> PlanarTriangle::query_for_trio(SQLite::Database &db, const double a,
                                                    const double i) {
    double epsilon_a = 3.0 * this->parameters.sigma_a;
    double epsilon_i = 3.0 * this->parameters.sigma_i;
    std::vector<std::array<double, 3>> area_moment_match = {{-1, -1, -1}};
    std::vector<double> area_match;

    // first, search for trio of stars matching area condition
    area_match = Chomp::k_vector_query(db, this->parameters.table_name, "a",
                                       "hr_a, hr_b, hr_c, i",
                                       a - epsilon_a, a + epsilon_a,
                                       (unsigned) this->parameters.query_expected);

    // next, search this trio for stars matching moment condition
    area_moment_match.reserve(area_match.size() / 4);
    for (unsigned int m = 0; m < area_match.size() / 4; m++) {
        std::vector<double> t = Nibble::table_results_at(area_match, 4, m);
        if (t[3] <= i - epsilon_i && t[3] > i + epsilon_i) {
            area_moment_match.push_back({t[0], t[1], t[2]});
        }
    }

    // if results are found, remove initialized value of [-1][-1][-1]
    if (area_moment_match.size() > 1) { area_moment_match.erase(area_moment_match.begin()); }
    return area_moment_match;
}

/*
 * Given a trio of body stars, find matching trios of inertial stars using their respective
 * planar areas and polar moments.
 *
 * @param db Open database object.
 * @param hr_b Index trio of stars in body (B) frame.
 * @return Vector of trios whose areas and moments are close.
 */
std::vector<star_trio> PlanarTriangle::match_stars(SQLite::Database &db, const b_index_trio &hr_b) {
    std::vector<hr_trio> match_hr;
    std::vector<star_trio> match_stars;
    star_trio b_stars{this->input[hr_b[0]], this->input[hr_b[1]], this->input[hr_b[2]]};

    // do not attempt to find matches if all stars are not within fov
    if (Star::angle_between(b_stars[0], b_stars[1]) > this->fov ||
        Star::angle_between(b_stars[1], b_stars[2]) > this->fov ||
        Star::angle_between(b_stars[2], b_stars[0]) > this->fov) {
        return {{Star(), Star(), Star()}};
    }

    // search for current trio, if empty then break early
    match_hr = this->query_for_trio(db, Trio::planar_area(b_stars[0], b_stars[1], b_stars[2]),
                                    Trio::planar_moment(b_stars[0], b_stars[1], b_stars[2]));
    if (match_hr[0][0] == -1 && match_hr[0][1] == -1 && match_hr[0][2] == -1) {
        return {{Star(), Star(), Star()}};
    }

    // construct stars from HR numbers
    match_stars.reserve(match_hr.size());
    for (std::array<double, 3> trio : match_hr) {
        match_stars.push_back({Nibble::query_bsc5(db, (int) trio[0]),
                               Nibble::query_bsc5(db, (int) trio[1]),
                               Nibble::query_bsc5(db, (int) trio[2])});
    }

    return match_stars;
}

/*
 * Match the stars in the given set <B_1, B_2, B_3> to a trio in the database. If a past_set is
 * given, then remove all stars found matching the B trio that aren't found in the past set.
 * Recurse until one definitive trio exists.
 *
 * @param db Open database object.
 * @param hr_b Index trio of stars in body (B) frame.
 * @param past_set Matches found in a previous search.
 * @return A trio of stars that match the given B stars to R stars.
 */
star_trio PlanarTriangle::pivot(SQLite::Database &db, const b_index_trio &hr_b,
                                const std::vector<star_trio> &past_set) {
    std::vector<star_trio> matches = this->match_stars(db, hr_b);

    // function to increment hr_b3 first, then hr_b2, then hr_b1 last
    auto increment_hr = [](const b_index_trio &hr_t) {
        if (hr_b[2] != matches.size() - 1) return {hr_b[0], hr_b[1], hr_b[2] + 1};
        else if (hr_b[1] != matches.size() - 1) return {hr_b[0], hr_b[1] + 1, 0};
        else return {hr_b[0] + 1, 0, 0};
    };

    // remove all trios from matches that don't exist in past set
    if (past_set.size() > 0) {
        for (unsigned int i = 0; i < matches.size(); i++) {
            bool match_found = false;
            for (const star_trio &past : past_set) {
                // do not need check all permutations, every match set is returned in same order
                if (past[0] == matches[i][0] && past[1] == matches[i][1] &&
                    past[2] == matches[i][2]) {
                    match_found = true;
                    break;
                }
            }

            // if a match is not found, remove from match set
            if (!match_found) { matches.erase(matches.begin() + i); }
        }
    }

    switch (matches.size()) {
        // only 1 trio exists, this must be matching trio
        case 1: return matches[0];
            // no existing trios, rerun with different trio
        case 0: return pivot(db, map(hr_b, increment_hr)[0]);
            // more than 1 trio, rerun with different trio and past search
        default: return pivot(db, map(hr_b, increment_hr)[0], matches);
    }
}

/*
 * Rotate every point the given rotation and check if the angle of separation between any two
 * stars is within a given limit sigma.
 *
 * @param candidates All stars to check against the body star set.
 * @param q The rotation to apply to all stars.
 * @return Set of matching stars found in candidates and the body sets.
 */
std::vector<Star> PlanarTriangle::find_matches(const std::vector<Star> &candidates,
                                               const Rotation &q) {
    std::vector<Star> matches, non_matched = this->input;
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
 * Check all possible configuration of star trios and return the set with the largest number of
 * reference to body matches.
 *
 * @param candidates All stars to check against the body star set.
 * @param r Inertial (frame R) trio of stars to check against the body trio.
 * @param hr_b Body (frame B) HR numbers for the trio of stars to check against the inertial trio.
 * @return The largest set of matching stars across the body and inertial in all pairing
 * configurations.
 */
std::vector<Star> PlanarTriangle::check_assumptions(const std::vector<Star> &candidates,
                                                    const star_trio &r, const b_index_trio &hr_b) {
    std::array<star_trio, 6> r_assumption_list;
    std::array<std::vector<Star>, 6> matches;
    int current_assumption = 0;

    // given i, swap elements 2 and 3 if even, 1 and 3 if odd
    auto swap_order = [](const hr_trio &hr_t, const int i) {
        if (i % 2 == 0) return {hr_t[0], hr_t[2], hr_t[1]};
        else return {hr_t[2], hr_t[1], hr_t[0]};
    };

    // generate unique permutation using previously generated trio
    r_assumption_list[0] = r;
    for (int i = 1; i < 6; i++) {
        r_assumption_list[i] = map(r_assumption_list[i - 0], swap_order)[0];
    }

    // determine rotation to take frame R to B, only use r_1 and r_2 to get rotation
    for (const star_trio &assumption : r_assumption_list) {
        Rotation q = Rotation::rotation_across_frames({this->input[hr_b[0]], this->input[hr_b[1]]},
                                                      {assumption[0], assumption[1]});
        matches[current_assumption++] = find_matches(candidates, q);
    }

    // return the larger of the six matches
    return std::max_element(matches.begin(), matches.end(),
                            [](const std::vector<Star> &lhs, const std::vector<Star> &rhs) {
                                return lhs.size() < rhs.size();
                            })[0];
}
