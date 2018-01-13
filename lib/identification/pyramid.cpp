/// @file pyramid.cpp
/// @author Glenn Galvizo
///
/// Source file for Pyramid class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#include "identification/pyramid.h"

/// Default parameters for the pyramid identification method.
const Identification::Parameters Pyramid::DEFAULT_PARAMETERS = {DEFAULT_SIGMA_QUERY, DEFAULT_SQL_LIMIT,
    DEFAULT_SIGMA_OVERLAY, DEFAULT_GAMMA, DEFAULT_NU_MAX, DEFAULT_NU, "PYRAMID_20"};

/// Returned when there exists no common stars between the label list pairs.
const Star Pyramid::NO_REFERENCE_FOUND = Star::zero();

/// Returned when there exists no candidate quad to be found for the given four stars.
const Pyramid::star_quad Pyramid::NO_CANDIDATE_QUAD_FOUND = {Star::zero(), Star::zero(), Star::zero(), Star::zero()};

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

/// Given three lists of pairs (E-I, E-J, E-K), return the first common star we find.
///
/// @param ei List of catalog ID pairs for potential candidates of stars E and I.
/// @param ej List of catalog ID pairs for potential candidates of stars E and J.
/// @param ek List of catalog ID pairs for potential candidates of stars E and K.
/// @return NO_REFERENCE_FOUND if no common star is found. Otherwise, the first common star found in all pair lists.
Star Pyramid::find_reference (const label_list_pair &ei, const label_list_pair &ej, const label_list_pair &ek) {
    auto flatten_pairs = [] (const label_list_pair &candidates, labels_list &out_list) -> void {
        out_list.reserve(candidates.size() * 2);
        for (const label_pair &candidate : candidates) {
            out_list.push_back(candidate[0]);
            out_list.push_back(candidate[1]);
        }
        std::sort(out_list.begin(), out_list.end());
    };
    
    // Flatten our list of pairs.
    labels_list ei_list, ej_list, ek_list, eij_common, candidates;
    flatten_pairs(ei, ei_list), flatten_pairs(ej, ej_list), flatten_pairs(ek, ek_list);
    
    // Find the intersection between lists EI and EJ.
    std::set_intersection(ei_list.begin(), ei_list.end(), ej_list.begin(), ej_list.end(),
                          std::back_inserter(eij_common));
    
    // Find the intersection between our previous intersection and EK.
    std::set_intersection(eij_common.begin(), eij_common.end(), ek_list.begin(), ek_list.end(),
                          std::back_inserter(candidates));
    
    return candidates.empty() ? NO_REFERENCE_FOUND : ch.query_hip(static_cast<int> (candidates[0]));
}

/// Given a quad of indices from the input set, determine the matching catalog IDs that correspond to each star. We
/// return the first match that we find here- there may exist better solutions past our initial find.
///
/// @param b_f Quad of stars in our body frame.
/// @return NO_CANDIDATE_QUAD_FOUND if no quad can be found. Otherwise, the catalog IDs of stars from the inertial
/// frame.
Pyramid::star_quad Pyramid::find_candidate_quad (const star_quad &b_f) {
    auto find_pairs = [this, &b_f] (const int triangle_index) -> label_list_pair {
        return this->query_for_pairs(Star::angle_between(b_f[triangle_index], b_f[3]));
    };
    label_list_pair ei_pairs = find_pairs(0), ej_pairs = find_pairs(1), ek_pairs = find_pairs(2);
    
    // Find a candidate for the star E (reference star). Break if no reference star exists.
    Star e_candidate = find_reference(ei_pairs, ej_pairs, ek_pairs);
    if (e_candidate == NO_REFERENCE_FOUND) {
        return NO_CANDIDATE_QUAD_FOUND;
    }
    auto choose_not_e = [&e_candidate] (const label_pair &pair) -> int {
        return (pair[0] == e_candidate.get_label()) ? pair[1] : pair[0];
    };
    
    // Remove all pairs that don't contain our reference star.
    auto e_nonexistence = [&e_candidate] (const label_pair &pair) -> bool {
        return std::find(pair.begin(), pair.end(), e_candidate.get_label()) == pair.end();
    };
    ei_pairs.erase(std::remove_if(ei_pairs.begin(), ei_pairs.end(), e_nonexistence), ei_pairs.end());
    ej_pairs.erase(std::remove_if(ej_pairs.begin(), ej_pairs.end(), e_nonexistence), ej_pairs.end());
    ek_pairs.erase(std::remove_if(ek_pairs.begin(), ek_pairs.end(), e_nonexistence), ek_pairs.end());
    
    // Return the first match we find that falls within the field-of-view.
    for (const label_pair &p_i : ei_pairs) {
        for (const label_pair &p_j : ej_pairs) {
            for (const label_pair &p_k : ek_pairs) {
                Star i = ch.query_hip(choose_not_e(p_i)), j = ch.query_hip(choose_not_e(p_j)), k = ch.query_hip(
                    choose_not_e(p_k));
                
                if (Star::within_angle({i, j, k}, fov)) {
                    return {i, j, k, e_candidate};
                }
            }
        }
    }
    
    // Otherwise, there exists no match. Return with catalog ID quad of NO_CANDIDATE_QUAD_FOUND.
    return NO_CANDIDATE_QUAD_FOUND;
}

/// Given our list of inertial candidates and a quad of body stars that map to the inertial frame, return all stars in
/// our input that can be mapped back to the catalog.
///
/// @param candidates List of stars from the inertial frame.
/// @param b Body stars from out input. These represent the body frame anchors.
/// @param r Inertial stars from the catalog. These represent the inertial frame anchors.
/// @return Set of matching stars found in candidates and the body sets.
Star::list Pyramid::match_remaining (const Star::list &candidates, const star_quad &b, const star_quad &r) {
    // Find rotation between the stars I and E. Use this to find the matches.
    return this->find_matches(candidates, Rotation::rotation_across_frames({b[0], b[3]}, {r[0], r[3]}));
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
std::vector<Identification::labels_list> Pyramid::experiment_query (const Star::list &s) {
    if (s.size() != QUERY_STAR_SET_SIZE) {
        throw "Input list does not have exactly two stars.";
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

/// Reproduction of the Pyramid method's alignment determination process for a single reference star and alignment
/// trio. Input image is used. We require the following be defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_query
///     - sql_limit
/// @endcode
///
/// @param candidates All stars found near the inertial pair. This **will not** be used here.
/// @param r Inertial (frame R) pair of stars that match the body pair. This **will not** be used here.
/// @param b Body (frame B) pair of stars that match the inertial pair. This must be of length =
/// FIRST_ALIGNMENT_STAR_SET_SIZE.
/// @return NO_CONFIDENT_ALIGNMENT if an alignment quad cannot be found. Otherwise, body stars b with the attached
/// labels of the inertial pair r.
Star::list Pyramid::experiment_first_alignment (const Star::list &candidates [[maybe_unused]],
                                                const Star::list &r [[maybe_unused]], const Star::list &b) {
    if (b.size() != FIRST_ALIGNMENT_STAR_SET_SIZE) {
        throw "Input list does not have exactly four stars.";
    }
    
    star_quad r_quad = find_candidate_quad({b[0], b[1], b[2], b[3]});
    if (std::equal(r_quad.begin(), r_quad.end(), NO_CANDIDATE_QUAD_FOUND.begin())) {
        return NO_CONFIDENT_ALIGNMENT;
    }
    
    auto attach_label = [&b, &r_quad] (const int i) -> Star {
        return Star::define_label(b[i], r_quad[i].get_label());
    };
    return {attach_label(0), attach_label(1), attach_label(2), attach_label(3)};
}

/// Finds the single, most likely result for the first four stars in our current benchmark. This trial is
/// basically the same as the first alignment trials. Input image is used. We require the following to be defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_query
///     - sql_limit
/// @endcode
///
/// @return NO_CANDIDATES_FOUND if a candidate quad cannot be found. Otherwise, a single match configuration found
/// by the angle method.
Identification::labels_list Pyramid::experiment_reduction () {
    Star::list c = experiment_first_alignment({}, {}, {input[0], input[1], input[2], input[3]});
    if (std::equal(c.begin(), c.end(), NO_CONFIDENT_ALIGNMENT.begin())) {
        return NO_CANDIDATES_FOUND;
    }
    return labels_list {c[0].get_label(), c[1].get_label(), c[2].get_label(), c[3].get_label()};
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
/// @return NO_CONFIDENT_ALIGNMENT if an alignment cannot be found exhaustively. EXCEEDED_NU_MAX if an alignment
/// cannot be found within a certain number of query picks. Otherwise, body stars b with the attached labels
/// of the inertial pair r.
Star::list Pyramid::experiment_alignment () {
    *parameters.nu = 0;
    
    // This procedure will not work |input| < 4. Exit early with NO_CONFIDENT_ALIGNMENT.
    if (input.size() < 4) {
        return NO_CONFIDENT_ALIGNMENT;
    }
    
    // Otherwise, there exists |input| choose 4 possibilities. Looping specified in paper. E chosen after K.
    for (unsigned int dj = 1; dj < input.size() - 1; dj++) {
        for (unsigned int dk = 1; dk < input.size() - dj - 1; dk++) {
            for (unsigned int di = 0; di < input.size() - dj - dk - 1; di++) {
                int i = di, j = di + dj, k = j + dk, e = k + 1;
                Star::list candidates, matches;
                (*parameters.nu)++;
                
                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (*parameters.nu > parameters.nu_max) {
                    return EXCEEDED_NU_MAX;
                }
                
                // Given four stars in our catalog, find their catalog IDs in the catalog.
                star_quad r_quad = find_candidate_quad(star_quad {input[i], input[j], input[k], input[e]});
                if (std::equal(r_quad.begin(), r_quad.end(), NO_CANDIDATE_QUAD_FOUND.begin())) {
                    continue;
                }
                
                return {Star::define_label(input[i], r_quad[0].get_label()),
                    Star::define_label(input[j], r_quad[1].get_label()),
                    Star::define_label(input[k], r_quad[2].get_label()),
                    Star::define_label(input[e], r_quad[3].get_label())};
            }
        }
    }
    
    return NO_CONFIDENT_ALIGNMENT;
}

/// Match the stars found in the given benchmark to those in the Nibble database. All parameters must be defined.
///
/// @return NO_CONFIDENT_MATCH_SET if an alignment cannot be found exhaustively. EXCEEDED_NU_MAX if an alignment
/// cannot be found within a certain number of query picks. Otherwise, a vector of body stars with their
/// inertial catalog IDs that qualify as matches.
Star::list Pyramid::experiment_crown () {
    *parameters.nu = 0;
    
    // This procedure will not work |input| < 4. Exit early with NO_CONFIDENT_MATCH_SET.
    if (input.size() < 4) {
        return NO_CONFIDENT_MATCH_SET;
    }
    
    // Otherwise, there exists |input| choose 4 possibilities. Looping specified in paper. E chosen after K.
    for (unsigned int dj = 1; dj < input.size() - 1; dj++) {
        for (unsigned int dk = 1; dk < input.size() - dj - 1; dk++) {
            for (unsigned int di = 0; di < input.size() - dj - dk - 1; di++) {
                int i = di, j = di + dj, k = j + dk, e = k + 1;
                Star::list candidates, matches;
                (*parameters.nu)++;
                
                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (*parameters.nu > parameters.nu_max) {
                    return EXCEEDED_NU_MAX;
                }
                
                // Given four stars in our catalog, find their catalog IDs in the catalog.
                star_quad b_quad = {input[i], input[j], input[k], input[e]};
                star_quad r_quad = find_candidate_quad(b_quad);
                if (std::equal(r_quad.begin(), r_quad.end(), NO_CANDIDATE_QUAD_FOUND.begin())) {
                    continue;
                }
                
                // Find candidate stars around the reference star.
                candidates = ch.nearby_hip_stars(r_quad[3], fov, static_cast<unsigned int> (3 * (input.size())));
                
                // Return all stars from our input that match the candidates. Append the appropriate catalog IDs.
                matches = match_remaining(candidates, b_quad, r_quad);
                
                // Definition of image match: |match| > gamma minimum OR |match| == |input|.
                if (matches.size() > ceil(input.size() * parameters.gamma)) {
                    return matches;
                }
            }
        }
    }
    
    return NO_CONFIDENT_MATCH_SET;
}
