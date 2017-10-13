/// @file query.cpp
/// @author Glenn Galvizo
///
/// Source file for the query trials. Allows us to characterize each identification method's hashing procedure.

// Give us access to everything in identification.
#define ENABLE_IDENTIFICATION_ACCESS

#include "trial/query.h"

/// Generate N random stars that fall within the specified field-of-view. Rotate this result by some random quaternion.
///
/// @param nb Open Nibble connection.
/// @param n Number of stars to generate.
/// @param seed Seed used to generate random stars with.
/// @return List of n stars who are within fov degrees from each other.
Star::list Query::generate_n_stars (Nibble &nb, const unsigned int n, std::random_device &seed) {
    std::mt19937_64 mersenne_twister(seed());
    std::uniform_int_distribution<int> dist(Nibble::BSC5_MIN_ID, Nibble::BSC5_MAX_ID);
    Star::list s;
    s.reserve(n);
    
    // Generate two distinct random stars within a given field-of-view.
    do {
        s.clear();
        for (unsigned int i = 0; i < n; i++) {
            Star s_i = nb.query_bsc5(dist(mersenne_twister));
            
            // Account for the gaps in our table (stars not visible in catalog, or are not objects).
            while (s_i == Star::zero()) {
                s_i = nb.query_bsc5(dist(mersenne_twister));
            }
            
            s.push_back(s_i);
        }
    }
    while (!Star::within_angle(s, WORKING_FOV) || s[0] == s[1]);
    
    // Rotate all stars by a random quaternion.
    Rotation q = Rotation::chance();
    for (Star &s_i : s) {
        s_i = Rotation::rotate(s_i, q);
    }
    
    return s;
}

/// Record the results of querying Nibble for nearby stars as the Angle method does.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void Query::trial_angle (Nibble &nb, std::ofstream &log) {
    Benchmark beta(WORKING_FOV, Star::chance(), Rotation::chance());
    std::random_device seed;
    
    for (int ss_i = -1; ss_i < SS_ITER; ss_i++) {
        for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
            for (int i = 0; i < QUERY_SAMPLES; i++) {
                beta.stars = generate_n_stars(nb, 2, seed);
                beta.focus = beta.stars[0];
                
                // If shift trials, shift stars by shift_sigma.
                if (ss_i != -1) {
                    beta.shift_light(2, SS_MIN + SS_STEP * ss_i), beta.error_models.clear();
                }
                
                // Log our label values, and get our result set.
                nb.select_table(ANGLE_TABLE);
                Angle::label_pair b = {beta.stars[0].get_label(), beta.stars[1].get_label()};
                std::vector<Angle::label_pair> r = Angle::trial_query(nb, beta.stars[0], beta.stars[1],
                                                                   QS_MIN * pow(QS_MULT, qs_i));
                
                // Log our results.
                log << "Angle," << QS_MIN * pow(QS_MULT, qs_i) << "," << ((ss_i == -1) ? 0 : SS_MIN + SS_STEP * ss_i)
                    << "," << r.size() << "," << set_existence(r, b) << '\n';
            }
        }
    }
}

/// Record the results of querying Nibble for nearby stars as the Plane method does.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void Query::trial_plane (Nibble &nb, std::ofstream &log) {
    Benchmark beta(WORKING_FOV, Star::chance(), Rotation::chance());
    std::random_device seed;
    Plane::Parameters p;
    p.table_name = PLANE_TABLE;
    
    // We load our quad-tree outside. Avoid rebuilding.
    std::shared_ptr<QuadNode> q_root = std::make_shared<QuadNode>(QuadNode::load_tree(Plane::Parameters().quadtree_w));
    for (int ss_i = -1; ss_i < SS_ITER; ss_i++) {
        for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
            for (int i = 0; i < QUERY_SAMPLES; i++) {
                beta.stars = generate_n_stars(nb, 2, seed);
                beta.focus = beta.stars[0];
                
                // Vary our area and moment sigma. If shift trials, shift stars by shift_sigma.
                p.sigma_a = QS_MIN * pow(QS_MULT, qs_i), p.sigma_i = QS_MIN * pow(QS_MULT, qs_i);
                if (ss_i != -1) {
                    beta.shift_light(3, SS_MIN + SS_STEP * ss_i), beta.error_models.clear();
                }
                
                // Log our label values, and get our result set.
                Plane::label_trio b = {(double) beta.stars[0].get_label(), (double) beta.stars[1].get_label(),
                    (double) beta.stars[2].get_label()};
                double a_i = Trio::planar_area(beta.stars[0], beta.stars[1], beta.stars[2]);
                double i_i = Trio::planar_moment(beta.stars[0], beta.stars[1], beta.stars[2]);
                std::vector<Plane::label_trio> r = Plane(beta, p, q_root).query_for_trio(a_i, i_i);
                
                // Log our results.
                log << "Plane," << QS_MIN * pow(QS_MULT, qs_i) << "," << ((ss_i == -1) ? 0 : SS_MIN + SS_STEP * ss_i)
                    << "," << r.size() << "," << set_existence(r, b) << '\n';
            }
        }
    }
}

/// Record the results of querying Nibble for nearby stars as the Sphere method does.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void Query::trial_sphere (Nibble &nb, std::ofstream &log) {
    Benchmark beta(WORKING_FOV, Star::chance(), Rotation::chance());
    std::random_device seed;
    Sphere::Parameters p;
    p.table_name = SPHERE_TABLE;
    
    // We load our quad-tree outside. Avoid rebuilding.
    std::shared_ptr<QuadNode> q_root = std::make_shared<QuadNode>(QuadNode::load_tree(Sphere::Parameters().quadtree_w));
    for (int ss_i = -1; ss_i < SS_ITER; ss_i++) {
        for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
            for (int i = 0; i < QUERY_SAMPLES; i++) {
                beta.stars = generate_n_stars(nb, 2, seed);
                beta.focus = beta.stars[0];
                
                // Vary our area and moment sigma. If shift trials, shift stars by shift_sigma.
                p.sigma_a = QS_MIN * pow(QS_MULT, qs_i), p.sigma_i = QS_MIN * pow(QS_MULT, qs_i);
                if (ss_i != -1) {
                    beta.shift_light(3, SS_MIN + SS_STEP * ss_i), beta.error_models.clear();
                }
                
                // Log our label values, and get our result set.
                Sphere::label_trio b = {(double) beta.stars[0].get_label(), (double) beta.stars[1].get_label(),
                    (double) beta.stars[2].get_label()};
                double a_i = Trio::spherical_area(beta.stars[0], beta.stars[1], beta.stars[2]);
                double i_i = Trio::spherical_moment(beta.stars[0], beta.stars[1], beta.stars[2],
                                                    Sphere::Parameters().moment_td_h);
                std::vector<Sphere::label_trio> r = Sphere(beta, p, q_root).query_for_trio(a_i, i_i);
                
                // Log our results.
                log << "Sphere," << QS_MIN * pow(QS_MULT, qs_i) << "," << ((ss_i == -1) ? 0 : SS_MIN + SS_STEP * ss_i)
                    << "," << r.size() << "," << set_existence(r, b) << '\n';
            }
        }
    }
}
