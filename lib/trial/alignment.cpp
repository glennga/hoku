/// @file alignment.cpp
/// @author Glenn Galvizo
///
/// Source file for the alignment trials. Allows us to characterize each identification method's of determining the
/// quaternion to take us from the R frame to the B frame.

// Give us access to everything in identification.
#define ENABLE_IDENTIFICATION_ACCESS

#include "trial/alignment.h"

/// Generate a set of stars near a random focus (inertial set), rotate it with some random rotation Q to get our
/// candidate set. The same stars share indices between the inertial and candidate set.
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param seed Random seed to use for rotation and focus star.
/// @param candidates Reference to the location to store the candidate list.
/// @param inertial Reference to the location to store the inertial set of stars.
/// @param q Reference to the location to store the rotation.
/// @param m_bar Minimum magnitude that all stars must be under.
void Alignment::present_candidates (Chomp &ch, std::random_device &seed, Star::list &candidates, Star::list &inertial,
                                    Rotation &q, const double m_bar) {
    candidates.clear();
    q = Rotation::chance(seed);
    
    candidates.reserve((unsigned int) WORKING_FOV * 4);
    for (const Star &s : ch.nearby_hip_stars(Star::chance(seed), WORKING_FOV / 2.0, (unsigned int) WORKING_FOV * 4)) {
        if (s.get_magnitude() < m_bar) {
            // We rotate the candidates (to get our body) and leave the inertial untouched.
            inertial.emplace_back(s);
            candidates.emplace_back(Rotation::rotate(s, q));
        }
    }
}

/// Generate gaussian noise for the first n candidates. Noise is distributed given shift_sigma.
///
/// @param seed Random seed to use for rotation and focus star.
/// @param candidates Reference to the location to store the candidate list.
/// @param shift_sigma Sigma for gaussian noise to apply. Defaults to 0.
/// @param shift_n Number of stars to shift. Shifts from the front of the list.
void Alignment::shift_candidates (std::random_device &seed, Star::list &candidates, const double shift_sigma,
                                  const int shift_n) {
    std::mt19937_64 mersenne_twister(seed());
    std::normal_distribution<double> dist(0, shift_sigma);
    
    if (shift_sigma != 0) {
        
        // Starting from the front of our list, apply noise. **Assumes that list generated is bigger than shift_n.
        for (int i = 0; i < shift_n; i++) {
            candidates[i] = Star(candidates[i][0] + dist(mersenne_twister), candidates[i][1] + dist(mersenne_twister),
                                 candidates[i][2] + dist(mersenne_twister), candidates[i].get_label(), true);
        }
    }
}

/// Record the results of Angle's attitude determination process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void Alignment::trial_angle (Chomp &ch, std::ofstream &log) {
    Rotation q, qe_optimal, qe_not_optimal;
    std::random_device seed;
    Star::list inertial;
    Angle::Parameters par;
    par.table_name = ANGLE_TABLE;
    
    // First run is clean, without shifts. Following are shift trials.
    Angle a(Benchmark(ch, seed, WORKING_FOV), Angle::Parameters());
    for (int ss_i = -1; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int ms_i = 0; ms_i < MS_ITER; ms_i++) {
                for (int i = 0; i < ALIGNMENT_SAMPLES; i++) {
                    present_candidates(ch, seed, a.input, inertial, q, (MB_MIN + mb_i * MB_STEP));
                    if (ss_i == -1) {
                        shift_candidates(seed, a.input, SS_MIN + ss_i * SS_STEP, 2);
                    }
                    
                    qe_optimal = a.trial_attitude_determine(a.input, {inertial[0], inertial[1]},
                                                            {a.input[0], a.input[1]});
                    qe_not_optimal = a.trial_attitude_determine(a.input, {inertial[1], inertial[0]},
                                                                {a.input[0], a.input[1]});
                    
                    // Log our results.
                    log << "Angle," << MS_MIN + ms_i * MS_STEP << "," << ((ss_i == -1) ? 0 : SS_MIN + ss_i * SS_STEP)
                        << "," << MB_MIN + mb_i * MB_STEP << "," << Rotation::rotational_difference(q, qe_optimal)
                        << "," << Rotation::rotational_difference(q, qe_not_optimal) << '\n';
                }
            }
        }
    }
}

/// Record the results of Plane's attitude determination process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void Alignment::trial_plane (Chomp &ch, std::ofstream &log) {
    Rotation q, qe_optimal, qe_not_optimal;
    std::random_device seed;
    Star::list inertial;
    Plane::Parameters par;
    par.table_name = PLANE_TABLE;
    
    // We load our quad-tree outside. Avoid rebuilding.
    std::shared_ptr<QuadNode> q_root = std::make_shared<QuadNode>(QuadNode::load_tree(Plane::Parameters().quadtree_w));
    Plane p(Benchmark(ch, seed, WORKING_FOV), par);
    
    // First run is clean, without shifts. Following are shift trials.
    for (int ss_i = -1; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int ms_i = 0; ms_i < MS_ITER; ms_i++) {
                for (int i = 0; i < ALIGNMENT_SAMPLES; i++) {
                    present_candidates(ch, seed, p.input, inertial, q, (MB_MIN + mb_i * MB_STEP));
                    if (ss_i == -1) {
                        shift_candidates(seed, p.input, SS_MIN + ss_i * SS_STEP, 3);
                    }
                    
                    qe_optimal = p.trial_attitude_determine(p.input, {inertial[0], inertial[1], inertial[2]},
                                                            {p.input[0], p.input[1], p.input[2]});
                    qe_not_optimal = p.trial_attitude_determine(p.input, {inertial[2], inertial[0], inertial[1]},
                                                                {p.input[0], p.input[1], p.input[2]});
                    
                    // Log our results.
                    log << "Plane," << MS_MIN + ms_i * MS_STEP << "," << ((ss_i == -1) ? 0 : SS_MIN + ss_i * SS_STEP)
                        << "," << MB_MIN + mb_i * MB_STEP << "," << abs(Rotation::rotational_difference(q, qe_optimal))
                        << "," << abs(Rotation::rotational_difference(q, qe_not_optimal)) << '\n';
                }
            }
        }
    }
}

/// Record the results of Sphere's attitude determination process. No error is introduced here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void Alignment::trial_sphere (Chomp &ch, std::ofstream &log) {
    Rotation q, qe_optimal, qe_not_optimal;
    std::random_device seed;
    Star::list inertial;
    Sphere::Parameters par;
    par.table_name = SPHERE_TABLE;
    
    // We load our quad-tree outside. Avoid rebuilding.
    std::shared_ptr<QuadNode> q_root = std::make_shared<QuadNode>(QuadNode::load_tree(Sphere::Parameters().quadtree_w));
    Sphere s(Benchmark(ch, seed, WORKING_FOV), par);
    
    // First run is clean, without shifts. Following are shift trials.
    for (int ss_i = -1; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int ms_i = 0; ms_i < MS_ITER; ms_i++) {
                for (int i = 0; i < ALIGNMENT_SAMPLES; i++) {
                    present_candidates(ch, seed, s.input, inertial, q, (MB_MIN + mb_i * MB_STEP));
                    if (ss_i == -1) {
                        shift_candidates(seed, s.input, SS_MIN + ss_i * SS_STEP, 3);
                    }
                    
                    qe_optimal = s.trial_attitude_determine(s.input, {inertial[0], inertial[1], inertial[2]},
                                                            {s.input[0], s.input[1], s.input[2]});
                    qe_not_optimal = s.trial_attitude_determine(s.input, {inertial[2], inertial[0], inertial[1]},
                                                                {s.input[0], s.input[1], s.input[2]});
                    
                    // Log our results.
                    log << "Sphere," << MS_MIN + ms_i * MS_STEP << "," << ((ss_i == -1) ? 0 : SS_MIN + ss_i * SS_STEP)
                        << "," << MB_MIN + mb_i * MB_STEP << "," << abs(Rotation::rotational_difference(q, qe_optimal))
                        << "," << abs(Rotation::rotational_difference(q, qe_not_optimal)) << '\n';
                }
            }
        }
    }
}
