/// @file pyramid.cpp
/// @author Glenn Galvizo
///
/// Source file for Pyramid class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#define _USE_MATH_DEFINES

#include <cmath>
#include <algorithm>

#include "math/random-draw.h"
#include "identification/pyramid.h"

const unsigned int Pyramid::QUERY_STAR_SET_SIZE = 3;
const int Pyramid::NO_CONFIDENT_R_FOUND_EITHER = -2;

int Pyramid::generate_table (const std::shared_ptr<Chomp> &ch, double fov, const std::string &table_name) {
    return Angle::generate_table(ch, fov, table_name);
}

Pyramid::labels_list_list Pyramid::query_for_pairs (const double theta) {
    // Noise is normally distributed. Angle within 3 sigma of theta.
    Chomp::tuples_d big_r_mn_tuples;
    labels_list_list big_r_mn_ell;

    // Query using theta with epsilon bounds.
    ch->select_table(this->table_name);
    big_r_mn_tuples = ch->simple_bound_query(
            {"theta"},
            "label_a, label_b, theta",
            {theta - epsilon_1},
            {theta + epsilon_1},
            500
    );
    (this->nu)++;

    // Append the results to our candidate list.
    big_r_mn_ell.reserve(big_r_mn_tuples.size());
    for (const Nibble::tuple_d &result: big_r_mn_tuples) {
        big_r_mn_ell.emplace_back(labels_list{(static_cast<int> (result[0])), static_cast<int> (result[1])});
    }

    return big_r_mn_ell;
}

/// Given two list of labels, determine the common stars that exist in both lists. Remove all stars "removed" in this
/// "intersection" if there exist any.
Star::list Pyramid::common (const labels_list_list &big_r_ab_ell, const labels_list_list &big_r_ac_ell,
                            const Star::list &removed) {
    static auto flatten_pairs = [] (const labels_list_list &big_r_ell, labels_list &out_ell) -> void {
        out_ell.reserve(big_r_ell.size() * 2);
        for (const labels_list &candidate : big_r_ell) {
            out_ell.push_back(candidate[0]);
            out_ell.push_back(candidate[1]);
        }
        std::sort(out_ell.begin(), out_ell.end());
    };

    // Flatten our list of lists.
    labels_list ab_ell_list, ac_ell_list, a_ell_list;
    flatten_pairs(big_r_ab_ell, ab_ell_list), flatten_pairs(big_r_ac_ell, ac_ell_list);

    // Find the intersection between lists AB and AC.
    std::set_intersection(ab_ell_list.begin(), ab_ell_list.end(), ac_ell_list.begin(), ac_ell_list.end(),
                          std::back_inserter(a_ell_list));

    // Remove any stars in I that exist in "removed".
    a_ell_list.erase(std::remove_if(a_ell_list.begin(), a_ell_list.end(), [&removed] (const int &ell) -> bool {
        for (const Star &s : removed) {
            if (s.get_label() == ell) return true;
        }
        return false;
    }), a_ell_list.end());

    // For each common label, retrieve the star from Nibble.
    Star::list big_r_a;
    for (const int &ell : a_ell_list) {
        big_r_a.push_back(ch->query_hip(static_cast<int> (ell)));
    }
    return big_r_a.empty() ? Star::list{} : big_r_a;
}

/// Overloaded common method. Given three list of labels, determine the common stars that exist in both lists. Remove
/// all stars "removed" in this "intersection" if there exist any.
Star::list Pyramid::common (const labels_list_list &big_r_ae_ell, const labels_list_list &big_r_be_ell,
                            const labels_list_list &big_r_ce_ell, const Star::list &removed) {
    static auto flatten_pairs = [] (const labels_list_list &big_r_ell, labels_list &out_ell) -> void {
        out_ell.reserve(big_r_ell.size() * 2);
        for (const labels_list &candidate : big_r_ell) {
            out_ell.push_back(candidate[0]);
            out_ell.push_back(candidate[1]);
        }
        std::sort(out_ell.begin(), out_ell.end());
    };

    // Flatten our list of lists.
    labels_list ae_ell_list, be_ell_list, ce_ell_list, abe_ell_list, e_ell_list;
    flatten_pairs(big_r_ae_ell, ae_ell_list), flatten_pairs(big_r_be_ell, be_ell_list);
    flatten_pairs(big_r_ce_ell, ce_ell_list);

    // Find the intersection between lists AE and BE, and then the 2nd intersection between CE.
    std::set_intersection(ae_ell_list.begin(), ae_ell_list.end(), be_ell_list.begin(), be_ell_list.end(),
                          std::back_inserter(abe_ell_list));
    std::set_intersection(abe_ell_list.begin(), abe_ell_list.end(), ce_ell_list.begin(), ce_ell_list.end(),
                          std::back_inserter(e_ell_list));

    // Remove any stars in I that exist in "removed".
    e_ell_list.erase(std::remove_if(e_ell_list.begin(), e_ell_list.end(), [&removed] (const int &ell) -> bool {
        for (const Star &s : removed) {
            if (s.get_label() == ell) return true;
        }
        return false;
    }), e_ell_list.end());

    // For each common label, retrieve the star from Nibble.
    Star::list big_r_a;
    for (const int &ell : e_ell_list) {
        big_r_a.push_back(ch->query_hip(static_cast<int> (ell)));
    }
    return big_r_a.empty() ? Star::list{} : big_r_a;
}

bool Pyramid::verification (const Star::trio &r, const Star::trio &b) {
    // Select a random star E. This must not exist in the current body trio.
    Star b_e;
    do {
        b_e = (be->get_image()->at(RandomDraw::draw_integer(0, b.size())));
    } while (std::find(b.begin(), b.end(), b_e) != b.end());

    // Find all star pairs between IE, JE, and KE.
    auto find_pairs = [this, &b_e, &b] (const int a) -> labels_list_list {
        return this->query_for_pairs((180.0 / M_PI) * Vector3::Angle(b[a], b_e.get_vector()));
    };
    labels_list_list big_r_ie_ell = find_pairs(0), big_r_je_ell = find_pairs(1), big_r_ke_ell = find_pairs(2);

    // Determine the star E in the catalog using common stars.
    Star::list big_t_e = common(big_r_ie_ell, big_r_je_ell, big_r_ke_ell, Star::list{});

    // If there isn't exactly one star, exit here.
    if (big_t_e.size() != 1 || big_t_e.empty()) {
        std::cout << "[PYRAMID] Verification failed." << std::endl;
        return false;
    }

    // If this star is near our R set in the catalog, then this test has passed.
    std::cout << "[PYRAMID] Verification passed." << std::endl;
    return Star::within_angle({r[0], r[1], r[2], big_t_e[0]}, be->get_fov());
}

/// Given a trio of indices from the input set, determine the matching catalog IDs that correspond to each star.
/// Two verification steps occur: the singular element test and the fourth star test. If these are not met, then the
/// error trio is returned.
Pyramid::TriosEither Pyramid::find_catalog_stars (const Star::trio &b) {
    std::cout << "[PYRAMID] Finding catalog stars." << std::endl;
    auto find_pairs = [this, &b] (const int m, const int n) -> labels_list_list {
        return this->query_for_pairs((180.0 / M_PI) * Vector3::Angle(b[m], b[n]));
    };
    labels_list_list big_r_ij_ell = find_pairs(0, 1), big_r_ik_ell = find_pairs(0, 2), big_r_jk_ell = find_pairs(1, 2);

    // Determine the star I, J, and K in the catalog using common stars.
    Star::list big_t_i = common(big_r_ij_ell, big_r_ik_ell, Star::list{});
    Star::list big_t_j = common(big_r_ij_ell, big_r_jk_ell, big_t_i), removed = big_t_i;
    removed.insert(removed.end(), big_t_j.begin(), big_t_j.end());
    Star::list big_t_k = common(big_r_ik_ell, big_r_jk_ell, removed);
    if (big_t_i.size() != 1 || big_t_j.size() != 1 || big_t_k.size() != 1) {
        return TriosEither{{}, NO_CONFIDENT_R_FOUND_EITHER};
    }
    else return TriosEither{{big_t_i[0], big_t_j[0], big_t_k[0]}, 0};
}

/// Identification determination process for a single identification trio.
Pyramid::StarsEither Pyramid::identify_as_list (const Star::list &b) {
    TriosEither r = find_catalog_stars({b[0], b[1], b[2]});
    if (r.error == NO_CONFIDENT_R_FOUND_EITHER) return StarsEither{{}, NO_CONFIDENT_A_EITHER};

    auto attach_label = [&b, &r] (const int i) -> Star {
        return Star::define_label(b[i], r.result[i].get_label());
    };
    return StarsEither{Star::list{attach_label(0), attach_label(1), attach_label(2)}, 0};
}

std::vector<Identification::labels_list> Pyramid::query () {
    auto find_pairs = [this] (const int a, const int b) -> labels_list_list {
        return this->query_for_pairs((180.0 / M_PI) * Vector3::Angle(be->get_image()->at(a), be->get_image()->at(b)));
    };
    labels_list_list big_r_ij_ell = find_pairs(0, 1), big_r_ik_ell = find_pairs(0, 2), big_r_jk_ell = find_pairs(1, 2);

    // Determine the star I, J, and K in the catalog using common stars.
    Star::list t_i = common(big_r_ij_ell, big_r_ik_ell, Star::list{});
    Star::list t_j = common(big_r_ij_ell, big_r_jk_ell, t_i), h = t_i;
    h.insert(h.end(), t_j.begin(), t_j.end());
    Star::list t_k = common(big_r_ik_ell, big_r_jk_ell, h);

    // Form trios all of combinations from t_i, t_j, and t_k.
    std::vector<Identification::labels_list> big_r_ell;
    for (const Star &s_i : t_i) {
        for (const Star &s_j : t_j) {
            for (const Star &s_k : t_k) {
                big_r_ell.emplace_back(labels_list{s_i.get_label(), s_j.get_label(), s_k.get_label()});
            }
        }
    }
    return big_r_ell;
}

Pyramid::StarsEither Pyramid::reduce () {
    ch->select_table(this->table_name);
    this->nu = 0;

    for (unsigned int dj = 1; dj < be->get_image()->size() - 1; dj++) {
        for (unsigned int dk = 1; dk < be->get_image()->size() - dj - 1; dk++) {
            for (unsigned int di = 0; di < be->get_image()->size() - dj - dk - 1; di++) {
                int i = di, j = di + dj, k = j + dk;
                StarsEither b = identify_as_list({
                                                         be->get_image()->at(i),
                                                         be->get_image()->at(j),
                                                         be->get_image()->at(k)
                                                 });

                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (nu > this->nu_max) return StarsEither{{}, NO_CONFIDENT_R_EITHER};

                // The reduction step: |R| = 1 (in terms of B here).
                if (b.error == NO_CONFIDENT_A_EITHER) continue;

                return StarsEither{Star::list{ch->query_hip(b.result[0].get_label()),
                                              ch->query_hip(b.result[1].get_label()),
                                              ch->query_hip(b.result[2].get_label())}, 0};
            }
        }
    }

    return StarsEither{{}, NO_CONFIDENT_R_EITHER};
}

Pyramid::StarsEither Pyramid::identify () {
    nu = 0;

    // This procedure will not work |big_i| < 4. Exit early with NO_CONFIDENT_A.
    if (be->get_image()->size() < 4) return StarsEither{{}, NO_CONFIDENT_A_EITHER};

    // Otherwise, there exists |big_i| choose 3 possibilities. Looping specified in paper.
    for (unsigned int dj = 1; dj < be->get_image()->size() - 1; dj++) {
        for (unsigned int dk = 1; dk < be->get_image()->size() - dj - 1; dk++) {
            for (unsigned int di = 0; di < be->get_image()->size() - dj - dk - 1; di++) {
                int i = di, j = di + dj, k = j + dk;
                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (nu > this->nu_max) return StarsEither{{}, EXCEEDED_NU_MAX_EITHER};

                // Given three stars in our catalog, find their catalog IDs in the catalog.
                Star::trio b = {be->get_image()->at(i), be->get_image()->at(j), be->get_image()->at(k)};
                TriosEither r = find_catalog_stars(b);
                if (r.error == NO_CONFIDENT_R_FOUND_EITHER) continue;

                // Run this through the verification step.
                if (!verification(r.result, b)) continue;
                else {
                    std::cout << "[PYRAMID] Match found!" << std::endl;
                    return StarsEither{Star::list{
                            Star::define_label(be->get_image()->at(i), r.result[0].get_label()),
                            Star::define_label(be->get_image()->at(j), r.result[1].get_label()),
                            Star::define_label(be->get_image()->at(k), r.result[2].get_label())
                    }, 0};
                }
            }
        }
    }

    return StarsEither{{}, NO_CONFIDENT_A_EITHER};
}
