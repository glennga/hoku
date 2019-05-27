/// @file dot-angle.cpp
/// @author Glenn Galvizo
///
/// Source file for DotAngle class, which matches a set of body vectors (stars) to their inertial counter-parts in
/// the database.

#include <iostream>

#include "math/trio.h"
#include "identification/dot-angle.h"

const unsigned int Dot::QUERY_STAR_SET_SIZE = 3;
const int Dot::NO_CANDIDATES_FOUND_EITHER = -1;
const int Dot::NO_CANDIDATE_TRIO_FOUND_EITHER = -2;

int Dot::generate_table (const std::shared_ptr<Chomp> &ch, double fov, const std::string &table_name) {
    // Exit early if the table already exists.
    if (ch->create_table(
            table_name,
            "label_a INT, "
            "label_b INT, "
            "label_c INT, "
            "theta_1 FLOAT, "
            "theta_2 FLOAT, "
            "phi FLOAT"
    ) == Nibble::TABLE_NOT_CREATED_RET)
        return TABLE_ALREADY_EXISTS;

    ch->select_table(table_name);

    // Iterate through all possible permutations of i, j, c...
    Star::list all_stars = ch->bright_as_list();
    for (unsigned int i = 0; i < all_stars.size(); i++) {
        SQLite::Transaction transaction(*ch->conn);

        for (unsigned int j = 0; j < all_stars.size(); j++) {
            for (unsigned int c = 0; c < all_stars.size(); c++) {
                if (i == j || i == c || j == c) continue;

                // Compute each feature (theta^1, theta^2, phi).
                double theta_1 = (180.0 / M_PI) * Vector3::Angle(all_stars[c], all_stars[i]);
                double theta_2 = (180.0 / M_PI) * Vector3::Angle(all_stars[c], all_stars[j]);
                double phi = Trio::dot_angle(all_stars[i], all_stars[j], all_stars[c]);

                // Condition 6d: theta^1 < theta^2.
                if (theta_1 < theta_2 && Star::within_angle({all_stars[i], all_stars[j], all_stars[c]}, fov)) {
                    ch->insert_into_table(
                            "label_a, label_b, label_c, theta_1, theta_2, phi",
                            Nibble::tuple_d{
                                    static_cast<double> (all_stars[i].get_label()),
                                    static_cast<double>(all_stars[j].get_label()),
                                    static_cast<double> (all_stars[c].get_label()),
                                    theta_1,
                                    theta_2,
                                    phi
                            });
                }
            }
        }
        transaction.commit();
    }
    return ch->sort_and_index("theta_1, theta_2, phi");
}

Dot::LabelsEither Dot::query_for_trio (double theta_1, double theta_2, double phi) {
    std::vector<labels_list> big_r_ell;
    Nibble::tuples_d matches;

    // Query for candidates using all fields.
    matches = ch->simple_bound_query(
            {"theta_1", "theta_2", "phi"},
            "label_a, label_b, label_c",
            {theta_1 - epsilon_1, theta_2 - epsilon_2, phi - epsilon_3},
            {theta_1 + epsilon_1, theta_2 + epsilon_2, phi + epsilon_3},
            500
    );
    nu++;

    // Transform each tuple into a candidate list of labels.
    big_r_ell.reserve(matches.size());
    std::for_each(matches.begin(), matches.end(), [&big_r_ell] (const Chomp::tuple_d &t) -> void {
        big_r_ell.emplace_back(labels_list{
                static_cast<int> (t[0]),
                static_cast<int> (t[1]),
                static_cast<int>(t[2])
        });
    });

    if (big_r_ell.empty()) return LabelsEither{{}, NO_CANDIDATES_FOUND_EITHER};
    else return LabelsEither{big_r_ell[0], 0};
}

Dot::TriosEither Dot::find_candidate_trio (const Star &b_i, const Star &b_j, const Star &b_c) {
    double theta_1 = (180.0 / M_PI) * Vector3::Angle(b_c.get_vector(), b_i.get_vector());
    double theta_2 = (180.0 / M_PI) * Vector3::Angle(b_c.get_vector(), b_j.get_vector());
    double phi = Trio::dot_angle(b_i, b_j, b_c);

    // Ensure that condition 6d holds, and that all stars are within fov. Exit early if this is not met.
    if (theta_1 > theta_2 || !Star::within_angle({b_i, b_j, b_c}, be->get_fov())) {
        return TriosEither{{}, NO_CANDIDATE_TRIO_FOUND_EITHER};
    }

    // If not candidate is found, break early.
    LabelsEither big_r_ell = this->query_for_trio(theta_1, theta_2, phi);
    if (big_r_ell.error == NO_CANDIDATES_FOUND_EITHER) return TriosEither{{}, NO_CANDIDATE_TRIO_FOUND_EITHER};

    // Otherwise, obtain and return the inertial vectors for the given candidates.
    return {Star::trio{
            ch->query_hip(big_r_ell.result[0]),
            ch->query_hip(big_r_ell.result[1]),
            ch->query_hip(big_r_ell.result[2])
    }, 0};
}

/// Find the 2 nearest neighbors to the given b_i star. This is a KNN approach, with K = 2 and
/// d = theta(b_i, other star). We return the b set we are going to move forward with.
Star::trio Dot::find_closest (const Star &b_i) {
    std::vector<std::array<double, 2>> theta_big_i;

    // Find all possible distances between the central star and all of stars in the image.
    theta_big_i.reserve(be->get_image()->size());
    for (unsigned int i = 0; i < be->get_image()->size(); i++) {
        if (be->get_image()->at(i) != b_i.get_vector()) {
            theta_big_i.emplace_back(std::array<double, 2>{
                    Star::Angle(b_i.get_vector(), be->get_image()->at(i)),
                    static_cast<double>(i)
            });
        }
    }

    // Sort based on the distance.
    std::sort(theta_big_i.begin(), theta_big_i.end(),
              [] (const std::array<double, 2> &s_1, const std::array<double, 2> &s_2) -> bool {
                  return s_1[0] < s_2[0];
              });

    // Return the two smallest stars (in terms of distance).
    return {
            be->get_image()->at(static_cast<int>(theta_big_i[0][1])),
            be->get_image()->at(static_cast<int>(theta_big_i[1][1])),
            b_i
    };
}

std::vector<Identification::labels_list> Dot::query () {
    std::vector<labels_list> big_r_ell;
    Nibble::tuples_d matches;

    double theta_1 = (180.0 / M_PI) * Vector3::Angle(be->get_image()->at(2), be->get_image()->at(0));
    double theta_2 = (180.0 / M_PI) * Vector3::Angle(be->get_image()->at(2), be->get_image()->at(1));
    double phi = Trio::dot_angle(be->get_image()->at(0), be->get_image()->at(1), be->get_image()->at(2));

    // Ensure that condition 6d holds: switch if not.
    if (theta_1 > theta_2) {
        double theta_t = theta_1;
        theta_1 = theta_2, theta_2 = theta_t;
    }

    // Query for our candidate set.
    matches = ch->simple_bound_query(
            {"theta_1", "theta_2", "phi"},
            "label_a, label_b, label_c",
            {theta_1 - epsilon_1, theta_2 - epsilon_2, phi - epsilon_3},
            {theta_1 + epsilon_1, theta_2 + epsilon_2, phi + epsilon_3},
            500
    );

    // Transform candidate set tuples into labels list.
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

Identification::StarsEither Dot::reduce () {
    ch->select_table(table_name), nu = 0;

    for (const Star &c : *be->get_image()) {
        Star::trio b = find_closest(c);
        TriosEither big_r = find_candidate_trio(b[0], b[1], b[2]);

        // Practical limit: exit early if we have iterated through too many comparisons without match.
        if (nu > nu_max) return StarsEither{{}, NO_CONFIDENT_R_EITHER};

        // The reduction step: |R| = 1.
        if (big_r.error == NO_CANDIDATE_TRIO_FOUND_EITHER) continue;
        else return StarsEither{{big_r.result[0], big_r.result[1], big_r.result[2]}, 0};
    }

    return StarsEither{{}, NO_CONFIDENT_R_EITHER};
}

Identification::StarsEither Dot::identify () {
    nu = 0;

    for (int i = 0; i < static_cast<signed> (be->get_image()->size() - 2); i++) {
	    for (int j = i + 1; j < static_cast<signed> (be->get_image()->size() - 1); j++) {
		    for (int k = i + 2; k < static_cast<signed> (be->get_image()->size()); k++) {
			    Star::trio b = {be->get_image()->at(i),
				            be->get_image()->at(j),
				            be->get_image()->at(k)};
    // for (const Star &c : *be->get_image()) {
        // Star::trio b = find_closest(c);
        bool is_swapped = false;

        // Practical limit: exit early if we have iterated through too many comparisons without match.
        if (nu > nu_max) return StarsEither{{}, EXCEEDED_NU_MAX_EITHER};

        // Determine which stars map to the current 'b'. If this fails, swap b_i and b_j.
        std::cout << "[DOT] Finding candidate trio." << std::endl;
        TriosEither r = find_candidate_trio(b[0], b[1], b[2]);
        // if (r.error == NO_CANDIDATE_TRIO_FOUND_EITHER) {
        //     std::cout << "[DOT] Reversing candidate trio." << std::endl;
        //     r = find_candidate_trio(b[0], b[1], b[2]), is_swapped = true;
        // }

        // If there exist no matches at this point, then repeat for another pair.
        if (r.error == NO_CANDIDATE_TRIO_FOUND_EITHER) continue;

        // Otherwise, attach the labels to the body and return this set.
        std::cout << "[DOT] Match found!" << std::endl;
        return StarsEither{Star::list{
                Star::define_label(b[2], r.result[2].get_label()),
                Star::define_label(b[(is_swapped) ? 1 : 0], r.result[0].get_label()),
                Star::define_label(b[(is_swapped) ? 0 : 1], r.result[1].get_label())
        }, 0};
		    }}}
    // }

    return StarsEither{{}, NO_CONFIDENT_A_EITHER};
}
