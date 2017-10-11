/// @file query.cpp
/// @author Glenn Galvizo
///
/// Source file for the query trials. Allows us to characterize each identification method's hashing procedure.

// Give us access to everything in identification.
#define ENABLE_IDENTIFICATION_ACCESS

#include "trial/query.h"

/// Generate N random stars that fall within the specified field-of-view.
///
/// @param nb Open Nibble connection.
/// @param n Number of stars to generate.
/// @param seed Seed used to generate random stars with.
/// @return List of n stars who are within fov degrees from each other.
Star::list Query::generate_n_stars (Nibble &nb, const unsigned int n, std::random_device &seed) {
    std::mt19937_64 mersenne_twister(seed());
    std::uniform_int_distribution<int> dist(Nibble::BSC5_MIN_HR, Nibble::BSC5_MAX_HR);
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
    
    return s;
}

/// Wrapper method for clean and shift angle trials. Creates the random seed.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void Query::trial_angle (Nibble &nb, std::ofstream &log) {
    std::random_device seed;
    trial_angle_clean(nb, log, seed);
    trial_angle_shift(nb, log, seed);
}

/// Record the results of querying Nibble for nearby stars as the Angle method does. No shifting is performed here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param seed Seed used to generate random stars with.
void Query::trial_angle_clean (Nibble &nb, std::ofstream &log, std::random_device &seed) {
    for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
        for (int i = 0; i < QUERY_SAMPLES; i++) {
            Star::list s = generate_n_stars(nb, 2, seed);
            
            // Log our hr values, and get our result set.
            nb.select_table(ANGLE_TABLE);
            Angle::hr_pair b = {s[0].get_hr(), s[1].get_hr()};
            std::vector<Angle::hr_pair> r = Angle::trial_query(nb, s[0], s[1], QS_MIN * pow(QS_MULT, qs_i));
            
            // Check for star existence in results returned.
            bool s_existence = false;
            for (Angle::hr_pair &r_bar : r) {
                std::sort(r_bar.begin(), r_bar.end()), std::sort(b.begin(), b.end());
                if (r_bar[0] == b[0] && r_bar[1] == b[1]) {
                    s_existence = true;
                    break;
                }
            }
            
            // Log our results. Note that our shift sigma is 0.
            log << "Angle," << QS_MIN * pow(QS_MULT, qs_i) << ",0," << r.size() << "," << s_existence << '\n';
        }
    }
}

/// Record the results of querying Nibble for nearby stars as the Angle method does. Shifting is performed here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param seed Seed used to generate random stars with.
void Query::trial_angle_shift (Nibble &nb, std::ofstream &log, std::random_device &seed) {
    Benchmark beta(WORKING_FOV, Star::chance(), Rotation::chance());
    
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
            for (int i = 0; i < QUERY_SAMPLES; i++) {
                beta.stars = generate_n_stars(nb, 2, seed);
                beta.focus = beta.stars[0];
                
                // Shift stars by shift_sigma.
                beta.shift_light(2, SS_MIN + SS_STEP * ss_i), beta.error_models.clear();
                
                // Log our hr values, and get our result set.
                nb.select_table(ANGLE_TABLE);
                Angle::hr_pair b = {beta.stars[0].get_hr(), beta.stars[1].get_hr()};
                std::vector<Angle::hr_pair> r = Angle::trial_query(nb, beta.stars[0], beta.stars[1],
                                                                   QS_MIN * pow(QS_MULT, qs_i));
                
                // Check for star existence in results returned.
                bool s_existence = false;
                for (Angle::hr_pair &r_bar : r) {
                    std::sort(r_bar.begin(), r_bar.end()), std::sort(b.begin(), b.end());
                    if (r_bar[0] == b[0] && r_bar[1] == b[1]) {
                        s_existence = true;
                        break;
                    }
                }
                
                // Log our results.
                log << "Angle," << QS_MIN * pow(QS_MULT, qs_i) << "," << SS_MIN + SS_STEP * ss_i << "," << r.size()
                    << "," << s_existence << '\n';
            }
        }
    }
}

/// Wrapper method for clean and shift plane trials. Creates the random seed.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void Query::trial_plane (Nibble &nb, std::ofstream &log) {
    std::random_device seed;
    trial_plane_clean(nb, log, seed);
    trial_plane_shift(nb, log, seed);
}

/// Record the results of querying Nibble for nearby stars as the Plane method does. No shifting is performed here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param seed Seed used to generate random stars with.
void Query::trial_plane_clean (Nibble &nb, std::ofstream &log, std::random_device &seed) {
    std::shared_ptr<QuadNode> q_root = std::make_shared<QuadNode>(QuadNode::load_tree(Plane::Parameters().quadtree_w));
    Benchmark beta(WORKING_FOV, Star::chance(), Rotation::chance());
    Plane::Parameters p;
    p.table_name = PLANE_TABLE;
    
    for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
        for (int i = 0; i < QUERY_SAMPLES; i++) {
            Star::list s = generate_n_stars(nb, 3, seed);
            
            // Vary our area and moment sigma.
            p.sigma_a = QS_MIN * pow(QS_MULT, qs_i), p.sigma_i = QS_MIN * pow(QS_MULT, qs_i);
            
            // Log our hr values, and get our result set.
            Plane::hr_trio b = {(double) s[0].get_hr(), (double) s[1].get_hr(), (double) s[2].get_hr()};
            double a_i = Trio::planar_area(s[0], s[1], s[2]), i_i = Trio::planar_moment(s[0], s[1], s[2]);
            std::vector<Plane::hr_trio> r = Plane(beta, p, q_root).query_for_trio(a_i, i_i);
            
            // Check for star existence in results returned.
            bool s_existence = false;
            for (Plane::hr_trio &r_bar : r) {
                std::sort(r_bar.begin(), r_bar.end()), std::sort(b.begin(), b.end());
                if (r_bar[0] == b[0] && r_bar[1] == b[1]) {
                    s_existence = true;
                    break;
                }
            }
            
            // Log our results. Note that our shift sigma is 0.
            log << "Plane," << QS_MIN * pow(QS_MULT, qs_i) << ",0," << r.size() << "," << s_existence << '\n';
        }
    }
}

/// Record the results of querying Nibble for nearby stars as the Plane method does. Shifting is performed here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param seed Seed used to generate random stars with.
void Query::trial_plane_shift (Nibble &nb, std::ofstream &log, std::random_device &seed) {
    std::shared_ptr<QuadNode> q_root = std::make_shared<QuadNode>(QuadNode::load_tree(Plane::Parameters().quadtree_w));
    Benchmark beta(WORKING_FOV, Star::chance(), Rotation::chance());
    Plane::Parameters p;
    p.table_name = PLANE_TABLE;
    
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
            for (int i = 0; i < QUERY_SAMPLES; i++) {
                beta.stars = generate_n_stars(nb, 3, seed);
                beta.focus = beta.stars[0];
                
                // Vary our area and moment sigma. Shift stars by shift_sigma.
                p.sigma_a = QS_MIN * pow(QS_MULT, qs_i), p.sigma_i = QS_MIN * pow(QS_MULT, qs_i);
                beta.shift_light(3, SS_MIN + SS_STEP * ss_i), beta.error_models.clear();
                
                // Log our hr values, and get our result set.
                Plane::hr_trio b = {(double) beta.stars[0].get_hr(), (double) beta.stars[1].get_hr(),
                    (double) beta.stars[2].get_hr()};
                double a_i = Trio::planar_area(beta.stars[0], beta.stars[1], beta.stars[2]);
                double i_i = Trio::planar_moment(beta.stars[0], beta.stars[1], beta.stars[2]);
                std::vector<Plane::hr_trio> r = Plane(beta, p, q_root).query_for_trio(a_i, i_i);
                
                // Check for star existence in results returned.
                bool s_existence = false;
                for (Plane::hr_trio &r_bar : r) {
                    std::sort(r_bar.begin(), r_bar.end()), std::sort(b.begin(), b.end());
                    if (r_bar[0] == b[0] && r_bar[1] == b[1]) {
                        s_existence = true;
                        break;
                    }
                }
                
                // Log our results.
                log << "Plane," << QS_MIN * pow(QS_MULT, qs_i) << "," << SS_MIN + SS_STEP * ss_i << "," << r.size()
                    << "," << s_existence << '\n';
            }
        }
    }
}

/// Wrapper method for clean and shift sphere trials. Creates the random seed.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void Query::trial_sphere (Nibble &nb, std::ofstream &log) {
    std::random_device seed;
    trial_sphere_clean(nb, log, seed);
    trial_sphere_shift(nb, log, seed);
}

/// Record the results of querying Nibble for nearby stars as the Sphere method does. No shifting is performed here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param seed Seed used to generate random stars with.
void Query::trial_sphere_clean (Nibble &nb, std::ofstream &log, std::random_device &seed) {
    std::shared_ptr<QuadNode> q_root = std::make_shared<QuadNode>(QuadNode::load_tree(Sphere::Parameters().quadtree_w));
    Benchmark beta(WORKING_FOV, Star::chance(), Rotation::chance());
    Sphere::Parameters p;
    p.table_name = SPHERE_TABLE;
    
    for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
        for (int i = 0; i < QUERY_SAMPLES; i++) {
            Star::list s = generate_n_stars(nb, 3, seed);
            
            // Vary our area and moment sigma.
            p.sigma_a = QS_MIN * pow(QS_MULT, qs_i), p.sigma_i = QS_MIN * pow(QS_MULT, qs_i);
            
            // Log our hr values, and get our result set.
            Sphere::hr_trio b = {(double) s[0].get_hr(), (double) s[1].get_hr(), (double) s[2].get_hr()};
            double a_i = Trio::spherical_area(s[0], s[1], s[2]);
            double i_i = Trio::spherical_moment(s[0], s[1], s[2], Sphere::Parameters().moment_td_h);
            std::vector<Sphere::hr_trio> r = Sphere(beta, p, q_root).query_for_trio(a_i, i_i);
            
            // Check for star existence in results returned.
            bool s_existence = false;
            for (Sphere::hr_trio &r_bar : r) {
                std::sort(r_bar.begin(), r_bar.end()), std::sort(b.begin(), b.end());
                if (r_bar[0] == b[0] && r_bar[1] == b[1]) {
                    s_existence = true;
                    break;
                }
            }
            
            // Log our results. Note that our shift sigma is 0.
            log << "Sphere," << QS_MIN * pow(QS_MULT, qs_i) << ",0," << r.size() << "," << s_existence << '\n';
        }
    }
}

/// Record the results of querying Nibble for nearby stars as the Sphere method does. Shifting is performed here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param seed Seed used to generate random stars with.
void Query::trial_sphere_shift (Nibble &nb, std::ofstream &log, std::random_device &seed) {
    std::shared_ptr<QuadNode> q_root = std::make_shared<QuadNode>(QuadNode::load_tree(Sphere::Parameters().quadtree_w));
    Benchmark beta(WORKING_FOV, Star::chance(), Rotation::chance());
    Sphere::Parameters p;
    p.table_name = SPHERE_TABLE;
    
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
            for (int i = 0; i < QUERY_SAMPLES; i++) {
                beta.stars = generate_n_stars(nb, 3, seed);
                beta.focus = beta.stars[0];
                
                // Vary our area and moment sigma.  Shift stars by shift_sigma.
                p.sigma_a = QS_MIN * pow(QS_MULT, qs_i), p.sigma_i = QS_MIN * pow(QS_MULT, qs_i);
                beta.shift_light(3, SS_MIN + SS_STEP * ss_i);
    
                // Log our hr values, and get our result set.
                Sphere::hr_trio b = {(double) beta.stars[0].get_hr(), (double) beta.stars[1].get_hr(),
                    (double) beta.stars[2].get_hr()};
                double a_i = Trio::spherical_area(beta.stars[0], beta.stars[1], beta.stars[2]);
                double i_i = Trio::spherical_moment(beta.stars[0], beta.stars[1], beta.stars[2],
                                                    Sphere::Parameters().moment_td_h);
                std::vector<Plane::hr_trio> r = Sphere(beta, p, q_root).query_for_trio(a_i, i_i);
                
                // Check for star existence in results returned.
                bool s_existence = false;
                for (Sphere::hr_trio &r_bar : r) {
                    std::sort(r_bar.begin(), r_bar.end()), std::sort(b.begin(), b.end());
                    if (r_bar[0] == b[0] && r_bar[1] == b[1]) {
                        s_existence = true;
                        break;
                    }
                }
                
                // Log our results.
                log << "Sphere," << QS_MIN * pow(QS_MULT, qs_i) << "," << SS_MIN + SS_STEP * ss_i << "," << r.size()
                    << "," << s_existence << '\n';
            }
        }
    }
}