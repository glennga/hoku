/// @file base-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for BaseTriangle class, which holds all common methods between the Planar and Spherical triangle
/// methods (they are very similar).

#include <iostream>
#include <cmath>
#include <algorithm>

#include "identification/base-triangle.h"

const int  BaseTriangle::NO_CANDIDATE_STARS_FOUND_EITHER = -1;
const int BaseTriangle::NO_CANDIDATE_STAR_SET_FOUND_EITHER = -2;
const BaseTriangle::index_trio BaseTriangle::STARTING_INDEX_TRIO = {0, 1, 2};

int BaseTriangle::generate_triangle_table (const std::shared_ptr<Chomp> &ch, const double fov,
                                           const std::string &table_name, area_function compute_area,
                                           moment_function compute_moment) {
    SQLite::Transaction initial_transaction(*ch->conn);

    // Exit early if the table already exists.
    if (ch->create_table(table_name,
                         "label_a INT, "
                         "label_b INT, "
                         "label_c INT, "
                         "a FLOAT, "
                         "i FLOAT")
        == Nibble::TABLE_NOT_CREATED_RET)
        return TABLE_ALREADY_EXISTS;

    initial_transaction.commit();
    ch->select_table(table_name);

    // (i, j, k) are distinct, where no (i, j, k) = (j, k, i), (j, i, k), ....
    Star::list all_stars = ch->bright_as_list();
    for (unsigned int i = 0; i < all_stars.size() - 2; i++) {
        SQLite::Transaction transaction(*ch->conn);
        for (unsigned int j = i + 1; j < all_stars.size() - 1; j++) {
            for (unsigned int k = j + 1; k < all_stars.size(); k++) {

                // Only insert if the angle between all stars are separated by fov degrees or less.
                if (Star::within_angle({all_stars[i], all_stars[j], all_stars[k]}, fov)) {
                    double a_t = compute_area(all_stars[i], all_stars[j], all_stars[k]);
                    double i_t = compute_moment(all_stars[i], all_stars[j], all_stars[k]);

                    // Prevent insertion of trios with error areas / moments.
                    if (a_t > 0 && !std::isnan(i_t) && i_t > 0) {
                        ch->insert_into_table(
                                "label_a, label_b, label_c, a, i",
                                Nibble::tuple_d{
                                        static_cast<double>(all_stars[i].get_label()),
                                        static_cast<double>(all_stars[j].get_label()),
                                        static_cast<double>(all_stars[k].get_label()),
                                        a_t,
                                        i_t
                                }
                        );
                    }
                }
            }
        }
        // Commit every star I change.
        transaction.commit();
    }

    return ch->sort_and_index(table_name);
}

std::vector<BaseTriangle::labels_list> BaseTriangle::query_for_trio (const double a, const double i) {
    std::vector<labels_list> big_r_ell;
    Nibble::tuples_d matches;

    // Query for candidates using all fields.
    matches = ch->simple_bound_query(
            {"a", "i"},
            "label_a, label_b, label_c",
            {a - epsilon_1, i - epsilon_2},
            {a + epsilon_1, i + epsilon_2},
            500
    );

    // Next, transform this set into candidate set labels.
    big_r_ell.reserve(matches.size());
    std::for_each(matches.begin(), matches.end(), [&big_r_ell] (const Chomp::tuple_d &t) -> void {
        big_r_ell.emplace_back(labels_list{
                static_cast<int> (t[0]),
                static_cast<int> (t[1]),
                static_cast<int>(t[2])
        });
    });

    return big_r_ell;
}

BaseTriangle::TrioVectorEither BaseTriangle::base_query_for_trios (const index_trio &c, area_function compute_area,
                                                                   moment_function compute_moment) {
    Star::trio b = {
            be->get_image()->at(c[0]),
            be->get_image()->at(c[1]),
            be->get_image()->at(c[2])
    };
    std::vector<labels_list> big_r_ell;
    std::vector<Star::trio> big_r;

    // Do not attempt to find matches if all stars are not within fov.
    if (!Star::within_angle({b[0], b[1], b[2]}, be->get_fov())) {
        return TrioVectorEither{{}, NO_CANDIDATE_STARS_FOUND_EITHER};
    }

    // Search for the current trio.
    big_r_ell = this->query_for_trio(compute_area(b[0], b[1], b[2]), compute_moment(b[0], b[1], b[2]));
    nu++;

    // If this is empty, then break early.
    if (big_r_ell.empty()) return TrioVectorEither{{}, NO_CANDIDATE_STARS_FOUND_EITHER};

    big_r.reserve(big_r_ell.size()); // Grab stars themselves from catalog IDs found in matches. Return these matches.
    std::for_each(
            big_r_ell.begin(), big_r_ell.end(),
            [&big_r, this] (const labels_list &r_ell) -> void {
                big_r.push_back({ch->query_hip(static_cast<int> (r_ell[0])),
                                 ch->query_hip(static_cast<int> (r_ell[1])),
                                 ch->query_hip(static_cast<int> (r_ell[2]))});
            }
    );

    return TrioVectorEither{big_r, 0};
}

/// Reset out r_1 match set. Generate a series of indices to iterate through as we perform the pivot operation. Store
/// the results in the p stack.
void BaseTriangle::initialize_pivot (const index_trio &c) {
    this->big_r_1 = nullptr;
    pivot_c.clear();

    for (unsigned int j = 0; j < be->get_image()->size(); j++) {
        if (std::find(c.begin(), c.end(), j) == c.end()) pivot_c.push_back(j);
    }
}

/// Match the stars in the given set {b_1, b_2, b_3} to a trio in the database. If a past_set is given, then remove
/// all stars found matching the b trio that aren't found in the past set. Recurse until one definitive trio exists.
BaseTriangle::TriosEither BaseTriangle::pivot (const index_trio &c) {
    // Practical limit: exit early if we have iterated through too many comparisons without match.
    if (nu > nu_max) return TriosEither{{}, EXCEEDED_NU_MAX_EITHER};

    // This is our first run. Initialize our r_1 match set.
    TrioVectorEither big_r = this->query_for_trios(c);
    if (this->big_r_1 == nullptr) big_r_1 = std::make_unique<std::vector<Star::trio>>(big_r.result);

    // Remove all trios from matches that have at least two stars in the past set (below is PartialMatch).
    if (big_r.error != NO_CANDIDATE_STARS_FOUND_EITHER) {
        big_r_1->erase(std::remove_if(
                big_r_1->begin(), big_r_1->end(),
                [&big_r] (const Star::trio &r_1) {
                    for (const Star::trio &r : big_r.result) {
                        if (static_cast<int>(r[0] == r_1[0] || r[0] == r_1[1] || r[0] == r_1[2])
                            + static_cast<int>(r[1] == r_1[0] || r[1] == r_1[1] || r[1] == r_1[2])
                            + static_cast<int>(r[2] == r_1[0] || r[2] == r_1[1] || r[2] == r_1[2]) >= 2)
                            return false;
                    }

                    return true;
                }), big_r_1->end());
    }

    switch (big_r_1->size()) {
        case 1: // Only 1 trio exists. This must be the matching trio.
            return TriosEither{(*big_r_1)[0], 0};
        case 0: // No trios exist. Exit early.
            return TriosEither{{}, NO_CANDIDATE_STAR_SET_FOUND_EITHER};
        default: // 2+ trios exists. Run with different 3rd element and history, or exit with error set.
            return (static_cast<unsigned>(c[2]) != be->get_image()->size() - 1) ? pivot(index_trio{
                    c[0],
                    c[1],
                    ptop(this->pivot_c)
            }) : TriosEither{{}, NO_CANDIDATE_STAR_SET_FOUND_EITHER};
    }
}

BaseTriangle::StarsEither BaseTriangle::direct_match_test (const Star::list &big_p, const Star::trio &r,
                                                           const Star::trio &b) {
    std::array<Star::list, 6> big_m = {}, big_a = {};

    // Generate unique permutations using previously generated trio.
    std::array<index_trio, 6> big_a_c = {
            STARTING_INDEX_TRIO,
            index_trio{0, 2, 1},
            index_trio{1, 0, 2},
            index_trio{1, 2, 0},
            index_trio{2, 0, 1},
            index_trio{2, 1, 0}
    };

    // Determine the rotation to take frame R to B.
    for (unsigned int i = 0; i < 6; i++) {
        big_m[i] = Identification::find_positive_overlay(*be->get_image(), big_p, Rotation::triad(
                {b[0], b[1], b[2]},
                {r[big_a_c[i][0]], r[big_a_c[i][1]], r[big_a_c[i][2]]}
        ), this->epsilon_4);
        big_a[i] = {
                Star::define_label(b[0], r[big_a_c[i][0]].get_label()),
                Star::define_label(b[1], r[big_a_c[i][1]].get_label()),
                Star::define_label(b[2], r[big_a_c[i][2]].get_label())
        };
    }

    // Return map set corresponding to the largest match (messy lambda and iterator stuff below D:).
    if (big_a[0].size() == big_a[1].size() ==
        big_a[2].size() == big_a[3].size() ==
        big_a[4].size() == big_a[5].size())
        return StarsEither{{}, NO_CONFIDENT_A_EITHER};

    else
        return StarsEither{
                big_a[std::max_element(
                        big_m.begin(), big_m.end(),
                        [] (const Star::list &lhs, const Star::list &rhs) {
                            return lhs.size() < rhs.size();
                        }) - big_m.begin()
                ], 0};
}

/// Find the matching pairs using the appropriate triangle table and by comparing areas and polar moments. This is
/// just a wrapper for query_for_trio.
std::vector<BaseTriangle::labels_list> BaseTriangle::e_query (double a, double i) { return query_for_trio(a, i); }

BaseTriangle::StarsEither BaseTriangle::e_reduction () {
    pivot_c = {};
    nu = 0;

    for (int i = 0; i < static_cast<signed> (be->get_image()->size() - 2); i++) {
        for (int j = i + 1; j < static_cast<signed> (be->get_image()->size() - 1); j++) {
            for (int k = j + 1; k < static_cast<signed> (be->get_image()->size()); k++) {
                initialize_pivot({i, j, k});
                TriosEither p = pivot({i, j, k});

                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (nu > nu_max) return StarsEither{{}, EXCEEDED_NU_MAX_EITHER};


                // Require that the pivot produces a meaningful result.
                if (p.error == NO_CANDIDATE_STAR_SET_FOUND_EITHER) continue;
                else return StarsEither{Star::list{p.result[0], p.result[1], p.result[2]}, 0};
            }
        }
    }

    return StarsEither{{}, NO_CONFIDENT_R_EITHER};
}

BaseTriangle::StarsEither BaseTriangle::e_identify () {
    pivot_c = {};
    nu = 0;

    // There exists |big_i| choose 3 possibilities.
    for (int i = 0; i < static_cast<signed> (be->get_image()->size() - 2); i++) {
        for (int j = i + 1; j < static_cast<signed> (be->get_image()->size() - 1); j++) {
            for (int k = j + 1; k < static_cast<signed> (be->get_image()->size()); k++) {
                initialize_pivot({i, j, k}); // Find matches of current body trio to catalog. Pivot if necessary.
                TriosEither r = pivot({i, j, k});

                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (nu > nu_max) return StarsEither{{}, EXCEEDED_NU_MAX_EITHER};

                // Require that the pivot produces a meaningful result.
                if (r.error == NO_CANDIDATE_STAR_SET_FOUND_EITHER) continue;

                // Find candidate stars around the candidate trio.
                Star::list big_p = ch->nearby_hip_stars(r.result[0], be->get_fov(), 500);
                nu++;

                // Find the most likely map given the two pairs.
                StarsEither a = direct_match_test(big_p, r.result, {
                        be->get_image()->at(i),
                        be->get_image()->at(j),
                        be->get_image()->at(k)
                });
                if (a.error != NO_CONFIDENT_A_EITHER) return a;
            }
        }
    }
    return StarsEither{{}, NO_CONFIDENT_A_EITHER};
}
