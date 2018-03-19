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
const std::vector<BaseTriangle::labels_list> BaseTriangle::NO_CANDIDATE_TRIOS_FOUND = {{-1, -1, -1}};

/// Returned when no candidates can be found from a match step.
const std::vector<Star::trio> BaseTriangle::NO_CANDIDATE_STARS_FOUND = {
    {Star::wrap(Vector3::Zero()), Star::wrap(Vector3::Zero()), Star::wrap(Vector3::Zero())}};

/// Returned with an unsuccessful pivoting step.
const Star::trio BaseTriangle::NO_CANDIDATE_STAR_SET_FOUND = {Star::wrap(Vector3::Zero()), Star::wrap(Vector3::Zero()),
    Star::wrap(Vector3::Zero())};

/// Starting index trio to perform pivot with.
const BaseTriangle::index_trio BaseTriangle::STARTING_INDEX_TRIO = {0, 1, 2};

/// Constructor. Initializes our permutation stack.
BaseTriangle::BaseTriangle () : Identification() {
    pivot_c = {};
}

/// Generate the triangle table given the specified FOV and table name. This find the area and polar moment
/// between each distinct permutation of trios, and only stores them if they fall within the corresponding
/// field-of-view.
///
/// @param cf Configuration reader holding all parameters to use.
/// @param triangle_type Field lookup for configuration file. Must be "planar-triangle" or "spherical-triangle".
/// @param compute_area Area function to compute area with.
/// @param compute_moment Polar moment function to compute moment with.
/// @return TABLE_ALREADY_EXISTS if the table already exists. Otherwise, 0 when finished.
int BaseTriangle::generate_triangle_table (INIReader &cf, const std::string &triangle_type, area_function compute_area,
                                           moment_function compute_moment) {
    Chomp ch;
    SQLite::Transaction initial_transaction(*ch.conn);
    double fov = cf.GetReal("hardware", "fov", 0);
    
    // Exit early if the table already exists.
    std::string table_name = cf.Get("table-names", triangle_type, "");
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
                    if (a_t > 0 && !std::isnan(i_t) && i_t > 0) {
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
    
    return ch.polish_table(cf.Get("table-focus", triangle_type, ""));
}

/// Find the matching pairs using the appropriate triangle table and by comparing areas and polar moments. Assumes
/// noise is normally distributed, searches using epsilon (3 * sigma_a) and a basic bounded query.
///
/// @param a Area (planar or spherical) to search with.
/// @param i_t Polar moment (planar or spherical) to search with.
/// @return NO_CANDIDATE_TRIOS_FOUND if no candidates found. Otherwise, all elements that met the criteria.
std::vector<BaseTriangle::labels_list> BaseTriangle::query_for_trio (const double a, const double i) {
    double epsilon_1 = 3.0 * this->parameters.sigma_1, epsilon_2 = 3.0 * this->parameters.sigma_2;
    std::vector<labels_list> big_r_ell = NO_CANDIDATE_TRIOS_FOUND;
    Nibble::tuples_d matches;
    
    // Query for candidates using all fields.
    matches = ch.simple_bound_query({"a", "i"}, "label_a, label_b, label_c", {a - epsilon_1, i - epsilon_2},
                                    {a + epsilon_1, i + epsilon_2}, this->parameters.sql_limit);
    
    // Next, transform this set into candidate set labels.
    big_r_ell.reserve(matches.size());
    std::for_each(matches.begin(), matches.end(), [&big_r_ell] (const Chomp::tuple_d &t) -> void {
        big_r_ell.emplace_back(labels_list {static_cast<int> (t[0]), static_cast<int> (t[1]), static_cast<int>(t[2])});
    });
    
    // If results are found, remove the initialized value of NO_CANDIDATE_TRIOS_FOUND.
    if (big_r_ell.size() > 1) {
        big_r_ell.erase(big_r_ell.begin());
    }
    
    // Favor bright stars if specified. Applied with the FAVOR_BRIGHT_STARS flag.
    if (this->parameters.favor_bright_stars) {
        sort_brightness(big_r_ell);
    }
    return big_r_ell;
}

/// Given a trio of body stars, find matching trios of inertial stars using their respective planar areas and polar
/// moments.
///
/// @param c Index trio of stars ('combination', but really permutation of I) in body frame.
/// @param compute_area Area function to compute area with.
/// @param compute_moment Polar moment function to compute moment with.
/// @return NO_CANDIDATE_STARS_FOUND if stars are not within the fov or if no matches currently exist. Otherwise,
/// vector of trios whose areas and moments are close.
std::vector<Star::trio> BaseTriangle::base_query_for_trios (const index_trio &c, area_function compute_area,
                                                            moment_function compute_moment) {
    Star::trio b = {this->big_i[c[0]], this->big_i[c[1]], this->big_i[c[2]]};
    std::vector<labels_list> big_r_ell;
    std::vector<Star::trio> big_r;
    
    // Do not attempt to find matches if all stars are not within fov.
    if (!Star::within_angle({b[0], b[1], b[2]}, this->fov)) {
        return NO_CANDIDATE_STARS_FOUND;
    }
    
    // Search for the current trio. If this is empty, then break early.
    big_r_ell = this->query_for_trio(compute_area(b[0], b[1], b[2]), compute_moment(b[0], b[1], b[2]));
    if (std::equal(big_r_ell.begin(), big_r_ell.end(), NO_CANDIDATE_TRIOS_FOUND.begin())) {
        return NO_CANDIDATE_STARS_FOUND;
    }
    
    // Grab stars themselves from catalog IDs found in matches. Return these matches.
    big_r.reserve(big_r_ell.size());
    std::for_each(big_r_ell.begin(), big_r_ell.end(), [&big_r, this] (const labels_list &r_ell) -> void {
        big_r.push_back({ch.query_hip(static_cast<int> (r_ell[0])), ch.query_hip(static_cast<int> (r_ell[1])),
                            ch.query_hip(static_cast<int> (r_ell[2]))});
    });
    
    return big_r;
}

/// Reset out r_1 match set. Generate a series of indices to iterate through as we perform the pivot operation. Store
/// the results in the p stack.
///
/// @param c Index trio of stars in the body (B) frame that are 'removed' from this series.
void BaseTriangle::initialize_pivot (const index_trio &c) {
    this->big_r_1 = nullptr;
    pivot_c.clear();
    
    for (unsigned int j = 0; j < big_i.size(); j++) {
        if (std::find(c.begin(), c.end(), j) == c.end()) {
            pivot_c.push_back(j);
        }
    }
}

/// Match the stars in the given set {b_1, b_2, b_3} to a trio in the database. If a past_set is given, then remove
/// all stars found matching the b trio that aren't found in the past set. Recurse until one definitive trio exists.
///
/// @param c Index trio of stars ('combination', but really permutation of I) in body frame.
/// @return NO_CANDIDATE_STAR_SET_FOUND if pivoting is unsuccessful. Otherwise, a trio of stars that match the given B
/// stars to R stars.
Star::trio BaseTriangle::pivot (const index_trio &c) {
    (*parameters.nu)++;
    
    // Practical limit: exit early if we have iterated through too many comparisons without match.
    if (*parameters.nu > parameters.nu_max) {
        return NO_CANDIDATE_STAR_SET_FOUND;
    }
    
    // This is our first run. Initialize our r_1 match set.
    std::vector<Star::trio> big_r = this->query_for_trios(c);
    if (this->big_r_1 == nullptr) {
        big_r_1 = std::make_unique<std::vector<Star::trio>>(big_r);
    }
    
    // Remove all trios from matches that have at least two stars in the past set (below is PartialMatch).
    if (!std::equal(big_r.begin(), big_r.end(), NO_CANDIDATE_STARS_FOUND.begin())) {
        (*big_r_1).erase(std::remove_if((*big_r_1).begin(), (*big_r_1).end(), [&big_r] (const Star::trio &r_1) {
            for (const Star::trio &r : big_r) {
                if (static_cast<int>(r[0] == r_1[0] || r[0] == r_1[1] || r[0] == r_1[2])
                    + static_cast<int>(r[1] == r_1[0] || r[1] == r_1[1] || r[1] == r_1[2])
                    + static_cast<int>(r[2] == r_1[0] || r[2] == r_1[1] || r[2] == r_1[2]) >= 2) {
                    return false;
                }
            }
            return true;
        }), (*big_r_1).end());
    }
    
    // Pivot restriction, w/o restriction we avoid recursion. Applied with the NO_REDUCTION flag.
    if (this->parameters.no_reduction) {
        return (*big_r_1)[0];
    }
    
    switch ((*big_r_1).size()) {
        case 1: return (*big_r_1)[0]; // Only 1 trio exists. This must be the matching trio.
        case 0: return NO_CANDIDATE_STAR_SET_FOUND; // No trios exist. Exit early.
        default: // 2+ trios exists. Run with different 3rd element and history, or exit with error set.
            return (static_cast<unsigned>(c[2]) != this->big_i.size() - 1) ? pivot(
                index_trio {c[0], c[1], ptop(this->pivot_c)}) : NO_CANDIDATE_STAR_SET_FOUND;
    }
}

/// Check all possible configuration of star trios and return quaternion corresponding to the set with the largest
/// number of reference to body matches.
///
/// @param big_p All stars to check against the body star set.
/// @param r Inertial (frame R) trio of stars to check against the body trio.
/// @param b Body (frame B) Trio of stars to check against the inertial trio.
/// @return The star list corresponding to largest set of matching stars across the body and inertial in all pairing
/// configurations.
Star::list BaseTriangle::direct_match_test (const Star::list &big_p, const Star::trio &r, const Star::trio &b) {
    std::array<Star::list, 6> big_m = {}, big_a = {};
    
    // Generate unique permutations using previously generated trio.
    std::array<index_trio, 6> big_a_c = {STARTING_INDEX_TRIO, index_trio {0, 2, 1}, index_trio {1, 0, 2}, index_trio
        {1, 2, 0}, index_trio {2, 0, 1}, index_trio {2, 1, 0}};
    auto attach_ell = [&r, &b, &big_a_c] (const int i, const int j) -> Star {
        return Star::define_label(b[j], r[big_a_c[i][j]].get_label());
    };
    
    // Determine the rotation to take frame R to B.
    for (unsigned int i = 0; i < 6; i++) {
        Rotation q = parameters.f({b[0], b[1], b[2]}, {r[big_a_c[i][0]], r[big_a_c[i][1]], r[big_a_c[i][2]]});
        big_m[i] = find_positive_overlay(big_p, q);
        big_a[i] = {attach_ell(i, 0), attach_ell(i, 1), attach_ell(i, 2)};
    }
    
    // Return map set corresponding to the largest match (messy lambda and iterator stuff below D:).
    return big_a[std::max_element(big_m.begin(), big_m.end(), [] (const Star::list &lhs, const Star::list &rhs) {
        return lhs.size() < rhs.size();
    }) - big_m.begin()];
}

/// Find the matching pairs using the appropriate triangle table and by comparing areas and polar moments. This is
/// just a wrapper for query_for_trio.
///
/// @param a Area (planar or spherical) to search with.
/// @param i_t Polar moment (planar or spherical) to search with.
/// @return All candidates that meet the criteria.
std::vector<BaseTriangle::labels_list> BaseTriangle::e_query (double a, double i) {
    std::vector<labels_list> big_r_ell = query_for_trio(a, i);
    return (big_r_ell[0] == NO_CANDIDATE_TRIOS_FOUND[0]) ? std::vector<labels_list> {labels_list {}} : big_r_ell;
}

/// Find the **best** matching pair to the first three stars in our benchmark using the appropriate triangle table.
/// Assumes noise is normally distributed, searches using epsilon (3 * sigma_a) and a basic bounded query.
/// We require the following be defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_query
///     - sql_limit
///     - nu
///     - nu_max
/// @endcode
///
/// @return NO_CONFIDENT_R if no candidates found. Otherwise, a single elements that best meets the criteria.
Star::list BaseTriangle::e_reduction () {
    *parameters.nu = 0;
    
    for (int i = 0; i < static_cast<signed> (big_i.size() - 2); i++) {
        for (int j = i + 1; j < static_cast<signed> (big_i.size() - 1); j++) {
            for (int k = j + 1; k < static_cast<signed> (big_i.size()); k++) {
                initialize_pivot({i, j, k});
                Star::trio p = pivot({i, j, k});
                
                // Require that the pivot produces a meaningful result.
                if (std::equal(p.begin(), p.end(), NO_CANDIDATE_STAR_SET_FOUND.begin())) {
                    continue;
                }
                return {p[0], p[1], p[2]};
            }
        }
    }
    return NO_CONFIDENT_R;
}

/// Find the rotation from the images in our current benchmark to our inertial frame (i.e. the catalog).
///
/// @return NO_CONFIDENT_A if an identification cannot be found exhaustively. EXCEEDED_NU_MAX if an
/// identification cannot be found within a certain number of query picks. Otherwise, body stars b with the attached
/// labels of the inertial pair r.
Star::list BaseTriangle::e_identify () {
    *parameters.nu = 0;
    
    // There exists |big_i| choose 3 possibilities.
    for (int i = 0; i < static_cast<signed> (big_i.size() - 2); i++) {
        for (int j = i + 1; j < static_cast<signed> (big_i.size() - 1); j++) {
            for (int k = j + 1; k < static_cast<signed> (big_i.size()); k++) {
                (*parameters.nu)++;
                
                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (*parameters.nu > parameters.nu_max) {
                    return EXCEEDED_NU_MAX;
                }
                
                // Find matches of current body trio to catalog. Pivot if necessary.
                initialize_pivot({i, j, k});
                Star::trio r = pivot({i, j, k});
                if (std::equal(r.begin(), r.end(), NO_CANDIDATE_STAR_SET_FOUND.begin())) {
                    continue;
                }
                
                // Find candidate stars around the candidate trio.
                Star::list big_p = ch.nearby_hip_stars(r[0], fov, static_cast<unsigned int> (3.0 * big_i.size()));
                
                // Find the most likely map given the two pairs.
                return direct_match_test(big_p, r, {big_i[i], big_i[j], big_i[k]});
            }
        }
    }
    return NO_CONFIDENT_A;
}
