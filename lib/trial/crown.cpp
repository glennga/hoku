/// @file crown.cpp
/// @author Glenn Galvizo
///
/// Source file for the crown trials. Allows us to characterize each identification method end to end.

// Give us access to everything in identification.
#define ENABLE_IDENTIFICATION_ACCESS

#include "trial/crown.h"

/// Generate the benchmark to be used for identification. Restricts which stars to include based on the given camera
/// sensitivity (m_bar).
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param seed Random seed to use for rotation and focus star.
/// @param body Reference to location of the body stars.
/// @param focus Reference to location of the focus star.
/// @param m_bar Minimum magnitude that all stars must be under.
void Crown::present_benchmark (Chomp &ch, std::random_device &seed, Star::list &body, Star &focus, const double m_bar) {
    std::mt19937_64 mersenne_twister(seed());
    Rotation q = Rotation::chance(seed);

    // We require at-least five stars to exist here.
    do {
        focus = Star::chance(seed);
        body.clear(), body.reserve((unsigned int) WORKING_FOV * 4);

        for (const Star &s : ch.nearby_hip_stars(focus, WORKING_FOV / 2.0, (unsigned int) WORKING_FOV * 4)) {
            if (s.get_magnitude() < m_bar) {
                body.emplace_back(Rotation::rotate(s, q));
            }
        }
    }
    while (body.size() < 5);

    // Shuffle and rotate the focus.
    std::shuffle(body.begin(), body.end(), mersenne_twister);
    focus = Rotation::rotate(focus, q);
}

/// Record the results of Angle's identification process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void Crown::trial_angle (Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    unsigned int z = 0;
    Star::list result, body;
    Star focus;

    // These are the optimal parameters for the Angle method.
    Angle::Parameters par;
    par.table_name = ANGLE_TABLE;
    par.match_sigma = 10e-10;
    par.query_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.z_max = 1500;

    // First run is clean, without shifts. Following are the shift trials.
    for (int ss_i = -1; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int es_i = 0; es_i < ES_ITER; es_i++) {
                for (int i = 0; i < CROWN_SAMPLES; i++) {
                    present_benchmark(ch, seed, body, focus, MB_MIN + mb_i * MB_STEP);
                    Benchmark input(seed, body, focus, WORKING_FOV);

                    // Append our error.
                    if (ss_i != -1) {
                        input.shift_light((int) input.stars.size(), SS_MIN * pow(SS_MULT, ss_i));
                    }
                    double p = ES_MIN + ES_STEP * es_i, clean_size = input.stars.size();
                    input.add_extra_light((int) ((p / (1 - p)) * clean_size));

                    // Find our result set. Run the identification.
                    z = 0;
                    result = Angle::identify(input, par, z);

                    // Log our results. Note that our match and query sigma are fixed.
                    log << "Angle," << par.match_sigma << "," << par.query_sigma << "," <<
                        ((ss_i == -1) ? 0 : SS_MIN * pow(SS_MULT, ss_i)) << "," << MB_MIN + mb_i * MB_STEP << ","
                        << p << "," << z << "," << input.stars.size() << "," << result.size() << ","
                        << Benchmark::compare_stars(input, result) / clean_size << '\n';
                }
            }
        }
    }
}

/// Record the results of Plane's identification process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void Crown::trial_plane (Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    unsigned int z = 0;
    Star::list result, body;
    Star focus;

    // These are the optimal parameters for the Plane method.
    Plane::Parameters par;
    par.table_name = PLANE_TABLE;
    par.match_sigma = 10e-10;
    par.sigma_a = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.sigma_i = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.z_max = 1500;

    // First run is clean, without shifts. Following are the shift trials.
    for (int ss_i = -1; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int es_i = 0; es_i < ES_ITER; es_i++) {
                for (int i = 0; i < CROWN_SAMPLES; i++) {
                    present_benchmark(ch, seed, body, focus, MB_MIN + mb_i * MB_STEP);
                    Benchmark input(seed, body, focus, WORKING_FOV);

                    // Append our error.
                    if (ss_i != -1) {
                        input.shift_light((int) input.stars.size(), SS_MIN * pow(SS_MULT, ss_i));
                    }
                    double p = ES_MIN + ES_STEP * es_i, clean_size = input.stars.size();
                    input.add_extra_light((int) ((p / (1 - p)) * clean_size));

                    // Find our result set. Run the identification.
                    z = 0;
                    result = Plane::identify(input, par, z);

                    // Log our results. Note that our match and query sigma are fixed.
                    log << "Plane," << par.match_sigma << "," << par.sigma_a << "," <<
                        ((ss_i == -1) ? 0 : SS_MIN * pow(SS_MULT, ss_i)) << "," << MB_MIN + mb_i * MB_STEP << ","
                        << p << "," << z << "," << input.stars.size() << "," << result.size() << ","
                        << Benchmark::compare_stars(input, result) / clean_size << '\n';
                }
            }
        }
    }
}

/// Record the results of Sphere's identification process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void Crown::trial_sphere (Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    unsigned int z = 0;
    Star::list result, body;
    Star focus;

    // These are the optimal parameters for the Sphere method.
    Sphere::Parameters par;
    par.table_name = SPHERE_TABLE;
    par.match_sigma = 10e-10;
    par.sigma_a = std::numeric_limits<double>::epsilon() * pow(3, 7);
    par.sigma_i = std::numeric_limits<double>::epsilon() * pow(3, 7);
    par.z_max = 1500;

    // First run is clean, without shifts. Following are the shift trials.
    for (int ss_i = -1; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int es_i = 0; es_i < ES_ITER; es_i++) {
                for (int i = 0; i < CROWN_SAMPLES; i++) {
                    present_benchmark(ch, seed, body, focus, MB_MIN + mb_i * MB_STEP);
                    Benchmark input(seed, body, focus, WORKING_FOV);

                    // Append our error.
                    if (ss_i != -1) {
                        input.shift_light((int) input.stars.size(), SS_MIN * pow(SS_MULT, ss_i));
                    }
                    double p = ES_MIN + ES_STEP * es_i, clean_size = input.stars.size();
                    input.add_extra_light((int) ((p / (1 - p)) * clean_size));

                    // Find our result set. Run the identification.
                    z = 0;
                    result = Sphere::identify(input, par, z);

                    // Log our results. Note that our match and query sigma are fixed.
                    log << "Sphere," << par.match_sigma << "," << par.sigma_a << "," <<
                        ((ss_i == -1) ? 0 : SS_MIN * pow(SS_MULT, ss_i)) << "," << MB_MIN + mb_i * MB_STEP << ","
                        << p << "," << z << "," << input.stars.size() << "," << result.size() << ","
                        << Benchmark::compare_stars(input, result) / clean_size << '\n';
                }
            }
        }
    }
}

/// Record the results of Pyramid's identification process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void Crown::trial_pyramid (Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    unsigned int z = 0;
    Star::list result, body;
    Star focus;

    // These are the optimal parameters for the Sphere method.
    Pyramid::Parameters par;
    par.table_name = PYRAMID_TABLE;
    par.match_sigma = 10e-10;
    par.query_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.z_max = 1500;

    // First run is clean, without shifts. Following are the shift trials.
    for (int ss_i = -1; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int es_i = 0; es_i < ES_ITER; es_i++) {
                for (int i = 0; i < CROWN_SAMPLES; i++) {
                    present_benchmark(ch, seed, body, focus, MB_MIN + mb_i * MB_STEP);
                    Benchmark input(seed, body, focus, WORKING_FOV);

                    // Append our error.
                    if (ss_i != -1) {
                        input.shift_light((int) input.stars.size(), SS_MIN * pow(SS_MULT, ss_i));
                    }
                    double p = ES_MIN + ES_STEP * es_i, clean_size = input.stars.size();
                    input.add_extra_light((int) ((p / (1 - p)) * clean_size));

                    // Find our result set. Run the identification.
                    z = 0;
                    result = Pyramid::identify(input, par, z);

                    // Log our results. Note that our match and query sigma are fixed.
                    log << "Pyramid," << par.match_sigma << "," << par.query_sigma << "," <<
                        ((ss_i == -1) ? 0 : SS_MIN * pow(SS_MULT, ss_i)) << "," << MB_MIN + mb_i * MB_STEP << ","
                        << p << "," << z << "," << input.stars.size() << "," << result.size() << ","
                        << Benchmark::compare_stars(input, result) / clean_size << '\n';
                }
            }
        }
    }
}