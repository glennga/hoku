/// @file semi-crown.cpp
/// @author Glenn Galvizo
///
/// Source file for the semi-crown trials. Allows us to characterize each identification method from beginning, to
/// the attitude determination step.

// Give us access to everything in identification.
#define ENABLE_IDENTIFICATION_ACCESS

#include "trial/semi-crown.h"

/// Generate the benchmark to be used for identification. Restricts which stars to include based on the given camera
/// sensitivity (m_bar).
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param seed Random seed to use for rotation and focus star.
/// @param body Reference to location of the body stars.
/// @param inertial Reference to location of the inertial stars.
/// @param focus Reference to location of the focus star.
/// @param m_bar Minimum magnitude that all stars must be under.
/// @param q Reference to where the actual rotation of the image will be placed.
void SemiCrown::present_benchmark (Chomp &ch, std::random_device &seed, Star::list &body, Star::list &inertial,
                                   Star &focus, const double m_bar, Rotation &q) {
    std::mt19937_64 mt1(seed());
    auto mt2 = mt1;

    std::vector<int> indices;
    q = Rotation::chance(seed);

    // We require at-least five stars to exist here.
    do {
        focus = Star::chance(seed);
        body.clear(), body.reserve((unsigned int) WORKING_FOV * 4);
        inertial.clear(); inertial.reserve((unsigned int) WORKING_FOV * 4);

        for (const Star &s : ch.nearby_hip_stars(focus, WORKING_FOV / 2.0, (unsigned int) WORKING_FOV * 4)) {
            if (s.get_magnitude() < m_bar) {
                inertial.emplace_back(s);
                body.emplace_back(Rotation::rotate(s, q));
            }
        }
    }
    while (body.size() < 5);

    // Shuffle and rotate the focus.
    std::shuffle(body.begin(), body.end(), mt1);
    std::shuffle(inertial.begin(), inertial.end(), mt2);
    focus = Rotation::rotate(focus, q);
}

/// Record the results of Angle's identification process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void SemiCrown::trial_angle (Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    unsigned int z;
    Rotation q, q_actual;
    Star::list body, inertial;
    Star focus;

    // These are the optimal parameters for the Angle method.
    Angle::Parameters par;
    par.table_name = ANGLE_TABLE;
    par.z_max = 20000;
    par.match_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.query_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);

    // First run is clean, without shifts. Following are the shift trials.
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int es_i = 0; es_i < ES_ITER; es_i++) {
                for (int i = 0; i < CROWN_SAMPLES; i++) {
                    present_benchmark(ch, seed, body, inertial, focus, MB_MIN + mb_i * MB_STEP, q_actual);
                    Benchmark input(seed, body, focus, WORKING_FOV);

                    // Append our error.
                    input.shift_light((int) input.stars.size(), ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)));
                    double p = ES_MIN + ES_STEP * es_i, clean_size = input.stars.size();
                    input.add_extra_light((int) ((p / (1 - p)) * clean_size));

                    // Find the resulting rotation.
                    z = 0;
                    q = Angle::trial_semi_crown(input, par, z);

                    // Log our results.
                    log << "Angle," << par.match_sigma << "," << par.query_sigma << ","
                        << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << MB_MIN + mb_i * MB_STEP << "," << p
                        << "," << z << "," << Rotation::rotation_difference(q_actual, q, inertial[0]).norm()
                        << "\n";
                }
            }
        }
    }
}

/// Record the results of Plane's identification process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void SemiCrown::trial_plane (Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    unsigned int z = 0;
    Rotation q, q_actual;
    Star::list body, inertial;
    Star focus;

    // These are the optimal parameters for the Plane method.
    Plane::Parameters par;
    par.table_name = PLANE_TABLE;
    par.z_max = 20000;
    par.match_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.sigma_a = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.sigma_i = std::numeric_limits<double>::epsilon() * pow(3, 5);

    // First run is clean, without shifts. Following are the shift trials.
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int es_i = 0; es_i < ES_ITER; es_i++) {
                for (int i = 0; i < CROWN_SAMPLES; i++) {
                    present_benchmark(ch, seed, body, inertial, focus, MB_MIN + mb_i * MB_STEP, q_actual);
                    Benchmark input(seed, body, focus, WORKING_FOV);

                    // Append our error.
                    input.shift_light((int) input.stars.size(), ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)));
                    double p = ES_MIN + ES_STEP * es_i, clean_size = input.stars.size();
                    input.add_extra_light((int) ((p / (1 - p)) * clean_size));

                    // Find our result set. Run the identification.
                    z = 0;
                    q = Plane(input, par).trial_semi_crown(z);

                    // Log our results.
                    log << "Plane," << par.match_sigma << "," << par.sigma_a << ","
                        << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << MB_MIN + mb_i * MB_STEP << "," << p
                        << "," << z << "," << Rotation::rotation_difference(q_actual, q, inertial[0]).norm()
                        << "\n";
                }
            }
        }
    }
}

/// Record the results of Sphere's identification process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void SemiCrown::trial_sphere (Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    unsigned int z = 0;
    Rotation q, q_actual;
    Star::list body, inertial;
    Star focus;

    // These are the optimal parameters for the Sphere method.
    Sphere::Parameters par;
    par.table_name = SPHERE_TABLE;
    par.z_max = 20000;
    par.match_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.sigma_a = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.sigma_i = std::numeric_limits<double>::epsilon() * pow(3, 5);

    // First run is clean, without shifts. Following are the shift trials.
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int es_i = 0; es_i < ES_ITER; es_i++) {
                for (int i = 0; i < CROWN_SAMPLES; i++) {
                    present_benchmark(ch, seed, body, inertial, focus, MB_MIN + mb_i * MB_STEP, q_actual);
                    Benchmark input(seed, body, focus, WORKING_FOV);

                    // Append our error.
                    input.shift_light((int) input.stars.size(), ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)));
                    double p = ES_MIN + ES_STEP * es_i, clean_size = input.stars.size();
                    input.add_extra_light((int) ((p / (1 - p)) * clean_size));

                    // Find our result set. Run the identification.
                    z = 0;
                    q = Sphere(input, par).trial_semi_crown(z);

                    // Log our results.
                    log << "Sphere," << par.match_sigma << "," << par.sigma_a << ","
                        << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << MB_MIN + mb_i * MB_STEP << "," << p
                        << "," << z << "," << Rotation::rotation_difference(q_actual, q, inertial[0]).norm()
                        << "\n";
                }
            }
        }
    }
}

/// Record the results of Pyramid's identification process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void SemiCrown::trial_pyramid (Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    unsigned int z = 0;
    Rotation q, q_actual;
    Star::list body, inertial;
    Star focus;

    // These are the optimal parameters for the Pyramid method.
    Pyramid::Parameters par;
    par.table_name = PYRAMID_TABLE;
    par.z_max = 20000;
    par.match_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.query_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);

    // First run is clean, without shifts. Following are the shift trials.
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int es_i = 0; es_i < ES_ITER; es_i++) {
                for (int i = 0; i < CROWN_SAMPLES; i++) {
                    present_benchmark(ch, seed, body, inertial, focus, MB_MIN + mb_i * MB_STEP, q_actual);
                    Benchmark input(seed, body, focus, WORKING_FOV);

                    // Append our error.
                    input.shift_light((int) input.stars.size(), ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)));
                    double p = ES_MIN + ES_STEP * es_i, clean_size = input.stars.size();
                    input.add_extra_light((int) ((p / (1 - p)) * clean_size));

                    // Find our result set. Run the identification.
                    z = 0;
                    q = Pyramid::trial_semi_crown(input, par, z);

                    // Log our results.
                    log << "Pyramid," << par.match_sigma << "," << par.query_sigma << ","
                        << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << MB_MIN + mb_i * MB_STEP << "," << p
                        << "," << z << "," << Rotation::rotation_difference(q_actual, q, inertial[0]).norm()
                        << "\n";
                }
            }
        }
    }
}

/// Record the results of Coin's identification process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void SemiCrown::trial_coin (Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    unsigned int z = 0;
    Rotation q, q_actual;
    Star::list body, inertial;
    Star focus;

    // These are the optimal parameters for the Coin method.
    Coin::Parameters par;
    par.table_name = COIN_TABLE;
    par.z_max = 20000;
    par.match_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.sigma_a = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.sigma_i = std::numeric_limits<double>::epsilon() * pow(3, 5);

    // First run is clean, without shifts. Following are the shift trials.
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int es_i = 0; es_i < ES_ITER; es_i++) {
                for (int i = 0; i < CROWN_SAMPLES; i++) {
                    present_benchmark(ch, seed, body, inertial, focus, MB_MIN + mb_i * MB_STEP, q_actual);
                    Benchmark input(seed, body, focus, WORKING_FOV);

                    // Append our error.
                    input.shift_light((int) input.stars.size(), ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)));
                    double p = ES_MIN + ES_STEP * es_i, clean_size = input.stars.size();
                    input.add_extra_light((int) ((p / (1 - p)) * clean_size));

                    // Find our result set. Run the identification.
                    z = 0;
                    q = Coin::trial_semi_crown(input, par, z);

                    // Log our results.
                    log << "Coin," << par.match_sigma << "," << par.sigma_a << ","
                        << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << MB_MIN + mb_i * MB_STEP << "," << p
                        << "," << z << "," << Rotation::rotation_difference(q_actual, q, inertial[0]).norm()
                        << "\n";
                }
            }
        }
    }
}