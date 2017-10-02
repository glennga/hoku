/// @file astrometry-net.cpp
/// @author Glenn Galvizo
///
/// Source file for AstrometryNet class, which matches a set of body vectors (stars) to their inertial counter-parts
/// in the database.

#include "identification/astrometry-net.h"

/// Constructor. Sets the benchmark data and fov. Sets the parameters and working table. Constructs both kd-trees and
/// saves the roots.
///
/// @param input Working Benchmark instance. We are **only** copying the star set and the fov.
/// @param parameters Parameters to use for identification.
/// @param star_root Working kd-tree root node for nearby stars. If none is specified, build the kd-tree here.
/// @param astro_root Working kd-tree root node for nearby asterisms. If none is specified, build the kd-tree here.
AstrometryNet::AstrometryNet (const Benchmark &input, const Parameters &parameters,
                              const std::shared_ptr<KdNode> &star_root, const std::shared_ptr<KdNode> &astro_root) {
    Nibble::tuple asterisms;
    unsigned int n;
    
    input.present_image(this->input, this->fov);
    this->parameters = parameters;
    
    // Load ASTRO_C table into RAM.
    ch.select_table(this->parameters.center_name);
    n = (unsigned) ch.search_table("MAX(rowid)", 1)[0];
    asterisms = ch.search_table("i, j, k", n * 3);
    
    // Load KD tree for nearby stars.
    if (star_root == nullptr) {
        this->star_root = std::make_shared<KdNode>(KdNode::load_tree(ch.all_bsc5_stars(), this->parameters.kd_tree_w));
    }
    else {
        this->star_root = star_root;
    }
    
    // Convert ASTRO_C table into stars.
    astro_stars.reserve(n / 3);
    for (unsigned int i = 0; i < n; i += 3) {
        Nibble::tuple astro_i = ch.table_results_at(asterisms, 3, i);
        astro_stars.emplace_back(Star(astro_i[0], astro_i[1], astro_i[2]));
    }
    
    // Load KD tree for nearby asterisms.
    if (astro_root == nullptr) {
        this->astro_root = std::make_shared<KdNode>(KdNode::load_tree(astro_stars, this->parameters.kd_tree_w));
    }
    else {
        this->astro_root = astro_root;
        
    }
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
/// @return -1 if any of these conditions fail. 0 otherwise.
int Astro::insert_astro_h (Nibble &nb, std::vector<int> &a_count, const double a_limit, const hr_quad &i,
                           const double fov) {
    Star::list s_l = {nb.query_bsc5(i[0]), nb.query_bsc5(i[1]), nb.query_bsc5(i[2]), nb.query_bsc5(i[3])};
    auto check_hash = [a_count, a_limit, i] (const int &i_hr) -> bool {
        return a_count[i[i_hr]] < a_limit;
    };
    
    // First, determine if we have passed the hash count for any of the stars.
    if (check_hash(0) && check_hash(1) && check_hash(2) && check_hash(3)) {
        Asterism::points_cd h_t = Asterism::hash({s_l[0], s_l[1], s_l[2], s_l[3]});
        
        // Check if the hash returned is valid, and if all stars are within FOV degrees.
        if (Star::within_angle(s_l, fov) && h_t[0] + h_t[1] + h_t[2] + h_t[3] != 0) {
            // If we are allowed, increment the asterism count and insert into Nibble.
            a_count[i[0]] += 1, a_count[i[1]] += 1, a_count[i[2]] += 1, a_count[i[3]] += 1;
            nb.insert_into_table("hr_0, hr_1, hr_2, hr_3, cx, cy, dx, dy", {(double) i[0], (double) i[1], (double) i[2],
                (double) i[3], h_t[0], h_t[1], h_t[2], h_t[3]});
            return 0;
        }
    }
    
    return -1;
}

/// Generate the asterism hash table given the specified FOV and table name. This finds the hash codes between all
/// distinct permutation of quads, and only stores them if they fall within the corresponding field-of-view.
///
/// **MUST RUN BEFORE "generate_center_table".**
///
/// @param fov Field of view limit (degrees) that all pairs must be within.
/// @param a_limit To restrict the number of hashes generated. Each star can only be used a_limit times in a hash.
/// @param hash_table Name of the table to generate.
/// @return 0 when finished.
int Astro::generate_hash_table (const double fov, const int a_limit, const std::string &hash_table) {
    Nibble nb;
    SQLite::Transaction initial_transaction(*nb.db);
    std::vector<int> a_count(Nibble::BSC5_MAX_HR + 1);
    
    // Insert the relation's attributes.
    nb.create_table(hash_table, "hr_0 INT, hr_1 INT, hr_2 INT, hr_3 INT, cx FLOAT, cy FLOAT, dx FLOAT, dy FLOAT");
    initial_transaction.commit();
    nb.select_table(hash_table);
    
    // All of our asterism counts start at zero.
    a_count.resize(Nibble::BSC5_MAX_HR);
    std::fill(a_count.begin(), a_count.end(), 0);
    
    // (i, j, k, m) are distinct, where no (i, j, k, m) = (j, k, m, i), (k, j, m, i), ...
    for (unsigned int i = 0; i < Nibble::BSC5_TABLE_LENGTH - 3; i++) {
        SQLite::Transaction transaction(*nb.db);
        std::cout << "\r" << "Current *A* Star: " << nb.query_bsc5(i).get_hr();
        for (unsigned int j = i + 1; j < Nibble::BSC5_TABLE_LENGTH - 2; j++) {
            for (unsigned int k = j + 1; k < Nibble::BSC5_TABLE_LENGTH - 1; k++) {
                for (unsigned int m = k + 1; m < Nibble::BSC5_TABLE_LENGTH; m++) {
                    insert_astro_h(nb, a_count, a_limit, {(signed) i, (signed) j, (signed) k, (signed) m}, fov);
                }
            }
        }
        
        // Commit every star I change.
        transaction.commit();
    }
    
    // Create an index for dimension 0 of C points.
    return nb.polish_table("cx");
}

/// Generate the asterism center table given the specified FOV and table name. This generates the centers based on
/// the ASTRO_H table, and stores them in a separate table.
///
/// **THIS MUST BE RUN AFTER "generate_hash_table".**
///
/// @param hash_table Name of the hash table generated..
/// @param center_table Name of the table to generate.
/// @return 0 when finished.
int Astro::generate_center_table (const std::string &hash_table, const std::string &center_table) {
    Nibble nb;
    SQLite::Transaction initial_transaction(*nb.db);
    
    nb.select_table(hash_table);
    unsigned n = (unsigned) nb.search_table("MAX(rowid)", 1)[0];
    
    // Find all possible HR values from the ASTRO_H table.
    Nibble::tuple hr = nb.search_table("hr_0, hr_1, hr_2, hr_3", n);
    
    // Insert the relation's attributes.
    nb.create_table(center_table, "hr_0 INT, hr_1 INT, hr_2 INT, hr_3 INT, i FLOAT, j FLOAT, k FLOAT");
    initial_transaction.commit();
    nb.select_table(center_table);
    
    // For every HR quad, we generate the center and store this.
    for (unsigned int i = 0; i < hr.size(); i += 4) {
        SQLite::Transaction transaction(*nb.db);
        Star center = Asterism::center({nb.query_bsc5(i), nb.query_bsc5(i + 1), nb.query_bsc5(i + 2),
                                           nb.query_bsc5(i + 3)});
        
        nb.insert_into_table("hr_0, hr_1, hr_2, hr_3, i, j, k", {hr[i], hr[i + 1], hr[i + 2], hr[i + 3], center[0],
            center[1], center[2]});
        transaction.commit();
    }
    
    // Create an index for center dimension 0.
    return nb.polish_table("i");
}

/// Given a set of indices from our input, compute the asterism.
///
/// @param b_i Selected indices from our input to represent the quad.
/// @return [-1][-1][-1][-1] if no match is found. The HR values of the input stars respectively, otherwise.
Astro::hr_quad Astro::query_for_asterism (const index_quad &b_i) {
    Asterism::stars s_q = {input[b_i[0]], input[b_i[1]], input[b_i[2]], input[b_i[3]]};
    double epsilon = 3.0 * this->parameters.query_sigma;
    
    // Determine hash, and determine the order of the given stars.
    Asterism::points_cd h = Asterism::hash(s_q);
    Asterism::stars s_abcd = Asterism::find_abcd(s_q);
    
    // If a hash cannot be generated, return [-1, -1, -1, -1].
    if (h[0] + h[1] + h[2] + h[3] == 0) {
        return {-1, -1, -1, -1};
    }
    
    // Search for matching C_x first.
    ch.select_table(this->parameters.hash_name);
    Chomp::tuple matches = ch.k_vector_query("cx", "hr_0, hr_1, hr_2, hr_3",
                                             h[0] - epsilon, h[0] + epsilon, this->parameters.query_expected);
    
    // Filter out all matches that don't match C_y, D_x, and D_y.
    for (unsigned int i = 0; i < matches.size(); i += 4) {
        auto within_epsilon = [epsilon, &matches] (const int &j, const double &v) -> bool {
            return v - epsilon < matches[j] && matches[j] < v + epsilon;
        };
        
        // We return the first match we find.
        if (within_epsilon(i + 1, h[1]) && within_epsilon(i + 2, h[2]) && within_epsilon(i + 3, h[3])) {
            Astro::hr_quad in_given_order = {0, 0, 0, 0};
            
            // Return the given list in the order of the indices given to us (fancy nested for loop below).
            for (int j = 0, k = 0; j < 4; k = (k < 3) ? k + 1 : 0 * j++) {
                in_given_order[j] = (s_abcd[j] == s_q[k]) ? (int) matches[i + j] : in_given_order[j];
            }
            return in_given_order;
        }
    }
    
    // If we don't find a match, return a quad of [-1, -1, -1, -1].
    return {-1, -1, -1, -1};
}

/// Given a set of indices from our input and another of set HR numbers, determine the rotation to take our inertial
/// stars to the body.
///
/// @param b_i Selected indices from our input to represent the quad.
/// @param r_hr HR values that represent the asterism.
/// @return Identity quaternion if no asterism is found. An inertial->body rotation otherwise.
Rotation Astro::propose_alignment (const index_quad &b_i, const hr_quad &r_hr) {
    auto query_for_hr = [this, &r_hr] (const int &hr) -> Star {
        return this->ch.query_bsc5(r_hr[hr]);
    };
    
    // Return identity if no asterism is found.
    if (r_hr[0] + r_hr[1] + r_hr[2] + r_hr[3] == -4) {
        return Rotation::identity();
    }
    
    // Otherwise, we query for the stars in the inertial frame.
    Asterism::stars r = {query_for_hr(0), query_for_hr(1), query_for_hr(2), query_for_hr(3)};
    
    // We use the first two stars in our inertial and body frames.
    return Rotation::rotation_across_frames({r[0], r[1]}, {input[b_i[0]], input[b_i[1]]});
}

/// Given a quad of HR values, and a proposed alignment, determine which nearby stars from the HR quad match the
/// input and which do not. **We only use the first star in the quad.**
///
/// @param r_hr HR values that represent the asterism.
/// @param q Rotation to take R frame stars to the B frame.
/// @return Model holding "matched" stars, and another "unmatched" stars.
Astro::models Astro::classify_matches (const hr_quad &r_hr, const Rotation &q) {
    models m;
    
    // Search for nearby stars to the first HR.
    Star::list nearby = (*star_root).nearby_stars(ch.query_bsc5(r_hr[0]), fov, parameters.nearby_expected, ch.all_bsc5_stars());
    
    // Note: If a star in 'nearby' is not found, no action is taken. 'm' depends on the input set.
    for (const Star &s: this->input) {
        for (unsigned int i = 0; i < nearby.size(); i++) {
            Star b_prime = Rotation::rotate(nearby[i], q);
            
            if (Star::angle_between(b_prime, s) < 3.0 * parameters.match_sigma) {
                // We insert the matched input star with the proper HR if found.
                m[0].emplace_back(Star(s[0], s[1], s[2], b_prime.get_hr()));
                
                // Remove the current star from the searching set. End the search for this star.
                nearby.erase(nearby.begin() + i);
            }
        }
        
        // There exists no matching star. Sort into the non-match.
        m[1].push_back(s);
    }
    
    return m;
}

/// Given an quad of stars (represented through HR numbers, R frame stars), find other nearby asterisms. Return these
/// HR values as a list.
///
/// @param r_hr HR values that represent the asterism.
/// @return List with r_hr if this is not a valid asterism. Otherwise, the HR values of the nearby asterisms.
Astro::hr_list_quad Astro::nearby_asterisms (const hr_quad &r_hr) {
    hr_list_quad nearby_quad;
    auto query_for_hr = [this, &r_hr] (const int &hr) -> Star {
        return this->ch.query_bsc5(r_hr[hr]);
    };
    
    // Do not attempt if there exists no asterism.
    if (r_hr[0] + r_hr[1] + r_hr[2] + r_hr[3] == -4) {
        return {{-1, -1, -1, -1}};
    }
    
    // Find the asterism center.
    Star center = Asterism::center({query_for_hr(0), query_for_hr(1), query_for_hr(2), query_for_hr(3)});
    
    // Query the tree for nearby asterism centers.
    Star::list nearby = (*astro_root).nearby_stars(center, fov, parameters.nearby_expected * 2, astro_stars);
    
    // Parse all of quad's HR values.
    nearby_quad.reserve(nearby.size());
    ch.select_table(this->parameters.center_name);
    for (const Star &n : nearby) {
        Chomp::tuple n_quad = ch.k_vector_query("i", "hr_0, hr_1, hr_2, hr_3",
                                                n[0] - std::numeric_limits<double>::epsilon(),
                                                n[0] + std::numeric_limits<double>::epsilon(), 4);
        
        nearby_quad.push_back(hr_quad {(int) n_quad[0], (int) n_quad[1], (int) n_quad[2], (int) n_quad[3]});
    }
    
    return nearby_quad;
}

/// Compute the addition to the bayes factor given proposed models and compared models. We compare every star in
/// proposed to the stars in compared. This ignores any stars that exist near the compared asterism that don't
/// actually exist near the proposed asterism.
///
/// @param proposed Proposed background and foreground models.
/// @param compared Compared background and foreground models.
/// @return The added bayes factor based on the two models.
unsigned int Astro::compare_alignments (const models &proposed, const models &compared) {
    unsigned int b_f = 0;
    
    // Loop through every star in matched.
    for (const Star &s_1 : proposed[0]) {
        for (const Star &s_2 : compared[0]) {
            if (s_1 == s_2) {
                // Match across models -> true positive.
                b_f += parameters.u_tp;
                break;
            }
        }
        // Match in proposed but not in compared -> false positive.
        b_f += parameters.u_fp;
    }
    
    // Loop through every star in non-matched.
    for (const Star &s_1 : proposed[1]) {
        for (const Star &s_2 : proposed[2]) {
            if (s_1 == s_2) {
                // Match across models -> true negative.
                b_f += parameters.u_tn;
                break;
            }
        }
        // Not-matched in proposed but not in compared -> false negative.
        b_f += parameters.u_fn;
    }
    
    return b_f;
}

/// Match the stars found in the given benchmark to those in the Nibble database.
///
/// @param input The set of benchmark data to work with.
/// @param parameters Adjustments to the identification process.
/// @param z Reference to variable that will hold the input comparison count.
/// @param star_root Working kd-tree root node for nearby stars.
/// @param astro_root Working kd-tree root node for nearby asterisms.
/// @return Vector of body stars with their inertial BSC IDs that qualify as matches.
Star::list Astro::identify (const Benchmark &input, const Parameters &parameters, unsigned int &z,
                            const std::shared_ptr<KdNode> &star_root, const std::shared_ptr<KdNode> &astro_root) {
    Astro a(input, parameters, star_root, astro_root);
    z = 0;
    
    // This procedure will not work |A_input| < 4. Exit early with empty list.
    if (a.input.size() < 4) {
        return Star::list{};
    }
    
    // Otherwise, there exists |A_input| choose 4 possibilities (fancy for loop wrapping below).
    const auto INPUT_SIZE = (int) a.input.size();
    for (int i = 0, j = 1; i < INPUT_SIZE - 3; j = (j < INPUT_SIZE - 3) ? j + 1 : i++ + 2) {
        for (int k = j + 1, m = k + 1; k < INPUT_SIZE - 1; m = (m < INPUT_SIZE - 1) ? m + 1 : k++ + 2) {
            hr_quad r_hr = a.query_for_asterism({i, j, k, m});
            unsigned long b_f = 1;
            z++;
            
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
                models na_models = a.classify_matches(na, a.propose_alignment({i, j, k, m}, na));
                b_f += a.compare_alignments(bf_models, na_models);
            }
            
            // Bayes factor condition is met. Return the proposed match set above.
            if (b_f > a.parameters.k_accept) {
                return bf_models[0];
            }
        }
    }
    
    return Star::list{};
}

/// Overloaded to not include the comparison count parameter. Match the stars found in the given benchmark to those in
/// the Nibble database.
///
/// @param input The set of benchmark data to work with.
/// @param parameters Adjustments to the identification process.
/// @param star_root Working kd-tree root node for nearby stars.
/// @param astro_root Working kd-tree root node for nearby asterisms.
/// @return Vector of body stars with their inertial BSC IDs that qualify as matches.
Star::list Astro::identify (const Benchmark &input, const Parameters &parameters,
                            const std::shared_ptr<KdNode> &star_root, const std::shared_ptr<KdNode> &astro_root) {
    unsigned int z;
    return Astro::identify(input, parameters, z, star_root, astro_root);
}