/// @file pyramid.cpp
/// @author Glenn Galvizo
///
/// Source file for Pyramid class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#include "identification/pyramid.h"

/// Constructor. Sets the benchmark data, fov, parameters, and current working table.
///
/// @param input Working Benchmark instance. We are **only** copying the star set and the fov.
Pyramid::Pyramid (const Benchmark &input, const Parameters &p) {
    input.present_image(this->input, this->fov);
    this->parameters = p;
    
    ch.select_table(this->parameters.table_name);
}

/// The Pyramid method uses the exact same table as the Angle method. Wrap Angle's 'generate_sep_table' method.
///
/// @param fov Field of view limit (degrees) that all pairs must be within.
/// @param table_name Name of the table to generate.
/// @return 0 when finished.
int Pyramid::generate_sep_table (const double fov, const std::string &table_name) {
    return Angle::generate_sep_table(fov, table_name);
}

/// Find all star pairs whose angle of separation is with 3 * query_sigma (epsilon) degrees of each other.
///
/// @param theta Separation angle (degrees) to search with.
/// @return List of star pairs that fall within epsilon degrees of theta.
Pyramid::label_list_pair Pyramid::query_for_pairs (const double theta) {
    // Noise is normally distributed. Angle within 3 sigma of theta.
    double epsilon = 3.0 * this->parameters.query_sigma;
    unsigned int limit = this->parameters.query_limit;
    Chomp::tuples_d results;
    label_list_pair candidates;
    
    // Query using theta with epsilon bounds.
    ch.select_table(parameters.table_name);
    results = ch.k_vector_query("theta", "label_a, label_b, theta", theta - epsilon, theta + epsilon, 3 * limit);
    
    // Append the results to our candidate list.
    candidates.reserve(results.size() / 2);
    for (const Nibble::tuple_d &result: results) {
        candidates.push_back(label_pair {(int) result[0], (int) result[1]});
    }
    
    return candidates;
}

/// Given three lists of pairs (E-I, E-J, E-K), return the first common star we find.
///
/// @param ei List of catalog ID pairs for potential candidates of stars E and I.
/// @param ej List of catalog ID pairs for potential candidates of stars E and J.
/// @param ek List of catalog ID pairs for potential candidates of stars E and K.
/// @return Zero star if no common star is found. Otherwise, the first common star found in all pair lists.
Star Pyramid::find_reference (const label_list_pair &ei, const label_list_pair &ej, const label_list_pair &ek) {
    auto flatten_pairs = [] (const label_list_pair &candidates, label_list &out_list) -> void {
        out_list.reserve(candidates.size() * 2);
        for (const label_pair &candidate : candidates) {
            out_list.push_back(candidate[0]);
            out_list.push_back(candidate[1]);
        }
        std::sort(out_list.begin(), out_list.end());
    };
    label_list ei_list, ej_list, ek_list;
    
    // Flatten our list of pairs.
    flatten_pairs(ei, ei_list), flatten_pairs(ej, ej_list), flatten_pairs(ek, ek_list);
    label_list eij_common(ei_list.size()), candidates(ei_list.size());
    
    // Find the intersection between lists EI and EJ.
    eij_common.reserve((ei_list.size() < ej_list.size()) ? ei_list.size() : ej_list.size());
    std::set_intersection(ei_list.begin(), ei_list.end(), ej_list.begin(), ej_list.end(), eij_common.begin());
    
    // Find the intersection between our previous intersection and EK.
    std::set_intersection(eij_common.begin(), eij_common.end(), ek_list.begin(), ek_list.end(), candidates.begin());
    
    return (candidates.empty()) ? Star::zero() : ch.query_hip((int) candidates[0]);
}

/// Given a quad of indices from the input set, determine the matching catalog IDs that correspond to each star. We
/// return the first match that we find here- there may exist better solutions past our initial find.
///
/// @param b_f Quad of indices for the input set that represent the stars in our body frame.
/// @return [-1][-1][-1][-1] if no quad can be found. Otherwise, the catalog IDs of stars from the inertial frame.
Pyramid::label_quad Pyramid::find_candidate_quad (const index_quad &b_f) {
    auto find_pairs = [this, &b_f] (const int triangle_index) -> label_list_pair {
        return this->query_for_pairs(Star::angle_between(this->input[b_f[triangle_index]], this->input[b_f[3]]));
    };
    label_list_pair ei_pairs = find_pairs(0), ej_pairs = find_pairs(1), ek_pairs = find_pairs(2);
    
    // Find a candidate for the star E (reference star). Break if no reference star exists.
    Star e_candidate = find_reference(ei_pairs, ej_pairs, ek_pairs);
    if (e_candidate == Star::zero()) {
        return {-1, -1, -1, -1};
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
                int i = choose_not_e(p_i), j = choose_not_e(p_j), k = choose_not_e(p_k);
                
                if (Star::within_angle({ch.query_hip(i), ch.query_hip(j), ch.query_hip(k)}, fov)) {
                    return {i, j, k, e_candidate.get_label()};
                }
            }
        }
    }
    
    // Otherwise, there exists no match. Return with catalog ID quad of [-1][-1][-1][-1].
    return {-1, -1, -1, -1};
}

/// Rotate every point the given rotation and check if the angle of separation between any two stars is within a
/// given limit sigma.
///
/// @param candidates All stars to check against the body star set.
/// @param q The rotation to apply to all stars.
/// @return Set of matching stars found in candidates and the body sets.
Star::list Pyramid::find_matches (const Star::list &candidates, const Rotation &q) {
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
Star::list Pyramid::match_remaining (const Star::list &candidates, const index_quad &b, const label_quad &r) {
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
Rotation Pyramid::trial_attitude_determine (const std::array<Star, 4> &b, const std::array<Star, 4> &r) {
    return Rotation::rotation_across_frames({b[0], b[3]}, {r[0], r[3]});
}

/// Match the stars found in the given benchmark to those in the Nibble database.
///
/// @param input The set of benchmark data to work with.
/// @param parameters Adjustments to the identification process.
/// @param z Reference to variable that will hold the input comparison count.
/// @return Empty list if an image match cannot be found in "time". Otherwise, a vector of body stars with their
/// inertial catalog IDs that qualify as matches.
Star::list Pyramid::identify (const Benchmark &input, const Parameters &parameters, unsigned int &z) {
    Pyramid p(input, parameters);
    z = 0;
    
    // This procedure will not work |P_input| < 4. Exit early with empty list.
    if (p.input.size() < 4) {
        return Star::list{};
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
                    return {};
                }
                
                // Return all stars from our input that match the candidates. Append the appropriate catalog IDs.
                matches = p.match_remaining(candidates, {(signed) i, j, k, e}, r_quad);
                if (matches.size() >= p.parameters.match_minimum) {
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
Star::list Pyramid::identify (const Benchmark &input, const Parameters &parameters) {
    unsigned int z;
    return Pyramid::identify(input, parameters, z);
}

/// Reproduction of the Pyramid method's Nibble querying. Unlike the one used in identification, this does not return
/// the most likely match.
///
/// **Need to select the proper table before calling this method.**
///
/// @param ch Open Nibble connection with Chomp.
/// @param s_1 Star one to query with.
/// @param s_2 Star two to query with.
/// @param query_sigma Theta must be within 3 * query_sigma to appear in results.
std::vector<Pyramid::label_pair> Pyramid::trial_query (Chomp &ch, const Star &s_1, const Star &s_2,
                                                       const double query_sigma) {
    double epsilon = 3.0 * query_sigma, theta = Star::angle_between(s_1, s_2);
    std::vector<Pyramid::label_pair> r_bar;
    
    // Query using theta with epsilon bounds.
    Nibble::tuples_d r = ch.k_vector_query("theta", "label_a, label_b", theta - epsilon, theta + epsilon, 500);
    
    // Sort tuple_d into list of catalog ID pairs.
    r_bar.reserve(r.size() / 2);
    for (const Nibble::tuple_d &r_t : r) {
        r_bar.emplace_back(Pyramid::label_pair {(int) r_t[0], (int) r_t[1]});
    }
    
    return r_bar;
}

/// Finds the single, most likely result for the first four stars in our current benchmark.
///
/// @param input The set of benchmark data to work with.
/// @param parameters Adjustments to the identification process.
/// @return [-1][-1][-1][-1] if there are no matches. Otherwise, the most likely result for the first four stars.
Pyramid::label_quad Pyramid::trial_reduction (const Benchmark &input, const Parameters &parameters) {
    Pyramid p(input, parameters);
    
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
Rotation Pyramid::trial_semi_crown (const Benchmark &input, const Parameters &parameters, unsigned int &z) {
    Pyramid p(input, parameters);
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