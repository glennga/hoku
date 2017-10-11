/// @file alignment.cpp
/// @author Glenn Galvizo
///
/// Source file for the alignment trials. Allows us to characterize each identification method's of determining the
/// quaternion to take us from the R frame to the B frame.

// Give us access to everything in identification.
#define ENABLE_IDENTIFICATION_ACCESS

#include "trial/alignment.h"


/// Reproduction of the BaseTriangle method check_assumption. Unlike BaseTriangle's method, this does not the largest
/// star list, but rather the rotation associated with it. This also accepts a trio of stars in lieu of input indices.
///
/// @param candidates All stars to check against the body star set.
/// @param r Inertial (frame R) trio of stars to check against the body trio.
/// @param b Body (frame B) trio of stars to check against the inertial trio.
/// @return The quaternion associated with the largest set of matching stars across the body and inertial in all
/// pairing configurations.
Star::list Alignment::check_assumptions (const Star::list &candidates, const Trio::stars &r, const Trio::stars &b) {
    BaseTriangle::index_trio current_order = {0, 1, 2};
    std::array<Trio::stars, 6> r_assumption_list;
    std::array<Star::list, 6> matches;
    std::array<Rotation, 6> q;
    
    // Generate unique permutations using previously generated trio.
    for (int i = 1; i < 6; i++) {
        r_assumption_list = {r[current_order[0]], r[current_order[1]], r[current_order[2]]};
        
        // Given i, swap elements 2 and 3 if even, or 1 and 3 if odd.
        current_order = (i % 2) == 0 ? index_trio {0, 2, 1} : index_trio {2, 1, 0};
    }
    
    // Determine the rotation to take frame R to B. Only use r_1 and r_2 to get rotation.
    int current_assumption = 0;
    for (const Trio::stars &assumption : r_assumption_list) {
        q[current_assumption] = Rotation::rotation_across_frames({b[0], b[1]}, {assumption[0], assumption[1]});
        matches[current_assumption++] = rotate_stars(candidates, q);
    }
    
    // Return the larger of the six matches.
    return std::max_element(matches.begin(), matches.end(), [] (const Star::list &lhs, const Star::list &rhs) {
        return lhs.size() < rhs.size();
    })[0];
}

/// Generate a random benchmark, and return it's list of stars and the rotation Q_RB (R frame to B frame).
///
/// @param candidates Reference to the location to store the candidate list.
/// @param r Reference to the location to store the rotation.
void Alignment::present_candidates (Star::list &candidates, Rotation &r) {
    r = Rotation::chance();
    double fov;
    
    Benchmark b(QUERY_FOV, Star::chance(), r);
    b.present_image(candidates, fov);
}

/// Record the results of Angle's attitude determination process. No error is introduced here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void Alignment::angle_clean(Nibble &nb, std::ofstream &log) {
    for (int i = 0; i < ALIGNMENT_SAMPLES; i++) {
        Star::list candidates;
        Rotation r;
        present_candidates(candidates, r);
    
        Star::list Alignment::check_assumptions (const Star::list &candidates, const Trio::stars &r, const Trio::stars &b
    }
}

/// Record the results of querying Nibble for nearby stars as the Angle method does. No shifting is performed here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void TrialQuery::angle_clean (Nibble &nb, std::ofstream &log) {
    for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
        for (int i = 0; i < QUERY_SAMPLES; i++) {
            Star::list s = generate_n_stars(nb, 2, QUERY_FOV);
            
            // Log our hr values, and get our result set.
            Angle::hr_pair b = {s[0].get_hr(), s[1].get_hr()};
            std::vector<Angle::hr_pair> r = angle_query(nb, s[1], s[2], QS_MIN + qs_i * QS_STEP);
            
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
            log << "Angle," << QS_MIN + qs_i * QS_STEP << ",0," << r.size() << "," << s_existence << '\n';
        }
    }
}

/// Record the results of querying Nibble for nearby stars as the Angle method does. Shifting is performed here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void TrialQuery::angle_shift (Nibble &nb, std::ofstream &log) {
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
            for (int i = 0; i < QUERY_SAMPLES; i++) {
                Star::list s = generate_n_stars(nb, 2, QUERY_FOV);
                
                // Shift stars by shift_sigma.
                Benchmark beta(s, s[0], QUERY_FOV);
                beta.shift_light(2, SS_MIN + ss_i * SS_STEP);
                s[0] = beta.stars[0], s[1] = beta.stars[1];
                
                // Log our hr values, and get our result set.
                Angle::hr_pair b = {s[0].get_hr(), s[1].get_hr()};
                std::vector<Angle::hr_pair> r = angle_query(nb, s[0], s[1], QS_MIN + qs_i * QS_STEP);
                
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
                log << "Angle," << QS_MIN + qs_i * QS_STEP << ",0," << r.size() << "," << s_existence << '\n';
            }
        }
    }
}

/// Record the results of querying Nibble for nearby stars as the Plane method does. No shifting is performed here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void TrialQuery::plane_clean (Nibble &nb, std::ofstream &log) {
    const int BQT_MAX = DCPLA::BQT_MIN + DCPLA::BQT_STEP * DCPLA::BQT_ITER;
    std::shared_ptr<QuadNode> default_root = std::make_shared<QuadNode>(QuadNode::load_tree(BQT_MAX));
    
    for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
        for (int i = 0; i < QUERY_SAMPLES; i++) {
            Benchmark beta(QUERY_FOV, Star::chance(), Rotation::chance());
            Star::list s = generate_n_stars(nb, 3, QUERY_FOV);
            
            // Vary our area and moment sigma.
            Plane::Parameters p;
            p.sigma_a = QS_MIN + qs_i * QS_STEP, p.sigma_i = QS_MIN + qs_i * QS_STEP;
            
            // Log our hr values, and get our result set.
            Plane::hr_trio b = {(double) s[0].get_hr(), (double) s[1].get_hr(), (double) s[2].get_hr()};
            double a_i = Trio::planar_area(s[0], s[1], s[2]), i_i = Trio::planar_moment(s[0], s[1], s[2]);
            std::vector<Plane::hr_trio> r = Plane(beta, p, default_root).query_for_trio(a_i, i_i);
            
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
            log << "Plane," << QS_MIN + qs_i * QS_STEP << ",0," << r.size() << "," << s_existence << '\n';
        }
    }
}

/// Record the results of querying Nibble for nearby stars as the Plane method does. Shifting is performed here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void TrialQuery::plane_shift (Nibble &nb, std::ofstream &log) {
    const int BQT_MAX = DCPLA::BQT_MIN + DCPLA::BQT_STEP * DCPLA::BQT_ITER;
    std::shared_ptr<QuadNode> default_root = std::make_shared<QuadNode>(QuadNode::load_tree(BQT_MAX));
    
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
            for (int i = 0; i < QUERY_SAMPLES; i++) {
                Star::list s = generate_n_stars(nb, 3, QUERY_FOV);
                
                // Vary our area and moment sigma.
                Plane::Parameters p;
                p.sigma_a = QS_MIN + qs_i * QS_STEP, p.sigma_i = QS_MIN + qs_i * QS_STEP;
                
                // Shift stars by shift_sigma.
                Benchmark beta(s, s[0], QUERY_FOV);
                beta.shift_light(3, SS_MIN + ss_i * SS_STEP);
                s[0] = beta.stars[0], s[1] = beta.stars[1], s[2] = beta.stars[2];
                
                // Log our hr values, and get our result set.
                Plane::hr_trio b = {(double) s[0].get_hr(), (double) s[1].get_hr(), (double) s[2].get_hr()};
                double a_i = Trio::planar_area(s[0], s[1], s[2]), i_i = Trio::planar_moment(s[0], s[1], s[2]);
                std::vector<Plane::hr_trio> r = Plane(beta, p, default_root).query_for_trio(a_i, i_i);
                
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
                log << "Plane," << QS_MIN + qs_i * QS_STEP << ",0," << r.size() << "," << s_existence << '\n';
            }
        }
    }
}

/// Record the results of querying Nibble for nearby stars as the Sphere method does. No shifting is performed here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void TrialQuery::sphere_clean (Nibble &nb, std::ofstream &log) {
    const int BQT_MAX = DCSPH::BQT_MIN + DCSPH::BQT_STEP * DCSPH::BQT_ITER;
    std::shared_ptr<QuadNode> default_root = std::make_shared<QuadNode>(QuadNode::load_tree(BQT_MAX));
    
    for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
        for (int i = 0; i < QUERY_SAMPLES; i++) {
            Benchmark beta(QUERY_FOV, Star::chance(), Rotation::chance());
            Star::list s = generate_n_stars(nb, 3, QUERY_FOV);
            
            // Vary our area and moment sigma.
            Sphere::Parameters p;
            p.sigma_a = QS_MIN + qs_i * QS_STEP, p.sigma_i = QS_MIN + qs_i * QS_STEP;
            
            // Log our hr values, and get our result set.
            Sphere::hr_trio b = {(double) s[0].get_hr(), (double) s[1].get_hr(), (double) s[2].get_hr()};
            double a_i = Trio::spherical_area(s[0], s[1], s[2]);
            double i_i = Trio::spherical_moment(s[0], s[1], s[2], DCSPH::TD_H_FOR_TABLE);
            std::vector<Sphere::hr_trio> r = Sphere(beta, p, default_root).query_for_trio(a_i, i_i);
            
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
            log << "Sphere," << QS_MIN + qs_i * QS_STEP << ",0," << r.size() << "," << s_existence << '\n';
        }
    }
}

/// Record the results of querying Nibble for nearby stars as the Sphere method does. Shifting is performed here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void TrialQuery::sphere_shift (Nibble &nb, std::ofstream &log) {
    const int BQT_MAX = DCSPH::BQT_MIN + DCSPH::BQT_STEP * DCSPH::BQT_ITER;
    std::shared_ptr<QuadNode> default_root = std::make_shared<QuadNode>(QuadNode::load_tree(BQT_MAX));
    
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int qs_i = 0; qs_i < QS_ITER; qs_i++) {
            for (int i = 0; i < QUERY_SAMPLES; i++) {
                Star::list s = generate_n_stars(nb, 3, QUERY_FOV);
                
                // Vary our area and moment sigma.
                Sphere::Parameters p;
                p.sigma_a = QS_MIN + qs_i * QS_STEP, p.sigma_i = QS_MIN + qs_i * QS_STEP;
                
                // Shift stars by shift_sigma.
                Benchmark beta(s, s[0], QUERY_FOV);
                beta.shift_light(3, SS_MIN + ss_i * SS_STEP);
                s[0] = beta.stars[0], s[1] = beta.stars[1], s[2] = beta.stars[2];
                
                // Log our hr values, and get our result set.
                Sphere::hr_trio b = {(double) s[0].get_hr(), (double) s[1].get_hr(), (double) s[2].get_hr()};
                double a_i = Trio::spherical_area(s[0], s[1], s[2]), i_i = Trio::spherical_moment(s[0], s[1], s[2]);
                std::vector<Sphere::hr_trio> r = Sphere(beta, p, default_root).query_for_trio(a_i, i_i);
                
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
                log << "Sphere," << QS_MIN + qs_i * QS_STEP << ",0," << r.size() << "," << s_existence << '\n';
            }
        }
    }
}