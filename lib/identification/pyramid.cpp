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

/// Exact number of query stars required for query experiment.
const unsigned int Pyramid::QUERY_STAR_SET_SIZE = 3;

/// Default parameters for the pyramid identification method.
const Identification::Parameters Pyramid::DEFAULT_PARAMETERS = {DEFAULT_SIGMA_QUERY, DEFAULT_SIGMA_QUERY,
    DEFAULT_SIGMA_QUERY, DEFAULT_SIGMA_4, DEFAULT_SQL_LIMIT, DEFAULT_NO_REDUCTION, DEFAULT_FAVOR_BRIGHT_STARS,
    DEFAULT_NU_MAX, DEFAULT_NU, DEFAULT_F, "PYRAMID_20"};

/// Returned when there exists no common stars between the label list pairs.
const Star::list Pyramid::NO_COMMON_FOUND = {Star::wrap(Vector3::Zero())};

/// Returned when there exists no candidate triangle to be found for the given three stars.
const Star::trio Pyramid::NO_CONFIDENT_R_FOUND = {Star::wrap(Vector3::Zero()), Star::wrap(Vector3::Zero()),
    Star::wrap(Vector3::Zero())};

/// Constructor. Sets the benchmark data, fov, parameters, and current working table.
///
/// @param input Working Benchmark instance. We are **only** copying the star set and the fov.
Pyramid::Pyramid (const Benchmark &input, const Parameters &p) : Identification() {
    input.present_image(this->big_i, this->fov);
    this->parameters = p;
    
    ch.select_table(this->parameters.table_name);
}

/// The Pyramid method uses the exact same table as the Angle method. Wrap Angle's 'generate_table' method.
///
/// @param cf Configuration reader holding all parameters to use.
/// @return TABLE_ALREADY_EXISTS if the table already exists. Otherwise, 0 when finished.
int Pyramid::generate_table (INIReader &cf) {
    return Angle::generate_table(cf, "pyramid");
}

/// Find all star pairs whose angle of separation is with 3 * query_sigma (epsilon) degrees of each other.
///
/// @param theta Separation angle (degrees) to search with.
/// @return List of star lists (of size = 2) that fall within epsilon degrees of theta.
Pyramid::labels_list_list Pyramid::query_for_pairs (const double theta) {
    // Noise is normally distributed. Angle within 3 sigma of theta.
    double epsilon = 3.0 * this->parameters.sigma_1;
    Chomp::tuples_d big_r_mn_tuples;
    labels_list_list big_r_mn_ell;
    
    // Query using theta with epsilon bounds.
    ch.select_table(parameters.table_name);
    big_r_mn_tuples = ch.simple_bound_query({"theta"}, "label_a, label_b, theta", {theta - epsilon}, {theta + epsilon},
                                            3 * this->parameters.sql_limit);
    
    // Append the results to our candidate list.
    big_r_mn_ell.reserve(big_r_mn_tuples.size());
    for (const Nibble::tuple_d &result: big_r_mn_tuples) {
        big_r_mn_ell.emplace_back(labels_list {(static_cast<int> (result[0])), static_cast<int> (result[1])});
    }
    
    return big_r_mn_ell;
}

/// Given two list of labels, determine the common stars that exist in both lists. Remove all stars "removed" in this
/// "intersection" if there exist any.
///
/// @param big_r_ab_ell List of AB star pairs. We are trying to determine A.
/// @param big_r_ac_ell List of AC star pairs. We are trying to determine A.
/// @param removed Our "removed" list. Remove any labels in the "intersection" that exist in this set's labels.
/// @return All common stars between AB and AC, with F removed.
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
            if (s.get_label() == ell) {
                return true;
            }
        }
        return false;
    }), a_ell_list.end());
    
    // For each common label, retrieve the star from Nibble.
    Star::list big_r_a;
    for (const int &ell : a_ell_list) {
        big_r_a.push_back(ch.query_hip(static_cast<int> (ell)));
    }
    return big_r_a.empty() ? NO_COMMON_FOUND : big_r_a;
}

/// Overloaded common method. Given three list of labels, determine the common stars that exist in both lists. Remove
/// all stars "removed" in this "intersection" if there exist any.
///
/// @param big_r_ae_ell List of AE star pairs. We are trying to determine E.
/// @param big_r_be_ell List of BE star pairs. We are trying to determine E.
/// @param big_r_ce_ell List of CE star pairs. We are trying to determine E.
/// @param removed Our "removed" list. Remove any labels in the "intersection" that exist in this set's labels.
/// @return All common stars between AE, BE, and CE.
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
            if (s.get_label() == ell) {
                return true;
            }
        }
        return false;
    }), e_ell_list.end());
    
    // For each common label, retrieve the star from Nibble.
    Star::list big_r_a;
    for (const int &ell : e_ell_list) {
        big_r_a.push_back(ch.query_hip(static_cast<int> (ell)));
    }
    return big_r_a.empty() ? NO_COMMON_FOUND : big_r_a;
}

/// Given the current map pair, select a random star in the image and verify that the identification is correct.
///
/// @param r Current reference star trio.
/// @param b Current body star trio.
/// @return True if the verification has passed. Otherwise, false.
bool Pyramid::verification (const Star::trio &r, const Star::trio &b) {
    // Select a random star E. This must not exist in the current body trio.
    Star b_e;
    do {
        b_e = big_i[RandomDraw::draw_integer(0, b.size())];
    }
    while (std::find(b.begin(), b.end(), b_e) != b.end());
    
    // Find all star pairs between IE, JE, and KE.
    auto find_pairs = [this, &b_e, &b] (const int a) -> labels_list_list {
        return this->query_for_pairs((180.0 / M_PI) * Vector3::Angle(b[a], b_e));
    };
    labels_list_list big_r_ie_ell = find_pairs(0), big_r_je_ell = find_pairs(1), big_r_ke_ell = find_pairs(2);
    
    // Determine the star E in the catalog using common stars.
    Star::list big_t_e = common(big_r_ie_ell, big_r_je_ell, big_r_ke_ell, Star::list {});
    
    // If there isn't exactly one star, exit here.
    if (big_t_e.size() != 1 || std::equal(big_t_e.begin(), big_t_e.end(), NO_COMMON_FOUND.begin())) {
        return false;
    }
    
    // If this star is near our R set in the catalog, then this test has passed.
    return Star::within_angle({r[0], r[1], r[2], big_t_e[0]}, fov);
}

/// Given a trio of indices from the input set, determine the matching catalog IDs that correspond to each star.
/// Two verification steps occur: the singular element test and the fourth star test. If these are not met, then the
/// error trio is returned.
///
/// @param b Trio of stars in our body frame.
/// @return NO_CONFIDENT_R_FOUND if no triangle can be found. Otherwise, the inertial stars.
Star::trio Pyramid::find_catalog_stars (const Star::trio &b) {
    auto find_pairs = [this, &b] (const int m, const int n) -> labels_list_list {
        return this->query_for_pairs((180.0 / M_PI) * Vector3::Angle(b[m], b[n]));
    };
    labels_list_list big_r_ij_ell = find_pairs(0, 1), big_r_ik_ell = find_pairs(0, 2), big_r_jk_ell = find_pairs(1, 2);
    
    // Determine the star I, J, and K in the catalog using common stars.
    Star::list big_t_i = common(big_r_ij_ell, big_r_ik_ell, Star::list {});
    Star::list big_t_j = common(big_r_ij_ell, big_r_jk_ell, big_t_i), removed = big_t_i;
    removed.insert(removed.end(), big_t_j.begin(), big_t_j.end());
    Star::list big_t_k = common(big_r_ik_ell, big_r_jk_ell, removed);
    
    // |R| = 1 restriction. If these lists do not contain exactly one element, break.
    if (big_t_i.size() == 1 && big_t_j.size() == 1 && big_t_k.size() == 1 && this->parameters.no_reduction) {
        return NO_CONFIDENT_R_FOUND;
    }
    
    // Favor bright stars if specified. Applied with the FAVOR_BRIGHT_STARS flag.
    Star::trio r = {big_t_i[0], big_t_j[0], big_t_k[0]};
    if (this->parameters.favor_bright_stars) {
        // Form trios all of combinations from big_t_i, big_t_j, and big_t_k.
        std::vector<Identification::labels_list> r_ell;
        for (const Star &s_i : big_t_i) {
            for (const Star &s_j : big_t_j) {
                for (const Star &s_k : big_t_k) {
                    r_ell.emplace_back(labels_list {s_i.get_label(), s_j.get_label(), s_k.get_label()});
                }
            }
        }
        
        sort_brightness(r_ell);
        r = {ch.query_hip(r_ell[0][0]), ch.query_hip(r_ell[0][1]), ch.query_hip(r_ell[0][2])};
    }
    
    return r;
}

/// Identification determination process for a single identification trio.
///
/// @param b Body (frame B) pair of stars that match the inertial pair.
/// @return NO_CONFIDENT_A if an identification quad cannot be found. Otherwise, body stars b with the attached
/// labels of the inertial pair r.
Star::list Pyramid::identify_as_list (const Star::list &b) {
    Star::trio r = find_catalog_stars({b[0], b[1], b[2]});
    if (std::equal(r.begin(), r.end(), NO_CONFIDENT_R_FOUND.begin())) {
        return NO_CONFIDENT_A;
    }
    
    auto attach_label = [&b, &r] (const int i) -> Star {
        return Star::define_label(b[i], r[i].get_label());
    };
    return {attach_label(0), attach_label(1), attach_label(2)};
}

/// Reproduction of the Pyramid method's database querying. Input image is not used. We require the following be
/// defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_query
///     - sql_limit
/// @endcode
///
/// @param s Stars to query with. This must be of length = QUERY_STAR_SET_SIZE.
/// @return Vector of likely matches found by the pyramid method.
std::vector<Identification::labels_list> Pyramid::query (const Star::list &s) {
    if (s.size() != QUERY_STAR_SET_SIZE) {
        throw std::runtime_error(std::string("Input list does not have exactly three b."));
    }
    
    auto find_pairs = [this, &s] (const int a, const int b) -> labels_list_list {
        return this->query_for_pairs((180.0 / M_PI) * Vector3::Angle(s[a], s[b]));
    };
    labels_list_list big_r_ij_ell = find_pairs(0, 1), big_r_ik_ell = find_pairs(0, 2), big_r_jk_ell = find_pairs(1, 2);
    
    // Determine the star I, J, and K in the catalog using common stars.
    Star::list t_i = common(big_r_ij_ell, big_r_ik_ell, Star::list {});
    Star::list t_j = common(big_r_ij_ell, big_r_jk_ell, t_i), h = t_i;
    h.insert(h.end(), t_j.begin(), t_j.end());
    Star::list t_k = common(big_r_ik_ell, big_r_jk_ell, h);
    
    // Form trios all of combinations from t_i, t_j, and t_k.
    std::vector<Identification::labels_list> big_r_ell;
    for (const Star &s_i : t_i) {
        for (const Star &s_j : t_j) {
            for (const Star &s_k : t_k) {
                if (s_i != NO_COMMON_FOUND[0]) {
                    big_r_ell.emplace_back(labels_list {s_i.get_label(), s_j.get_label(), s_k.get_label()});
                }
                
                // Follow the SQL limit in the parameters.
                if (big_r_ell.size() == parameters.sql_limit) {
                    return big_r_ell;
                }
            }
        }
    }
    return big_r_ell;
}

/// Finds the single, most likely result for the first four stars in our current benchmark. This trial is
/// basically the same as the first identification trials. Input image is used. We require the following to be defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_query
///     - sql_limit
///     - nu
///     - nu_max
/// @endcode
///
/// @return EMPTY_BIG_R_ELL if a candidate quad cannot be found. Otherwise, a single match configuration found
/// by the angle method.
Star::list Pyramid::reduce () {
    ch.select_table(parameters.table_name);
    *parameters.nu = 0;
    
    for (unsigned int dj = 1; dj < big_i.size() - 1; dj++) {
        for (unsigned int dk = 1; dk < big_i.size() - dj - 1; dk++) {
            for (unsigned int di = 0; di < big_i.size() - dj - dk - 1; di++) {
                int i = di, j = di + dj, k = j + dk;
                Star::list b = identify_as_list({big_i[i], big_i[j], big_i[k]});
                (*parameters.nu)++;
    
                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (*parameters.nu > parameters.nu_max) {
                    return NO_CONFIDENT_R;
                }
                
                // The reduction step: |R| = 1 (in terms of B here).
                if (std::equal(b.begin(), b.end(), NO_CONFIDENT_A.begin())) {
                    continue;
                }
                return {ch.query_hip(b[0].get_label()), ch.query_hip(b[1].get_label()), ch.query_hip(b[2].get_label())};
            }
        }
    }
    
    return NO_CONFIDENT_R;
}

/// Reproduction of the Pyramid method's process from beginning to the orientation determination. Input image is used.
/// We require the following be defined:
///
/// @code{.cpp}
///     - table_name
///     - sql_limit
///     - sigma_query
///     - nu
///     - nu_max
/// @endcode
///
/// @param input The set of benchmark data to work with.
/// @param p Adjustments to the identification process.
/// @return NO_CONFIDENT_A if an identification cannot be found exhaustively. EXCEEDED_NU_MAX if an
/// identification cannot be found within a certain number of query picks. Otherwise, body stars b with the attached
/// labels of the inertial pair r.
Star::list Pyramid::identify () {
    *parameters.nu = 0;
    
    // This procedure will not work |big_i| < 4. Exit early with NO_CONFIDENT_A.
    if (big_i.size() < 4) {
        return NO_CONFIDENT_A;
    }
    
    // Otherwise, there exists |big_i| choose 3 possibilities. Looping specified in paper.
    for (unsigned int dj = 1; dj < big_i.size() - 1; dj++) {
        for (unsigned int dk = 1; dk < big_i.size() - dj - 1; dk++) {
            for (unsigned int di = 0; di < big_i.size() - dj - dk - 1; di++) {
                int i = di, j = di + dj, k = j + dk;
                (*parameters.nu)++;
                
                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (*parameters.nu > parameters.nu_max) {
                    return EXCEEDED_NU_MAX;
                }
                
                // Given three stars in our catalog, find their catalog IDs in the catalog.
                Star::trio b = {big_i[i], big_i[j], big_i[k]};
                Star::trio r = find_catalog_stars(b);
                if (std::equal(r.begin(), r.end(), NO_CONFIDENT_R_FOUND.begin())) {
                    continue;
                }

                // Run this through the verification step.
                if (!verification(r, b) && !parameters.no_reduction) {
                    continue;
                }
                
                return {Star::define_label(big_i[i], r[0].get_label()), Star::define_label(big_i[j], r[1].get_label()),
                    Star::define_label(big_i[k], r[2].get_label())};
            }
        }
    }
    
    return NO_CONFIDENT_A;
}
