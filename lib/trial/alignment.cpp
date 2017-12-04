/// @file alignment.cpp
/// @author Glenn Galvizo
///
/// Source file for the alignment trials. Allows us to characterize each identification method's of determining the
/// quaternion to take us from the R frame to the B frame.

// Give us access to everything in identification.
#define ENABLE_IDENTIFICATION_ACCESS

#include "trial/alignment.h"

/// Generate a set of stars near a random focus (inertial set), rotate it with some random rotation Q to get our
/// body set. The same stars share indices between the inertial and body set.
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param seed Random seed to use for rotation and focus star.
/// @param body Reference to the location to store the body list.
/// @param inertial Reference to the location to store the inertial set of stars.
/// @param q Reference to the location to store the rotation.
/// @param m_bar Minimum magnitude that all stars must be under.
void Alignment::present_stars (Chomp &ch, std::random_device &seed, Star::list &body, Star::list &inertial, Rotation &q,
                               const double m_bar) {
    std::mt19937_64 mersenne_twister(seed());
    q = Rotation::chance(seed);
    
    // We require at-least five stars to exist here.
    do {
        inertial.clear(), inertial.reserve((unsigned int) WORKING_FOV * 4);
        for (const Star &s : ch.nearby_hip_stars(Star::chance(seed), WORKING_FOV / 2.0,
                                                 (unsigned int) WORKING_FOV * 4)) {
            if (s.get_magnitude() < m_bar) {
                inertial.emplace_back(s);
            }
        }
    }
    while (inertial.size() < 5);
    
    // Shuffle our inertial, then rotate our inertial set to get our body set.
    std::shuffle(inertial.begin(), inertial.end(), mersenne_twister);
    body.clear(), body.reserve(inertial.size());
    for (const Star &s : inertial) {
        body.emplace_back(Rotation::rotate(s, q));
    }
}

/// Generate gaussian noise for the first n body stars. Noise is distributed given shift_sigma.
///
/// @param seed Random seed to use for rotation and focus star.
/// @param candidates Reference to the location to store the candidate list.
/// @param shift_sigma Sigma for gaussian noise to apply. Defaults to 0.
/// @param shift_n Number of stars to shift. Shifts from the front of the list.
void Alignment::shift_body (std::random_device &seed, Star::list &candidates, const double shift_sigma,
                            const int shift_n) {
    if (shift_sigma != 0) {
        // Starting from the front of our list, apply noise. **Assumes that list generated is bigger than shift_n.
        for (int i = 0; i < shift_n; i++) {
            candidates[i] = Rotation::shake(candidates[i], shift_sigma, seed);
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
    par.match_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);
    
    // First run is clean, without shifts. Following are shift trials.
    Angle a(Benchmark(ch, seed, WORKING_FOV), Angle::Parameters());
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int i = 0; i < ALIGNMENT_SAMPLES; i++) {
                present_stars(ch, seed, a.input, inertial, q, (MB_MIN + mb_i * MB_STEP));
                shift_body(seed, a.input, ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)), (signed) a.input.size());
                
                qe_optimal = a.trial_attitude_determine(inertial, {inertial[0], inertial[1]}, {a.input[0], a.input[1]});
                qe_not_optimal = a.trial_attitude_determine(inertial, {inertial[0], inertial[1]},
                                                            {a.input[1], a.input[0]});
                
                // Log our results.
                log << "Angle," << MS_MIN + ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << ","
                    << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << MB_MIN + mb_i * MB_STEP << ","
                    << Rotation::angle_between(q, qe_optimal) << "," << Rotation::angle_between(q, qe_not_optimal)
                    << "," << Rotation::rotation_difference(q, qe_optimal, inertial[2]).norm() << ","
                    << Rotation::rotation_difference(q, qe_not_optimal, inertial[2]).norm() << '\n';
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
    par.match_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);
    
    // First run is clean, without shifts. Following are shift trials.
    Plane p(Benchmark(ch, seed, WORKING_FOV), par);
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int i = 0; i < ALIGNMENT_SAMPLES; i++) {
                present_stars(ch, seed, p.input, inertial, q, (MB_MIN + mb_i * MB_STEP));
                shift_body(seed, p.input, ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)), (signed) p.input.size());
                
                qe_optimal = p.trial_attitude_determine(inertial, {inertial[0], inertial[1], inertial[2]},
                                                        {p.input[0], p.input[1], p.input[2]});
                qe_not_optimal = p.trial_attitude_determine(inertial, {inertial[2], inertial[0], inertial[1]},
                                                            {p.input[0], p.input[1], p.input[2]});
                
                // Log our results.
                log << "Plane," << MS_MIN + ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << ","
                    << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << MB_MIN + mb_i * MB_STEP << ","
                    << Rotation::angle_between(q, qe_optimal) << "," << Rotation::angle_between(q, qe_not_optimal)
                    << "," << Rotation::rotation_difference(q, qe_optimal, inertial[3]).norm() << ","
                    << Rotation::rotation_difference(q, qe_not_optimal, inertial[3]).norm() << '\n';
            }
        }
    }
}

/// Record the results of Sphere's attitude determination process.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void Alignment::trial_sphere (Chomp &ch, std::ofstream &log) {
    Rotation q, qe_optimal, qe_not_optimal;
    std::random_device seed;
    Star::list inertial;
    Sphere::Parameters par;
    par.table_name = SPHERE_TABLE;
    par.match_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);
    
    // First run is clean, without shifts. Following are shift trials.
    Sphere s(Benchmark(ch, seed, WORKING_FOV), par);
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int i = 0; i < ALIGNMENT_SAMPLES; i++) {
                present_stars(ch, seed, s.input, inertial, q, (MB_MIN + mb_i * MB_STEP));
                shift_body(seed, s.input, ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)), (signed) s.input.size());
                
                qe_optimal = s.trial_attitude_determine(inertial, {inertial[0], inertial[1], inertial[2]},
                                                        {s.input[0], s.input[1], s.input[2]});
                qe_not_optimal = s.trial_attitude_determine(inertial, {inertial[2], inertial[0], inertial[1]},
                                                            {s.input[0], s.input[1], s.input[2]});
                
                // Log our results.
                log << "Sphere," << MS_MIN + ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << ","
                    << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << MB_MIN + mb_i * MB_STEP << ","
                    << Rotation::angle_between(q, qe_optimal) << "," << Rotation::angle_between(q, qe_not_optimal)
                    << "," << Rotation::rotation_difference(q, qe_optimal, inertial[3]).norm() << ","
                    << Rotation::rotation_difference(q, qe_not_optimal, inertial[3]).norm() << '\n';
            }
        }
    }
}

/// Record the results of Pyramid's attitude determination process. We note that a correct star configuration is
/// assumed to be found prior to determining the attitude for this specific method.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void Alignment::trial_pyramid (Chomp &ch, std::ofstream &log) {
    Rotation q, qe_optimal, qe_not_optimal;
    std::random_device seed;
    Star::list inertial;
    Pyramid::Parameters par;
    par.table_name = PYRAMID_TABLE;
    par.match_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);
    
    // First run is clean, without shifts. Following are shift trials.
    Pyramid p(Benchmark(ch, seed, WORKING_FOV), par);
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int i = 0; i < ALIGNMENT_SAMPLES; i++) {
                present_stars(ch, seed, p.input, inertial, q, (MB_MIN + mb_i * MB_STEP));
                shift_body(seed, p.input, ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)), (signed) p.input.size());
                
                // A correct star configuration is assumed to be found prior to determining the attitude.
                qe_optimal = p.trial_attitude_determine({p.input[0], p.input[1], p.input[2], p.input[3]},
                                                        {inertial[0], inertial[1], inertial[2], inertial[3]});
                qe_not_optimal = qe_optimal;
                
                // Log our results.
                log << "Pyramid," << MS_MIN + ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << ","
                    << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << MB_MIN + mb_i * MB_STEP << ","
                    << Rotation::angle_between(q, qe_optimal) << "," << Rotation::angle_between(q, qe_not_optimal)
                    << "," << Rotation::rotation_difference(q, qe_optimal, inertial[4]).norm() << ","
                    << Rotation::rotation_difference(q, qe_not_optimal, inertial[4]).norm() << '\n';
            }
        }
    }
}

/// Record the results of Coin's attitude determination process. We note that a correct star configuration is
/// assumed to be found prior to determining the attitude for this specific method.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
void Alignment::trial_coin (Chomp &ch, std::ofstream &log) {
    Rotation q, qe_optimal, qe_not_optimal;
    std::random_device seed;
    Star::list inertial;
    Coin::Parameters par;
    par.table_name = PYRAMID_TABLE;
    par.match_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);
    
    // First run is clean, without shifts. Following are shift trials.
    Coin c(Benchmark(ch, seed, WORKING_FOV), par);
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int i = 0; i < ALIGNMENT_SAMPLES; i++) {
                present_stars(ch, seed, c.input, inertial, q, (MB_MIN + mb_i * MB_STEP));
                shift_body(seed, c.input, ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)), (signed) c.input.size());
                
                // A correct star configuration is assumed to be found prior to determining the attitude.
                qe_optimal = c.trial_attitude_determine({c.input[0], c.input[1], c.input[2], c.input[3]},
                                                        {inertial[0], inertial[1], inertial[2], inertial[3]});
                qe_not_optimal = qe_optimal;
                
                // Log our results.
                log << "Coin," << MS_MIN + ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << ","
                    << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << MB_MIN + mb_i * MB_STEP << ","
                    << Rotation::angle_between(q, qe_optimal) << "," << Rotation::angle_between(q, qe_not_optimal)
                    << "," << Rotation::rotation_difference(q, qe_optimal, inertial[4]).norm() << ","
                    << Rotation::rotation_difference(q, qe_not_optimal, inertial[4]).norm() << '\n';
            }
        }
    }
}