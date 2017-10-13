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
/// @param Open Nibble connection.
/// @param candidates Reference to the location to store the candidate list.
/// @param inertial Reference to the location to store the inertial set of stars.
/// @param q Reference to the location to store the rotation.
/// @param shift_sigma Sigma for gaussian noise to apply. Defaults to 0.
/// @param shift_n Number of stars to shift. Shifts from the front of the list.
void Alignment::present_candidates (Nibble &nb, Star::list &candidates, Star::list &inertial, Rotation &q,
                                    const double shift_sigma, const int shift_n) {
    // Need to keep random device static to avoid starting with same seed.
    static std::random_device seed;
    static std::mt19937_64 mersenne_twister(seed());
    std::normal_distribution<double> dist(0, shift_sigma);
    
    candidates.clear();
    q = Rotation::chance();
    
    candidates.reserve((unsigned int) WORKING_FOV * 4);
    for (const Star &s : nb.nearby_stars(Star::chance(), WORKING_FOV / 2.0, (unsigned int) WORKING_FOV * 4)) {
        // We rotate the candidates (to get our body) and leave the inertial untouched.
        inertial.emplace_back(s);
        candidates.emplace_back(Rotation::rotate(s, q));
    }
    
    // If desired, we distribute noise amongst our candidates.
    if (shift_sigma != 0) {
        
        // Starting from the front of our list, apply noise. **Assumes that list generated is bigger than shift_n.
        for (int i = 0; i < shift_n; i++) {
            candidates[i] = Star(candidates[i][0] + dist(mersenne_twister), candidates[i][1] + dist(mersenne_twister),
                                 candidates[i][2] + dist(mersenne_twister), candidates[i].get_hr(), true);
        }
    }
}

/// Record the results of Angle's attitude determination process. No error is introduced here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void Alignment::trial_angle (Nibble &nb, std::ofstream &log) {
    Angle a(Benchmark(WORKING_FOV, Star::chance(), Rotation::chance()), Angle::Parameters());
    Star::list inertial;
    Rotation q, qe_optimal, qe_not_optimal;
    
    // First run is clean, without shifts. Following are shift trials.
    for (int ss_i = -1; ss_i < SS_ITER; ss_i++) {
        for (int ms_i = 0; ms_i < MS_ITER; ms_i++) {
            for (int i = 0; i < ALIGNMENT_SAMPLES; i++) {
                present_candidates(nb, a.input, inertial, q, (ss_i == -1) ? 0 : SS_MIN + ss_i * SS_STEP,
                                   (ss_i == -1) ? 0 : 2);
                
                qe_optimal = a.trial_attitude_determine(a.input, {inertial[0], inertial[1]}, {a.input[0], a.input[1]});
                qe_not_optimal = a.trial_attitude_determine(a.input, {inertial[1], inertial[0]},
                                                            {a.input[0], a.input[1]});
                
                // Log our results.
                log << "Angle," << MS_MIN + ms_i * MS_STEP << "," << ((ss_i == -1) ? 0 : SS_MIN + ss_i * SS_STEP) << ","
                    << abs(Rotation::rotational_difference(q, qe_optimal, a.input[2])) << ","
                    << abs(Rotation::rotational_difference(q, qe_not_optimal, a.input[2])) << '\n';
            }
        }
    }
}

/// Record the results of Plane's attitude determination process. No error is introduced here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void Alignment::trial_plane (Nibble &nb, std::ofstream &log) {
    Plane p(Benchmark(WORKING_FOV, Star::chance(), Rotation::chance()), Plane::Parameters());
    Rotation q, qe_optimal, qe_not_optimal;
    Star::list inertial;
    Plane::Parameters par;
    par.table_name = PLANE_TABLE;
    
    // We load our quad-tree outside. Avoid rebuilding.
    std::shared_ptr<QuadNode> q_root = std::make_shared<QuadNode>(QuadNode::load_tree(Plane::Parameters().quadtree_w));
    
    // First run is clean, without shifts. Following are shift trials.
    for (int ss_i = -1; ss_i < SS_ITER; ss_i++) {
        for (int ms_i = 0; ms_i < MS_ITER; ms_i++) {
            for (int i = 0; i < ALIGNMENT_SAMPLES; i++) {
                present_candidates(nb, p.input, inertial, q, (ss_i == -1) ? 0 : SS_MIN + ss_i * SS_STEP,
                                   (ss_i == -1) ? 0 : 3);
                
                qe_optimal = p.trial_attitude_determine(p.input, {inertial[0], inertial[1], inertial[2]},
                                                        {p.input[0], p.input[1], p.input[2]});
                qe_not_optimal = p.trial_attitude_determine(p.input, {inertial[2], inertial[0], inertial[1]},
                                                            {p.input[0], p.input[1], p.input[2]});
                
                // Log our results.
                log << "Plane," << MS_MIN + ms_i * MS_STEP << "," << ((ss_i == -1) ? 0 : SS_MIN + ss_i * SS_STEP) << ","
                    << abs(Rotation::rotational_difference(q, qe_optimal, p.input[3])) << ","
                    << abs(Rotation::rotational_difference(q, qe_not_optimal, p.input[3])) << '\n';
            }
        }
    }
}

/// Record the results of Sphere's attitude determination process. No error is introduced here.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void Alignment::trial_sphere (Nibble &nb, std::ofstream &log) {
    Sphere s(Benchmark(WORKING_FOV, Star::chance(), Rotation::chance()), Sphere::Parameters());
    Rotation q, qe_optimal, qe_not_optimal;
    Star::list inertial;
    Sphere::Parameters par;
    par.table_name = SPHERE_TABLE;
    
    // We load our quad-tree outside. Avoid rebuilding.
    std::shared_ptr<QuadNode> q_root = std::make_shared<QuadNode>(QuadNode::load_tree(Sphere::Parameters().quadtree_w));
    
    // First run is clean, without shifts. Following are shift trials.
    for (int ss_i = -1; ss_i < SS_ITER; ss_i++) {
        for (int ms_i = 0; ms_i < MS_ITER; ms_i++) {
            for (int i = 0; i < ALIGNMENT_SAMPLES; i++) {
                present_candidates(nb, s.input, inertial, q, (ss_i == -1) ? 0 : SS_MIN + ss_i * SS_STEP,
                                   (ss_i == -1) ? 0 : 3);
                
                qe_optimal = s.trial_attitude_determine(s.input, {inertial[0], inertial[1], inertial[2]},
                                                        {s.input[0], s.input[1], s.input[2]});
                qe_not_optimal = s.trial_attitude_determine(s.input, {inertial[2], inertial[0], inertial[1]},
                                                            {s.input[0], s.input[1], s.input[2]});
                
                // Log our results.
                log << "Sphere," << MS_MIN + ms_i * MS_STEP << "," << ((ss_i == -1) ? 0 : SS_MIN + ss_i * SS_STEP)
                    << "," << abs(Rotation::rotational_difference(q, qe_optimal, s.input[3])) << ","
                    << abs(Rotation::rotational_difference(q, qe_not_optimal, s.input[3])) << '\n';
            }
        }
    }
}
