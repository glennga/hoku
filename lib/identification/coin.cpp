/// @file coin.cpp
/// @author Glenn Galvizo
///
/// Source file for Coin class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#include "identification/coin.h"

/// Constructor. Sets the benchmark data, fov, parameters, and current working table.
///
/// @param input Working Benchmark instance. We are **only&& copying the star set and the fov.
Coin::Coin (const Benchmark &input, const Parameters &p) {
    input.present_image(this->input, this->fov);
    this->parameters = p;
    
    this->ch.select_table(parameters.table_name);
}

/// Generate the planar triangle table given the specified FOV and table name. This is a wrapper for the Planar
/// Triangle's generate_triangle_table method.

/// @param fov Field of view limit (degrees) that all pairs must be within.
/// @param table_name Name of the table to generate.
/// @return 0 when finished.
int Coin::generate_triangle_table (const double fov, const std::string &table_name) {
    return Plane::generate_triangle_table(fov, table_name);
}

/// Find all triangles whose area and moments are within 3 * sigma (epsilon) of the given a and i.
///
/// @param a Planar area to search with.
/// @param i_t Planar polar moment to search with.
/// @return List of star trios whose area and moments are close to the given a and i.
Coin::label_list_trio Coin::query_for_trios (const double a, const double i) {
    double epsilon_a = 3.0 * this->parameters.sigma_a;
    double epsilon_i = 3.0 * this->parameters.sigma_i;
    label_list_trio area_moment_match = {{-1, -1, -1}};
    Nibble::tuples_d area_match;
    
    // First, search for trio of stars matching area condition.
    ch.select_table(this->parameters.table_name);
    area_match = ch.k_vector_query("a", "label_a, label_b, label_c, i", a - epsilon_a, a + epsilon_a,
                                   this->parameters.query_expected);
    
    // Next, search this trio for stars matching the moment condition.
    area_moment_match.reserve(area_match.size() / 4);
    for (Chomp::tuple_d t : area_match) {
        if (t[3] >= i - epsilon_i && t[3] < i + epsilon_i) {
            area_moment_match.push_back(label_trio {(int) t[0], (int) t[1], (int) t[2]});
        }
    }
    
    // If results are found, remove the initialized value of [-1][-1][-1].
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
/// @return Zero star if no common star is found. Otherwise, the first common star found in all pair lists.
Star Coin::find_common (const label_list_trio &u_a, const label_list_trio &u_b, const label_list_trio &u_c,
                        const label_list &u_r) {
    auto flatten_trios = [] (const label_list_trio &candidates, label_list &out_list) -> void {
        out_list.reserve(candidates.size() * 2);
        for (const label_trio &candidate : candidates) {
            out_list.push_back(candidate[0]);
            out_list.push_back(candidate[1]);
            out_list.push_back(candidate[2]);
        }
        std::sort(out_list.begin(), out_list.end());
    };
    label_list a_list, b_list, c_list;
    
    // Flatten our list of trios.
    flatten_trios(u_a, a_list), flatten_trios(u_b, b_list), flatten_trios(u_c, c_list);
    label_list u_ab(a_list.size()), candidates(a_list.size());
    
    // Find the intersection between lists A and B.
    u_ab.reserve((a_list.size() < b_list.size()) ? a_list.size() : b_list.size());
    std::set_intersection(a_list.begin(), a_list.end(), b_list.begin(), b_list.end(), u_ab.begin());
    
    // Find the intersection between our previous intersection and C.
    std::set_intersection(u_ab.begin(), u_ab.end(), b_list.begin(), b_list.end(), candidates.begin());
    
    // We find the first star in our candidates that does not exist in our removed set.
    for (const int &c : candidates) {
        if (std::find(u_r.begin(), u_r.end(), c) == u_r.end()) {
            return ch.query_hip(c);
        }
    }
    
    // If no such star is found, we return a zero star.
    return Star::zero();
}

/// Given a quad of indices from the input set, determine the matching catalog IDs that correspond to each star. We
/// return the first match that we find here- there may exist better solutions past our initial find.
///
/// @param b_f Quad of indices for the input set that represent the stars in our body frame.
/// @return [-1][-1][-1][-1] if no quad can be found. Otherwise, the catalog IDs of stars from the inertial frame.
Coin::label_quad Coin::find_candidate_quad (const index_quad &b_f) {
    auto find_trios = [this, &b_f] (const int i_a, const int i_b, const int i_c) -> label_list_trio {
        return this->query_for_trios(
            Trio::planar_area(this->input[b_f[i_a]], this->input[b_f[i_b]], this->input[b_f[i_c]]),
            Trio::planar_moment(this->input[b_f[i_a]], this->input[b_f[i_b]], this->input[b_f[i_c]]));
    };
    label_list_trio ijk_trios = find_trios(0, 1, 2), eij_trios = find_trios(0, 1, 3);
    label_list_trio eik_trios = find_trios(0, 2, 3), ejk_trios = find_trios(1, 2, 3);
    
    // We find our candidate stars for E, I, J, and K.
    Star e_candidate = find_common(eij_trios, eik_trios, ejk_trios);
    Star i_candidate = find_common(ijk_trios, eij_trios, eik_trios, {e_candidate.get_label()});
    Star j_candidate = find_common(ijk_trios, eij_trios, ejk_trios, {e_candidate.get_label(), i_candidate.get_label()});
    Star k_candidate = find_common(ijk_trios, eik_trios, ejk_trios,
                                   {e_candidate.get_label(), i_candidate.get_label(), j_candidate.get_label()});
    Star::list candidates = {e_candidate, i_candidate, j_candidate, k_candidate};
    
    // Break if any of the results return a star that doesn't exist.
    if (std::find_if(candidates.begin(), candidates.end(), [] (const Star &s) -> bool {
        return s == Star::zero();
    }) != candidates.end()) {
        return {-1, -1, -1, -1};
    }
    
    // Otherwise, we have unique solutions for all candidates. Return our candidates.
    return {i_candidate.get_label(), j_candidate.get_label(), k_candidate.get_label(), e_candidate.get_label()};
}

/// Rotate every point the given rotation and check if the angle of separation between any two stars is within a
/// given limit sigma.
///
/// @param candidates All stars to check against the body star set.
/// @param q The rotation to apply to all stars.
/// @return Set of matching stars found in candidates and the body sets.
Star::list Coin::find_matches (const Star::list &candidates, const Rotation &q) {
    Star::list matches, non_matched = this->input;
    double epsilon = 3.0 * this->parameters.match_sigma;
    matches.reserve(this->input.size());
    
    for (const Star &candidate : candidates) {
        Star r_prime = Rotation::rotate(candidate, q);
        for (unsigned int i = 0; i < non_matched.size(); i++) {
            if (Star::angle_between(r_prime, non_matched[i]) < epsilon) {
                // Add this match to the list by noting the candidate star's catalog ID number.
                matches.emplace_back(
                    Star(non_matched[i][0], non_matched[i][1], non_matched[i][2], candidate.get_label()));
                
                // Remove the current star from the searching set. End the search for this star.
                non_matched.erase(non_matched.begin() + i);
                break;
            }
        }
    }
    
    return matches;
}

/// Given our list of inertial candidates and a quad of body stars that map to the inertial frame, return all stars in
/// our input that can be mapped back to the catalog.
///
/// @param candidates List of stars from the inertial frame.
/// @param b Quad of indices for stars in our input. Represent the body frame anchors.
/// @param r Quad of catalog IDs for stars in catalog. Represent the inertial frame anchors.
/// @return Set of matching stars found in candidates and the body sets.
Star::list Coin::match_remaining (const Star::list &candidates, const index_quad &b, const label_quad &r) {
    // Find rotation between the stars I and E. Use this to find the matches.
    return this->find_matches(candidates, Rotation::rotation_across_frames({input[b[0]], input[b[3]]},
                                                                           {ch.query_hip(r[0]), ch.query_hip(r[3])}));
}

/// Return the rotation between the reference star and the first triangle star in both the body and inertial frames.
/// Unlike the method used in identification, this does not return the the matches associated with the rotation.
///
/// @param r Inertial (frame R) quad of stars to check against the body quad.
/// @param b Body (frame B) quad of stars to check against the inertial quad.
/// @return Quaternion between body and inertial sets.
Rotation Coin::trial_attitude_determine (const std::array<Star, 4> &b, const std::array<Star, 4> &r) {
    return Rotation::rotation_across_frames({b[0], b[3]}, {r[0], r[3]});
}


/// Match the stars found in the given benchmark to those in the Nibble database.
///
/// @param input The set of benchmark data to work with.
/// @param parameters Adjustments to the identification process.
/// @param z Reference to variable that will hold the input comparison count.
/// @return Empty list if an image match cannot be found in "time". Otherwise, a vector of body stars with their
/// inertial catalog IDs that qualify as matches.
Star::list Coin::identify (const Benchmark &input, const Parameters &parameters, unsigned int &z) {
    Coin c(input, parameters);
    z = 0;
    
    // This procedure will not work |C_input| < 4. Exit early with empty list.
    if (c.input.size() < 4) {
        return Star::list{};
    }
    
    // Otherwise, there exists |C_input| choose 4 possibilities. Looping specified in paper. E chosen after K.
    for (unsigned int dj = 1; dj < c.input.size() - 1; dj++) {
        for (unsigned int dk = 1; dk < c.input.size() - dj - 1; dk++) {
            for (unsigned int i = 0; i < c.input.size() - dj - dk - 1; i++) {
                int j = i + dj, k = j + dk, e = k + 1;
                Star::list candidates, matches;
                z++;
                
                // Given four stars in our catalog, find their catalog IDs in the catalog.
                label_quad r_quad = c.find_candidate_quad({(signed) i, j, k, e});
                if (std::find(r_quad.begin(), r_quad.end(), 0) != r_quad.end()) {
                    break;
                }
                
                // Find candidate stars around the reference star.
                candidates = c.ch.nearby_hip_stars(c.ch.query_hip(r_quad[3]), c.fov,
                                                   3 * ((unsigned int) c.input.size()));
                
                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (z > c.parameters.z_max) {
                    return {};
                }
                
                // Return all stars from our input that match the candidates. Append the appropriate catalog IDs.
                matches = c.match_remaining(candidates, {(signed) i, j, k, e}, r_quad);
                if (matches.size() >= c.parameters.match_minimum) {
                    return matches;
                }
            }
        }
    }
    
    // There exists no matches. Return an empty list.
    return {};
}

/// Overloaded to not include the comparison count parameter. Match the stars found in the given benchmark to those in
/// the Nibble database.
///
/// @param input The set of benchmark data to work with.
/// @param parameters Adjustments to the identification process.
/// @return Vector of body stars with their inertial BSC IDs that qualify as matches.
Star::list Coin::identify (const Benchmark &input, const Parameters &parameters) {
    unsigned int z;
    return Coin::identify(input, parameters, z);
}

/// Finds the single, most likely result for the first four stars in our current benchmark.
///
/// @param input The set of benchmark data to work with.
/// @param parameters Adjustments to the identification process.
/// @return [-1][-1][-1][-1] if there are no matches. Otherwise, the most likely result for the first four stars.
Coin::label_quad Coin::trial_reduction (const Benchmark &input, const Parameters &parameters) {
    Coin p(input, parameters);
    
    label_quad r_quad = p.find_candidate_quad({0, 1, 2, 3});
    if (std::find(r_quad.begin(), r_quad.end(), 0) != r_quad.end()) {
        return {-1, -1, -1, -1};
    }
    else {
        return r_quad;
    }
}

/// Determine the most likely rotation of the given benchmark to the star catalog.
///
/// @param input The set of benchmark data to work with.
/// @param parameters Adjustments to the identification process.
/// @param z Reference to variable that will hold the input comparison count.
/// @return Identity quaternion if there exists no match. Otherwise, the resulting rotation after our attitude
/// determination step.
Rotation Coin::trial_semi_crown (const Benchmark &input, const Parameters &parameters, unsigned int &z) {
    Coin p(input, parameters);
    z = 0;
    
    // This procedure will not work |P_input| < 4. Exit early with empty list.
    if (p.input.size() < 4) {
        return Rotation::identity();
    }
    
    // Otherwise, there exists |P_input| choose 4 possibilities. Looping specified in paper. E chosen after K.
    for (unsigned int dj = 1; dj < p.input.size() - 1; dj++) {
        for (unsigned int dk = 1; dk < p.input.size() - dj - 1; dk++) {
            for (unsigned int i = 0; i < p.input.size() - dj - dk - 1; i++) {
                int j = i + dj, k = j + dk, e = k + 1;
                Star::list candidates, matches;
                z++;
                
                // Given four stars in our catalog, find their catalog IDs in the catalog.
                label_quad r_quad = p.find_candidate_quad({(signed) i, j, k, e});
                if (std::find(r_quad.begin(), r_quad.end(), 0) != r_quad.end()) {
                    break;
                }
                
                // Find candidate stars around the reference star.
                candidates = p.ch.nearby_hip_stars(p.ch.query_hip(r_quad[3]), p.fov,
                                                   3 * ((unsigned int) p.input.size()));
                
                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (z > p.parameters.z_max) {
                    return Rotation::identity();
                }
                
                // Find the rotation given the two pairs.
                return Rotation::rotation_across_frames({p.input[i], p.input[e]},
                                                        {p.ch.query_hip(r_quad[0]), p.ch.query_hip(r_quad[3])});
            }
        }
    }
    return Rotation::identity();
}