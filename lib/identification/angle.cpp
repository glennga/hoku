/// @file angle.cpp
/// @author Glenn Galvizo
///
/// Source file for Angle class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#include <iostream>

#include "identification/angle.h"

const unsigned int Angle::QUERY_STAR_SET_SIZE = 2;
const int Angle::NO_CANDIDATES_FOUND_EITHER = -1;
const int Angle::NO_CANDIDATE_PAIR_FOUND_EITHER = -2;

int Angle::generate_table (const std::shared_ptr<Chomp> &ch, const double fov, const std::string &table_name) {
    SQLite::Transaction transaction(*ch->conn);

    // Exit early if the table already exists.
    if (ch->create_table(
            table_name,
            "label_a INT, "
            "label_b INT, "
            "theta FLOAT"
    ) == Nibble::TABLE_NOT_CREATED_RET)
        return TABLE_ALREADY_EXISTS;

    ch->select_table(table_name);

    // (i, j) are distinct, where no (i, j) = (j, i).
    Star::list all_stars = ch->bright_as_list();
    for (unsigned int i = 0; i < all_stars.size() - 1; i++) {
        for (unsigned int j = i + 1; j < all_stars.size(); j++) {
            double theta = (180.0 / M_PI) * Vector3::Angle(all_stars[i], all_stars[j]);

            // Only insert if the angle between both stars is less than fov.
            if (theta < fov)
                ch->insert_into_table(
                        "label_a, label_b, theta",
                        Nibble::tuple_d{static_cast<double> (all_stars[i].get_label()),
                                        static_cast<double> (all_stars[j].get_label()),
                                        theta
                        });
        }
    }

    transaction.commit();
    return ch->sort_and_index("theta");
}

Identification::LabelsEither Angle::query_for_pair (const double theta) {
    std::vector<labels_list> big_r_ell;
    Nibble::tuples_d big_r_ell_tuples;

    // Query using theta with epsilon bounds. Return NO_CONFIDENT_R if nothing is found.
    big_r_ell_tuples = ch->simple_bound_query(
            {"theta"},
            "label_a, label_b",
            {theta - epsilon_1},
            {theta + epsilon_2},
            500
    );
    nu++;

    // Create the candidate label list.
    for (const Nibble::tuple_d &candidate : big_r_ell_tuples) {
        big_r_ell.emplace_back(labels_list{static_cast<int>(candidate[0]), static_cast<int>(candidate[1])});
    }

    return LabelsEither{big_r_ell[0], 0};
}

Angle::PairsEither Angle::find_candidate_pair (const Star &b_i, const Star &b_j) {
    double theta = (180.0 / M_PI) * Vector3::Angle(b_i.get_vector(), b_j.get_vector());

    // If the current angle is greater than the current fov, break early.
    if (theta > be->get_fov()) return PairsEither{{}, NO_CANDIDATE_PAIR_FOUND_EITHER};

    // If no candidate is found, break early.
    LabelsEither big_r_ell = this->query_for_pair(theta);
    if (big_r_ell.error == NO_CANDIDATES_FOUND_EITHER) return PairsEither{{}, NO_CANDIDATE_PAIR_FOUND_EITHER};

    // Otherwise, obtain and return the inertial vectors for the given candidates.
    return PairsEither{Star::pair{
            ch->query_hip(big_r_ell.result[0]),
            ch->query_hip(big_r_ell.result[1])
    }, 0};
}

/// Find the best fitting match of input stars (body) to database stars (catalog) using the given pair as reference.
////
/// Assumption One: b_1 = r_1, b_2 = r_2
/// Assumption Two: b_1 = r_2, b_2 = r_1
Identification::StarsEither Angle::direct_match_test (const Star::list &big_p, const Star::list &r,
                                                      const Star::list &b) {
    if (r.size() != 2 || b.size() != 2) {
        throw std::runtime_error(std::string("Input lists does not have exactly two b."));
    }
    std::array<Star::list, 2> big_m = {}, big_a = {};

    // Determine the rotation to take frame B to A, find all matches with this rotation.
    for (unsigned int i = 0; i < 2; i++) {
        std::array<int, 2> a = {(i == 0) ? 0 : 1, (i == 0) ? 1 : 0}; // We define our identity 'a' below.

        big_m[i] = Identification::find_positive_overlay(*be->get_image(), big_p, Rotation::triad(
                {b[0], b[1]},
                {r[a[0]], r[a[1]]}
        ), this->epsilon_4);
        big_a[i] = {Star::define_label(b[0], r[a[0]].get_label()), Star::define_label(b[1], r[a[1]].get_label())};
    }

    // Return the body pair with the appropriate labels.
    if (big_m[0].size() != big_m[1].size()) {
        return StarsEither{(big_m[0].size() > big_m[1].size()) ? big_a[0] : big_a[1], 0};
    }
    else return StarsEither{{}, NO_CONFIDENT_A_EITHER};
}

std::vector<Identification::labels_list> Angle::query () {
    double theta = (180.0 / M_PI) * Vector3::Angle(be->get_image()->at(0), be->get_image()->at(1));
    std::vector<labels_list> big_r_ell;

    // Query using theta with epsilon bounds.
    ch->select_table(table_name);
    Nibble::tuples_d big_r_tuples = ch->simple_bound_query(
            {"theta"},
            "label_a, label_b",
            {theta - epsilon_1},
            {theta + epsilon_1},
            500
    );

    big_r_ell.reserve(big_r_tuples.size() / 2); // Sort r into list of catalog ID pairs.
    for (const Nibble::tuple_d &r_t : big_r_tuples) {
        big_r_ell.emplace_back(labels_list{static_cast<int>(r_t[0]), static_cast<int>(r_t[1])});
    }

    return big_r_ell;
}

Identification::StarsEither Angle::reduce () {
    ch->select_table(table_name), nu = 0;

    for (unsigned int i = 0; i < be->get_image()->size() - 1; i++) {
        for (unsigned int j = i + 1; j < be->get_image()->size(); j++) {
            PairsEither r = find_candidate_pair(be->get_image()->at(i), be->get_image()->at(j));

            // Practical limit: exit early if we have iterated through too many comparisons without match.
            if (nu > nu_max) return StarsEither{{}, NO_CONFIDENT_R_EITHER};

            // The reduction step: |R| = 1.
            if (r.error == NO_CANDIDATE_PAIR_FOUND_EITHER) continue;
            else return StarsEither{Star::list{r.result[0], r.result[1]}, 0};
        }
    }
    return StarsEither{{}, NO_CONFIDENT_R_EITHER};
}

/// @return NO_CONFIDENT_A if an identification cannot be found exhaustively. EXCEEDED_NU_MAX if an
/// identification cannot be found within a certain number of query picks. Otherwise, body stars b with the attached
/// labels of the inertial pair r.
Identification::StarsEither Angle::identify () {
    nu = 0;

    // There exists |big_i| choose 2 possibilities.
    for (unsigned int i = 0; i < be->get_image()->size() - 1; i++) {
        for (unsigned int j = i + 1; j < be->get_image()->size(); j++) {
            // Practical limit: exit early if we have iterated through too many comparisons without match.
            if (nu > nu_max) return StarsEither{{}, NO_CONFIDENT_R_EITHER};

            // Narrow down current pair to two stars in catalog. The order is currently unknown.
            PairsEither r = find_candidate_pair(be->get_image()->at(i), be->get_image()->at(j));
            if (r.error == NO_CANDIDATE_PAIR_FOUND_EITHER) continue;

            // Find candidate stars around the candidate pair.
            Star::list big_p = ch->nearby_hip_stars(r.result[0], be->get_fov(), 500);
            nu++;

            // Find the most likely pair combination given the two pairs.
            StarsEither a = direct_match_test(
                    big_p,
                    {r.result[0], r.result[1]},
                    {be->get_image()->at(i), be->get_image()->at(j)}
            );
            if (a.error != NO_CONFIDENT_A_EITHER) return a;
        }
    }

    return StarsEither{{}, NO_CONFIDENT_A_EITHER};
}
