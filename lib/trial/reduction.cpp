/// @file alignment.cpp
/// @author Glenn Galvizo
///
/// Source file for the reduction trials. Allows us to characterize each identification method's of candidate
/// reduction, starting from being given the image.

// Give us access to everything in identification.
#define ENABLE_IDENTIFICATION_ACCESS

#include "trial/reduction.h"

/// Generate the benchmark to be used for identification. Restricts which stars to include based on the given camera
/// sensitivity (m_bar).
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param seed Random seed to use for rotation and focus star.
/// @param body Reference to location of the body stars.
/// @param focus Reference to location of the focus star.
/// @param m_bar Minimum magnitude that all stars must be under.
void Reduction::present_benchmark (Chomp &ch, std::random_device &seed, Star::list &body, Star &focus,
                                   const double m_bar) {
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

/// Record the results of Angle's candidate reduction process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void Reduction::trial_angle (Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    Star::list body;
    Star focus;
    
    // These are the optimal parameters for the Angle method.
    Angle::Parameters par;
    ch.select_table(ANGLE_TABLE);
    par.query_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.z_max = 20000;
    
    // First run is clean, without shifts. Following are the shift trials.
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int i = 0; i < CROWN_SAMPLES; i++) {
                present_benchmark(ch, seed, body, focus, MB_MIN + mb_i * MB_STEP);
                Benchmark input(seed, body, focus, WORKING_FOV);
                
                // Append our error.
                input.shift_light((int) input.stars.size(), SS_MIN + SS_STEP * ss_i);
                par.match_sigma = SS_MIN + SS_STEP * ss_i;
                
                // Find the resulting pair.
                Angle::label_pair p = Angle::trial_reduction(ch, input.stars[0], input.stars[1], par.query_sigma);
                
                // Log our results.
                log << "Angle," << par.match_sigma << "," << par.query_sigma << "," << SS_MIN + SS_STEP * ss_i << ","
                    << MB_MIN + mb_i * MB_STEP << "," << is_correctly_identified({input.stars[0], input.stars[1]}, p)
                    << "\n";
            }
        }
    }
}

/// Record the results of Plane's identification process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void Reduction::trial_plane (Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    Star::list body;
    Star focus;
    
    // These are the optimal parameters for the Plane method.
    Plane::Parameters par;
    par.table_name = PLANE_TABLE;
    par.sigma_a = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.sigma_i = std::numeric_limits<double>::epsilon() * pow(3, 5);
    
    // First run is clean, without shifts. Following are the shift trials.
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int i = 0; i < CROWN_SAMPLES; i++) {
                present_benchmark(ch, seed, body, focus, MB_MIN + mb_i * MB_STEP);
                Benchmark input(seed, body, focus, WORKING_FOV);
                
                // Append our error.
                input.shift_light((int) input.stars.size(), SS_MIN + SS_STEP * ss_i);
                par.match_sigma = SS_MIN + SS_STEP * ss_i;
                
                // Find the resulting pair.
                Plane::label_trio p = Plane(input, par).trial_reduction();
                
                // Log our results.
                log << "Plane," << par.match_sigma << "," << par.sigma_a << "," << SS_MIN + SS_STEP * ss_i << ","
                    << MB_MIN + mb_i * MB_STEP << ","
                    << is_correctly_identified({input.stars[0], input.stars[1], input.stars[2]}, p) << "\n";
            }
        }
    }
}

/// Record the results of Sphere's identification process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void Reduction::trial_sphere (Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    Star::list body;
    Star focus;
    
    // These are the optimal parameters for the Sphere method.
    Sphere::Parameters par;
    par.table_name = SPHERE_TABLE;
    par.sigma_a = std::numeric_limits<double>::epsilon() * pow(3, 7);
    par.sigma_i = std::numeric_limits<double>::epsilon() * pow(3, 7);
    par.z_max = 20000;
    
    // First run is clean, without shifts. Following are the shift trials.
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int i = 0; i < CROWN_SAMPLES; i++) {
                present_benchmark(ch, seed, body, focus, MB_MIN + mb_i * MB_STEP);
                Benchmark input(seed, body, focus, WORKING_FOV);
                
                // Append our error.
                input.shift_light((int) input.stars.size(), SS_MIN + SS_STEP * ss_i);
                par.match_sigma = SS_MIN + SS_STEP * ss_i;
                
                // Find the resulting pair.
                Sphere::label_trio p = Sphere(input, par).trial_reduction();
                
                // Log our results.
                log << "Sphere," << par.match_sigma << "," << par.sigma_a << "," << SS_MIN + SS_STEP * ss_i << ","
                    << MB_MIN + mb_i * MB_STEP << ","
                    << is_correctly_identified({input.stars[0], input.stars[1], input.stars[2]}, p) << "\n";
            }
        }
    }
}

/// Record the results of Pyramid's identification process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void Reduction::trial_pyramid (Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    Star::list body;
    Star focus;
    
    // These are the optimal parameters for the Pyramid method.
    Pyramid::Parameters par;
    par.table_name = PYRAMID_TABLE;
    par.query_sigma = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.z_max = 20000;
    
    // First run is clean, without shifts. Following are the shift trials.
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int i = 0; i < CROWN_SAMPLES; i++) {
                present_benchmark(ch, seed, body, focus, MB_MIN + mb_i * MB_STEP);
                Benchmark input(seed, body, focus, WORKING_FOV);
                
                // Append our error.
                input.shift_light((int) input.stars.size(), SS_MIN + SS_STEP * ss_i);
                par.match_sigma = SS_MIN + SS_STEP * ss_i;
                
                // Find the resulting pair.
                Pyramid::label_quad p = Pyramid::trial_reduction(input, par);
                
                // Log our results.
                log << "Pyramid," << par.match_sigma << "," << par.query_sigma << "," << SS_MIN + SS_STEP * ss_i << ","
                    << MB_MIN + mb_i * MB_STEP << ","
                    << is_correctly_identified({input.stars[0], input.stars[1], input.stars[2], input.stars[3]}, p)
                    << "\n";
            }
        }
    }
}

/// Record the results of Coin's identification process.
///
/// @param ch Open Nibble connection using Chomp method.
/// @param log Open stream to log file.
void Reduction::trial_coin (Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    Star::list body;
    Star focus;
    
    // These are the optimal parameters for the Coin method.
    Coin::Parameters par;
    par.table_name = COIN_TABLE;
    par.sigma_a = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.sigma_i = std::numeric_limits<double>::epsilon() * pow(3, 5);
    par.z_max = 20000;
    
    // First run is clean, without shifts. Following are the shift trials.
    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int mb_i = 0; mb_i < MB_ITER; mb_i++) {
            for (int i = 0; i < CROWN_SAMPLES; i++) {
                present_benchmark(ch, seed, body, focus, MB_MIN + mb_i * MB_STEP);
                Benchmark input(seed, body, focus, WORKING_FOV);
                
                // Append our error.
                input.shift_light((int) input.stars.size(), SS_MIN + SS_STEP * ss_i);
                par.match_sigma = SS_MIN + SS_STEP * ss_i;
                
                // Find the resulting pair.
                Coin::label_quad p = Coin::trial_reduction(input, par);
                
                // Log our results.
                log << "Coin," << par.match_sigma << "," << par.sigma_a << "," << SS_MIN + SS_STEP * ss_i << ","
                    << MB_MIN + mb_i * MB_STEP << ","
                    << is_correctly_identified({input.stars[0], input.stars[1], input.stars[2], input.stars[3]}, p)
                    << "\n";
            }
        }
    }
}