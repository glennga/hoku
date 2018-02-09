/// @file pyramid.cpp
/// @author Glenn Galvizo
///
/// Source file for Pyramid class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#include <algorithm>

#include "math/random-draw.h"
#include "identification/pyramid.h"

/// Default parameters for the pyramid identification method.
const Identification::Parameters Pyramid::DEFAULT_PARAMETERS = {DEFAULT_SIGMA_QUERY, DEFAULT_SQL_LIMIT,
    DEFAULT_PASS_R_SET_CARDINALITY, DEFAULT_SIGMA_OVERLAY, DEFAULT_NU_MAX, DEFAULT_NU, DEFAULT_F, "PYRAMID_20"};

/// Returned when there exists no common stars between the label list pairs.
const Star::list Pyramid::NO_COMMON_FOUND = {Star::zero()};

/// Returned when there exists no candidate triangle to be found for the given three stars.
const Pyramid::star_trio Pyramid::NO_CANDIDATE_TRIANGLE_FOUND = {Star::zero(), Star::zero(), Star::zero()};

/// Constructor. Sets the benchmark data, fov, parameters, and current working table.
///
/// @param input Working Benchmark instance. We are **only** copying the star set and the fov.
Pyramid::Pyramid (const Benchmark &input, const Parameters &p) : Identification() {
    input.present_image(this->input, this->fov);
    this->parameters = p;
    
    ch.select_table(this->parameters.table_name);
}

/// The Pyramid method uses the exact same table as the Angle method. Wrap Angle's 'generate_table' method.
///
/// @param fov Field of view limit (degrees) that all pairs must be within.
/// @param table_name Name of the table to generate.
/// @return TABLE_ALREADY_EXISTS if the table already exists. Otherwise, 0 when finished.
int Pyramid::generate_table (const double fov, const std::string &table_name) {
    return Angle::generate_table(fov, table_name);
}

/// Find all star pairs whose angle of separation is with 3 * query_sigma (epsilon) degrees of each other.
///
/// @param theta Separation angle (degrees) to search with.
/// @return List of star pairs that fall within epsilon degrees of theta.
Pyramid::label_list_pair Pyramid::query_for_pairs (const double theta) {
    // Noise is normally distributed. Angle within 3 sigma of theta.
    double epsilon = 3.0 * this->parameters.sigma_query;
    Chomp::tuples_d results;
    label_list_pair candidates;
    
    // Query using theta with epsilon bounds.
    ch.select_table(parameters.table_name);
    results = ch.simple_bound_query("theta", "label_a, label_b, theta", theta - epsilon, theta + epsilon,
                                    3 * this->parameters.sql_limit);
    
    // Append the results to our candidate list.
    candidates.reserve(results.size() / 2);
    for (const Nibble::tuple_d &result: results) {
        candidates.push_back(label_pair {(static_cast<int> (result[0])), static_cast<int> (result[1])});
    }
    
    return candidates;
}

/// Given two list of labels, determine the common stars that exist in both lists. Remove all stars "F" in this
/// "intersection" if there exist any.
///
/// @param r_ab List of AB star pairs. We are trying to determine A.
/// @param r_ac List of AC star pairs. We are trying to determine A.
/// @param f Our "removed" list. Remove any labels in the "intersection" that exist in this set's labels.
/// @return All common stars between AB and AC, with F removed.
Star::list Pyramid::common_stars (const label_list_pair &r_ab, const label_list_pair &r_ac, const Star::list &f) {
    auto flatten_pairs = [] (const label_list_pair &candidates, labels_list &out_list) -> void {
        out_list.reserve(candidates.size() * 2);
        for (const label_pair &candidate : candidates) {
            out_list.push_back(candidate[0]);
            out_list.push_back(candidate[1]);
        }
        std::sort(out_list.begin(), out_list.end());
    };
    
    // Flatten our list of pairs.
    labels_list ab_list, ac_list, i, i_r;
    flatten_pairs(r_ab, ab_list), flatten_pairs(r_ac, ac_list);
    
    // Find the intersection between lists AB and AC.
    std::set_intersection(ab_list.begin(), ab_list.end(), ac_list.begin(), ac_list.end(), std::back_inserter(i));
    
    // Remove any stars in I that exist in F.
    labels_list r_f;
    for (const Star &s : f) {
        r_f.push_back(s.get_label());
    }
    std::set_difference(i.begin(), i.end(), r_f.begin(), r_f.end(), std::back_inserter(i_r));
    
    // For each common label, retrieve the star from Nibble.
    Star::list common;
    for (const int &ell : i_r) {
        common.push_back(ch.query_hip(static_cast<int> (ell)));
    }
    return common.empty() ? NO_COMMON_FOUND : common;
}

/// Given the current map pair, select a random star in the image and verify that the identification is correct.
///
/// @param r Current reference star trio.
/// @param b Current body star trio.
/// @return True if the verification has passed. Otherwise, false.
bool Pyramid::verification (const star_trio &r, const star_trio &b_f) {
    // Select a random star E. This must not exist in the current body trio.
    Star e;
    do {
        e = input[RandomDraw::draw_integer(0, b_f.size())];
    }
    while (std::find(b_f.begin(), b_f.end(), e) != b_f.end());
    
    // Find all star pairs between IE, JE, and KE.
    auto find_pairs = [this, &e, &b_f] (const int a) -> label_list_pair {
        return this->query_for_pairs(Star::angle_between(b_f[a], e));
    };
    label_list_pair r_ie = find_pairs(0), r_je = find_pairs(1), r_ke = find_pairs(2), h;
    
    // Determine the star E in the catalog using common stars.
    std::set_union(r_ie.begin(), r_ie.end(), r_je.begin(), r_je.end(), std::back_inserter(h));
    Star::list t_e = common_stars(r_ke, h, Star::list {});
    
    // If there isn't exactly one star, exit here.
    if (t_e.size() != 1) {
        return false;
    }
    
    // If this star is near our R set in the catalog, then this test has passed.
    return Star::within_angle({r[0], r[1], r[2], t_e[0]}, fov);
}

/// Given a trio of indices from the input set, determine the matching catalog IDs that correspond to each star.
/// Two verification steps occur: the singular element test and the fourth star test. If these are not met, then the
/// error trio is returned.
///
/// @param b_f Trio of stars in our body frame.
/// @return NO_CANDIDATE_TRIANGLE_FOUND if no triangle can be found. Otherwise, the catalog IDs of stars from the
/// inertial frame.
Pyramid::star_trio Pyramid::find_candidate_trio (const star_trio &b_f) {
    auto find_pairs = [this, &b_f] (const int a, const int b) -> label_list_pair {
        return this->query_for_pairs(Star::angle_between(b_f[a], b_f[b]));
    };
    label_list_pair r_ij = find_pairs(0, 1), r_ik = find_pairs(0, 2), r_jk = find_pairs(1, 2);
    
    // Determine the star I, J, and K in the catalog using common stars.
    Star::list t_i = common_stars(r_ij, r_ik, Star::list {});
    Star::list t_j = common_stars(r_ij, r_jk, t_i), h = t_i;
    h.insert(h.end(), t_j.begin(), t_j.end());
    Star::list t_k = common_stars(r_ik, r_jk, h);
    
    // |R| = 1 restriction. If these lists do not contain exactly one element, break.
    if ((t_i.size() + t_j.size() + t_k.size() != 3) && this->parameters.pass_r_set_cardinality) {
        return NO_CANDIDATE_TRIANGLE_FOUND;
    }
    
    // Otherwise, attempt to verify the triangle.
    star_trio r = {t_i[0], t_j[0], t_k[0]};
    if (!verification(r, b_f)) {
        return NO_CANDIDATE_TRIANGLE_FOUND;
    }
    
    return r;
}

/// Identification determination process for a single reference star and identification trio.
///
/// @param b Body (frame B) pair of stars that match the inertial pair.
/// @return NO_CONFIDENT_IDENTITY if an identification quad cannot be found. Otherwise, body stars b with the attached
/// labels of the inertial pair r.
Star::list Pyramid::singular_identification (const Star::list &b) {
    star_trio r_quad = find_candidate_trio({b[0], b[1], b[2]});
    if (std::equal(r_quad.begin(), r_quad.end(), NO_CANDIDATE_TRIANGLE_FOUND.begin())) {
        return NO_CONFIDENT_IDENTITY;
    }
    
    auto attach_label = [&b, &r_quad] (const int i) -> Star {
        return Star::define_label(b[i], r_quad[i].get_label());
    };
    return {attach_label(0), attach_label(1), attach_label(2), attach_label(3)};
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
        throw std::runtime_error(std::string("Input list does not have exactly two stars."));
    }
    double epsilon = 3.0 * this->parameters.sigma_query, theta = Star::angle_between(s[0], s[1]);
    std::vector<labels_list> r_bar;
    
    // Query using theta with epsilon bounds.
    Nibble::tuples_d r = ch.simple_bound_query("theta", "label_a, label_b", theta - epsilon, theta + epsilon,
                                               this->parameters.sql_limit);
    
    // Sort tuple_d into list of catalog ID pairs.
    r_bar.reserve(r.size() / 2);
    for (const Nibble::tuple_d &r_t : r) {
        r_bar.emplace_back(labels_list {static_cast<int> (r_t[0]), static_cast<int> (r_t[1])});
    }
    
    return r_bar;
}

/// Finds the single, most likely result for the first four stars in our current benchmark. This trial is
/// basically the same as the first identification trials. Input image is used. We require the following to be defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_query
///     - sql_limit
/// @endcode
///
/// @return NO_CANDIDATES_FOUND if a candidate quad cannot be found. Otherwise, a single match configuration found
/// by the angle method.
Identification::labels_list Pyramid::reduce () {
    Star::list c = singular_identification({input[0], input[1], input[2]});
    if (std::equal(c.begin(), c.end(), NO_CONFIDENT_IDENTITY.begin())) {
        return NO_CANDIDATES_FOUND;
    }
    return labels_list {c[0].get_label(), c[1].get_label(), c[2].get_label()};
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
/// @return NO_CONFIDENT_IDENTITY if an identification cannot be found exhaustively. EXCEEDED_NU_MAX if an
/// identification cannot be found within a certain number of query picks. Otherwise, body stars b with the attached
/// labels of the inertial pair r.
Star::list Pyramid::identify () {
    *parameters.nu = 0;
    
    // This procedure will not work |input| < 4. Exit early with NO_CONFIDENT_IDENTITY.
    if (input.size() < 4) {
        return NO_CONFIDENT_IDENTITY;
    }
    
    // Otherwise, there exists |input| choose 3 possibilities. Looping specified in paper.
    for (unsigned int dj = 1; dj < input.size() - 1; dj++) {
        for (unsigned int dk = 1; dk < input.size() - dj - 1; dk++) {
            for (unsigned int di = 0; di < input.size() - dj - dk - 1; di++) {
                int i = di, j = di + dj, k = j + dk;
                Star::list candidates, matches;
                (*parameters.nu)++;
                
                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (*parameters.nu > parameters.nu_max) {
                    return EXCEEDED_NU_MAX;
                }
                
                // Given three stars in our catalog, find their catalog IDs in the catalog.
                star_trio r_quad = find_candidate_trio(star_trio {input[i], input[j], input[k]});
                if (std::equal(r_quad.begin(), r_quad.end(), NO_CANDIDATE_TRIANGLE_FOUND.begin())) {
                    continue;
                }
                
                return {Star::define_label(input[i], r_quad[0].get_label()),
                    Star::define_label(input[j], r_quad[1].get_label()),
                    Star::define_label(input[k], r_quad[2].get_label())};
            }
        }
    }
    
    return NO_CONFIDENT_IDENTITY;
}
