/// @file astrometry-net.cpp
/// @author Glenn Galvizo
///
/// Source file for AstrometryNet class, which matches a set of body vectors (stars) to their inertial counter-parts
/// in the database.

#include "astrometry-net.h"

/// Constructor. Sets the benchmark data and fov. Sets the parameters and working table. Constructs both kd-trees and
/// saves the roots.
///
/// @param input Working Benchmark instance. We are **only** copying the star set and the fov.
/// @param parameters Parameters to use for identification.
AstrometryNet::AstrometryNet (const Benchmark &input, const Parameters &parameters) {
    Nibble::tuple asterisms;
    unsigned int n;
    
    input.present_image(this->input, this->fov);
    this->parameters = parameters;
    
    // Load ASTRO_C table into RAM.
    ch.select_table(this->parameters.center_name);
    n = (unsigned) ch.search_table("MAX(rowid)", 1)[0];
    asterisms = ch.search_table("i, j, k", n * 3);
    
    // Load KD tree for nearby stars.
    star_root = std::make_shared<KdNode>(KdNode::load_tree(ch.all_bsc5_stars(), this->parameters.kd_tree_w));
    
    // Convert ASTRO_C table into stars.
    astro_stars.reserve(n / 3);
        Nibble::tuple astro_i = ch.table_results_at(asterisms, 3, i);
        astro_stars.push_back(Star(astro_i[0], astro_i[1], astro_i[2]));
    }
    
    // Load KD tree for nearby asterisms.
    astro_root = std::make_shared<KdNode>(KdNode::load_tree(astro_stars, this->parameters.kd_tree_w));
}

/// Helper method for the ASTRO_H table generation. Checks if all stars are within the given FOV, if an asterism can be
/// computed given our current permutation, and if we have made too many hashes from one of the stars. If all of
/// these conditions are met, then we increment the hash count "a_count" and insert our computed hash into Nibble.
///
/// @param nb Open Nibble connection.
/// @param a_count Vector holding the number of times a star was used in a hash.
/// @param a_limit The limit each star can be in a hash.
/// @param i The working indices of the BSC5 star list (our current quad).
/// @param fov All stars must be within fov degrees of each other.
/// @param w_n The projection width to use for the hash function.
                            const double fov, const int w_n) {
    Star::list s_l = {nb.query_bsc5(i[0]), nb.query_bsc5(i[1]), nb.query_bsc5(i[2]), nb.query_bsc5(i[3])};
    
    // First, determine if we have passed the hash count for any of the stars.

/// Match the stars found in the given benchmark to those in the Nibble database.
///
/// @param input The set of benchmark data to work with.
/// @param parameters Adjustments to the identification process.
/// @return Vector of body stars with their inertial BSC IDs that qualify as matches.
Star::list Astro::identify (const Benchmark &input, const Parameters &parameters) {
    Astro a(input, parameters);
    
    // This procedure will not work |A_input| < 4. Exit early with empty list.
    if (a.input.size() < 4) {
        return Star::list{};
    }
    
    // Otherwise, there exists |A_input| choose 4 possibilities (fancy for loop wrapping below).
    const int INPUT_SIZE = (int) a.input.size();
    for (int i = 0, j = 1; i < INPUT_SIZE - 3; j = (j < INPUT_SIZE - 3) ? j + 1 : i++ + 2) {
        for (int k = j + 1, m = k + 1; k < INPUT_SIZE - 1; m = (m < INPUT_SIZE - 1) ? m + 1 : k++ + 2) {
            hr_quad r_hr = a.query_for_asterism({i, j, k, m});
            unsigned long b_f = 1;
            // Propose an alignment (r -> b frame). If no asterism can be generated, we break early.
            Rotation a_p = a.propose_alignment({i, j, k, m}, r_hr);
            if (a_p == Rotation::identity()) {
                break;
            }
            // Find stars who align from the R frame to a B frame star.
            models bf_models = a.classify_matches(r_hr, a_p);
            // Find nearby asterisms. Compute the bayes factor based on these alignments.
            hr_list_quad nearby_quad = a.nearby_asterisms(r_hr);
            for (const hr_quad &na : nearby_quad) {
                b_f += a.compare_alignments(bf_models, na_models);
            }
            // Bayes factor condition is met. Return the proposed match set above.
            if (b_f > a.parameters.k_alignment_accept) {
                return bf_models[0];
            }
        }
    }
    
    return Star::list{};
}
