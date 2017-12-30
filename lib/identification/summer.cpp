/// @file summer.cpp
/// @author Glenn Galvizo
///
/// Source file for Summer class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#include "identification/summer.h"

/// Default parameters for the summer identification method.
const Identification::Parameters Summer::DEFAULT_PARAMETERS = {DEFAULT_SIGMA_QUERY, DEFAULT_SQL_LIMIT,
    DEFAULT_SIGMA_OVERLAY, DEFAULT_GAMMA, DEFAULT_NU_MAX, DEFAULT_NU, "SUMMER_20"};

/// Returned when there exists no common stars between the label list trios.
const Star Summer::NO_COMMON_STAR_FOUND = Star::zero();

/// Returned when there exists no candidate quad to be found for the given four stars.
const Summer::star_quad Summer::NO_CANDIDATE_QUAD_FOUND = {Star::zero(), Star::zero(), Star::zero(), Star::zero()};

/// Used to indicate that there are no labels to restrict searching for, when looking for a common star.
const Identification::labels_list Summer::NO_COMMON_RESTRICTIONS = {};

/// Returned when there exists no trios with the given set of stars.
const Summer::label_list_trio Summer::NO_CANDIDATE_TRIOS_FOUND = {{-1, -1, -1}};

/// Constructor. Sets the benchmark data, fov, parameters, and current working table.
///
/// @param input Working Benchmark instance. We are **only&& copying the star set and the fov.
Summer::Summer (const Benchmark &input, const Parameters &p) : Identification() {
    input.present_image(this->input, this->fov);
    this->parameters = p;
    
    this->ch.select_table(parameters.table_name);
}

/// Generate the planar triangle table given the specified FOV and table name. This is a wrapper for the Planar
/// Triangle's generate_table method.

/// @param fov Field of view limit (degrees) that all pairs must be within.
/// @param table_name Name of the table to generate.
/// @return TABLE_ALREADY_EXISTS if the table already exists. Otherwise, 0 when finished.
int Summer::generate_table (const double fov, const std::string &table_name) {
    return Plane::generate_table(fov, table_name);
}

/// Find all triangles whose area and moments are within 3 * sigma (epsilon) of the given a and i.
///
/// @param a Planar area to search with.
/// @param i_t Planar polar moment to search with.
/// @return NO_CANDIDATE_TRIOS_FOUND if there exists no results from the query. Otherwise, list of star trios whose area
/// and moments are close to the given a and i.
Summer::label_list_trio Summer::query_for_trios (const double a, const double i) {
    double epsilon = 3.0 * this->parameters.sigma_query;
    label_list_trio area_moment_match = NO_CANDIDATE_TRIOS_FOUND;
    Nibble::tuples_d area_match;
    
    // First, search for trio of stars matching area condition.
    ch.select_table(this->parameters.table_name);
    area_match = ch.simple_bound_query("a", "label_a, label_b, label_c, i", a - epsilon, a + epsilon,
                                       this->parameters.sql_limit);
    
    // Next, search this trio for stars matching the moment condition.
    area_moment_match.reserve(area_match.size() / 4);
    for (Chomp::tuple_d t : area_match) {
        if (t[3] >= i - epsilon && t[3] < i + epsilon) {
            area_moment_match.push_back(
                label_trio {static_cast<int> (t[0]), static_cast<int> (t[1]), static_cast<int> (t[2])});
        }
    }
    
    // If results are found, remove the initialized value of NO_CANDIDATE_TRIOS_FOUND.
    if (area_moment_match.size() > 1) {
        area_moment_match.erase(area_moment_match.begin());
    }
    return area_moment_match;
}

/// Given three lists of trios (A, B, C), return the first common star we find that does not exist in our removed list.
///
/// @param u_a List of catalog ID trios for potential candidates of stars in A.
/// @param u_b List of catalog ID trios for potential candidates of stars in B.
/// @param u_c List of catalog ID trios for potential candidates of stars in C.
/// @param u_r List of catalog IDs that our result cannot be.
/// @return NO_COMMON_STAR_FOUND if no common star is found. Otherwise, the first common star found in all pair lists.
Star Summer::find_common (const label_list_trio &u_a, const label_list_trio &u_b, const label_list_trio &u_c,
                          const labels_list &u_r) {
    auto flatten_trios = [] (const label_list_trio &candidates, labels_list &out_list) -> void {
        out_list.reserve(candidates.size() * 3);
        for (const label_trio &candidate : candidates) {
            out_list.push_back(candidate[0]);
            out_list.push_back(candidate[1]);
            out_list.push_back(candidate[2]);
        }
        std::sort(out_list.begin(), out_list.end());
    };
    
    // Flatten our list of trios.
    labels_list a_list, b_list, c_list, u_ab, candidates;
    flatten_trios(u_a, a_list), flatten_trios(u_b, b_list), flatten_trios(u_c, c_list);
    
    // Find the intersection between lists A and B.
    std::set_intersection(a_list.begin(), a_list.end(), b_list.begin(), b_list.end(), std::back_inserter(u_ab));
    
    // Find the intersection between our previous intersection and C. Remove invalid stars from candidates.
    std::set_intersection(u_ab.begin(), u_ab.end(), c_list.begin(), c_list.end(), std::back_inserter(candidates));
    
    // We find the first star in our candidates that does not exist in our removed set.
    for (const int &c : candidates) {
        if (std::find(u_r.begin(), u_r.end(), c) == u_r.end()) {
            return ch.query_hip(c);
        }
    }
    
    // If no such star is found, we return NO_COMMON_STAR_FOUND.
    return NO_COMMON_STAR_FOUND;
}

/// Given a quad of indices from the input set, determine the matching catalog IDs that correspond to each star. We
/// return the first match that we find here- there may exist better solutions past our initial find. 
///
/// @param b_f Quad of stars in our body frame.
/// @return NO_CANDIDATE_QUAD_FOUND if no quad can be found. Otherwise, the catalog IDs of stars from the inertial
/// frame.
Summer::star_quad Summer::find_candidate_quad (const star_quad &b_f) {
    auto find_trios = [this, &b_f] (const int i_a, const int i_b, const int i_c) -> label_list_trio {
        return this->query_for_trios(Trio::planar_area(b_f[i_a], b_f[i_b], b_f[i_c]),
                                     Trio::planar_moment(b_f[i_a], b_f[i_b], b_f[i_c]));
    };
    label_list_trio ijk_trios = find_trios(0, 1, 2), eij_trios = find_trios(0, 1, 3);
    label_list_trio eik_trios = find_trios(0, 2, 3), ejk_trios = find_trios(1, 2, 3);
    
    // We find our candidate stars for E, I, J, and K.
    Star e_candidate = find_common(eij_trios, eik_trios, ejk_trios, NO_COMMON_RESTRICTIONS);
    Star i_candidate = find_common(ijk_trios, eij_trios, eik_trios, {e_candidate.get_label()});
    Star j_candidate = find_common(ijk_trios, eij_trios, ejk_trios, {e_candidate.get_label(), i_candidate.get_label()});
    Star k_candidate = find_common(ijk_trios, eik_trios, ejk_trios,
                                   {e_candidate.get_label(), i_candidate.get_label(), j_candidate.get_label()});
    star_quad candidates = {i_candidate, j_candidate, k_candidate, e_candidate};
    
    // Check if any of the results return a star that doesn't exist.
    if (std::any_of(candidates.begin(), candidates.end(), [] (const Star &s) -> bool {
        return s == NO_COMMON_STAR_FOUND;
    })) {
        return NO_CANDIDATE_QUAD_FOUND;
    }
    
    // Otherwise, we have unique solutions for all candidates. Return our candidates.
    return candidates;
}

/// Given our list of inertial candidates and a quad of body stars that map to the inertial frame, return all stars in
/// our input that can be mapped back to the catalog.
///
/// @param candidates List of stars from the inertial frame.
/// @param b Body stars from out input. These represent the body frame anchors.
/// @param r Inertial stars from the catalog. These represent the inertial frame anchors.
/// @return Set of matching stars found in candidates and the body sets.
Star::list Summer::match_remaining (const Star::list &candidates, const star_quad &b, const star_quad &r) {
    // Find rotation between the stars I and E. Use this to find the matches.
    return this->find_matches(candidates, Rotation::rotation_across_frames({b[0], b[3]}, {r[0], r[3]}));
}

/// Reproduction of the Summer method's database querying. Input image is not used. We require the following be
/// defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_query
///     - sql_limit
/// @endcode
///
/// @param s Stars to query with. This must be of length = 4.
/// @return A vector of likely matches found by the pyramid method.
std::vector<Identification::labels_list> Summer::experiment_query (const Star::list &s) {
    if (s.size() != 4) {
        throw "Input list does not have exactly four stars.";
    }
    
    double epsilon = 3.0 * this->parameters.sigma_query, a = Trio::planar_area(s[0], s[1], s[3]);
    std::vector<labels_list> h_bar = {};
    Nibble::tuples_d area_match;
    
    // First, search for trio of stars matching area condition.
    area_match = ch.simple_bound_query("a", "label_a, label_b, label_c, i", a - epsilon, a + epsilon,
                                       this->parameters.sql_limit);
    
    // Next, search this trio for stars matching the moment condition.
    double i = Trio::planar_moment(s[0], s[1], s[3]);
    h_bar.reserve(area_match.size() / 4);
    for (Chomp::tuple_d t : area_match) {
        if (t[3] >= i - epsilon && t[3] < i + epsilon) {
            h_bar.push_back(labels_list {static_cast<int> (t[0]), static_cast<int> (t[1]), static_cast<int>(t[2])});
        }
    }
    
    return h_bar;
}

/// Reproduction of the Summer method's alignment determination process for a single reference star and alignment
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
/// @param b Body (frame B) pair of stars that match the inertial pair. This must be of length = 4.
/// @return NO_CONFIDENT_ALIGNMENT if an alignment quad cannot be found. Otherwise, body stars b with the attached
/// labels of the inertial pair r.
Star::list Summer::experiment_first_alignment (const Star::list &candidates [[maybe_unused]],
                                               const Star::list &r [[maybe_unused]], const Star::list &b) {
    if (b.size() != 4) {
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
Identification::labels_list Summer::experiment_reduction () {
    Star::list c = experiment_first_alignment({}, {}, {input[0], input[1], input[2], input[3]});
    if (std::equal(c.begin(), c.end(), NO_CONFIDENT_ALIGNMENT.begin())) {
        return NO_CANDIDATES_FOUND;
    }
    return labels_list {c[0].get_label(), c[1].get_label(), c[2].get_label(), c[3].get_label()};
}

/// Reproduction of the Summer method's process from beginning to the orientation determination. Input image is used.
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
Star::list Summer::experiment_alignment () {
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
Star::list Summer::experiment_crown () {
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
