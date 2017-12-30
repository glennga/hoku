/// @file query.cpp
/// @author Glenn Galvizo
///
/// Source file for the query trials. Allows us to characterize each identification method's hashing procedure.

// Give us access to everything in identification.
#define ENABLE_IDENTIFICATION_ACCESS

#include <identification/summer.h>
#include "trial/query.h"

/// Generate N random stars that fall within the specified field-of-view. Rotate this result by some random quaternion.
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param n Number of stars to generate.
/// @param seed Seed used to generate random stars with.
/// @return List of n stars who are within fov degrees from each other.
Star::list Query::generate_n_stars(Chomp &ch, const unsigned int n, std::random_device &seed) {
    std::mt19937_64 mersenne_twister(seed());
    std::uniform_int_distribution<int> dist(0, Chomp::BRIGHT_TABLE_LENGTH);
    Star::list s, hip_bright = ch.bright_as_list();
    s.reserve(n);

    // Generate two distinct random stars within a given field-of-view.
    do {
        s.clear();
        for (unsigned int i = 0; i < n; i++) {
            Star s_i = hip_bright[dist(mersenne_twister)];

            // Account for the gaps in our table (stars not visible in catalog, or are not objects).
            while (s_i == Star::zero()) {
                s_i = hip_bright[dist(mersenne_twister)];
            }

            s.push_back(s_i);
        }
    } while (!Star::within_angle(s, WORKING_FOV) || s[0] == s[1]);

    // Rotate all stars by a random quaternion.
    Rotation q = Rotation::chance(seed);
    for (Star &s_i : s) {
        s_i = Rotation::rotate(s_i, q);
    }

    return s;
}

/// Record the results of querying Nibble for nearby stars as the Angle method does.
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param log Open stream to log file.
void Query::trial_angle(Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    Benchmark beta(ch, seed, WORKING_FOV);

    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int i = 0; i < QUERY_SAMPLES; i++) {
            beta.stars = generate_n_stars(ch, 2, seed);
            beta.focus = beta.stars[0];
            beta.shift_light(2, ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i))), beta.error_models.clear();

            // Log our label values, and get our result set.
            ch.select_table(ANGLE_TABLE);
            Angle::label_pair b = {beta.stars[0].get_label(), beta.stars[1].get_label()};
            std::vector<Angle::label_pair> r = Angle::trial_query(ch, beta.stars[0], beta.stars[1], QS_MIN);

            // Log our results.
            log << "Angle," << QS_MIN << "," << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << r.size() << ","
                << set_existence(r, b) << '\n';
        }
    }
}

/// Record the results of querying Nibble for nearby stars as the Plane method does.
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param log Open stream to log file.
void Query::trial_plane(Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    Benchmark beta(ch, seed, WORKING_FOV);
    Plane::Parameters p;
    p.table_name = PLANE_TABLE;
    p.query_expected = 10000;

    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int i = 0; i < QUERY_SAMPLES; i++) {
            beta.stars = generate_n_stars(ch, 3, seed);
            beta.focus = beta.stars[0];

            // Vary our area and moment sigma.
            beta.shift_light(3, ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i))), beta.error_models.clear();
            p.sigma_a = QS_MIN;
            p.sigma_i = QS_MIN;

            // Log our label values, and get our result set.
            Plane::label_trio b = {(double) beta.stars[0].get_label(), (double) beta.stars[1].get_label(),
                    (double) beta.stars[2].get_label()};
            double a_i = Trio::planar_area(beta.stars[0], beta.stars[1], beta.stars[2]);
            double i_i = Trio::planar_moment(beta.stars[0], beta.stars[1], beta.stars[2]);
            std::vector<Plane::label_trio> r = Plane(beta, p).query_for_trio(a_i, i_i);

            // Log our results.
            log << "Plane," << QS_MIN << "," << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << r.size() << ","
                << set_existence(r, b) << '\n';
        }
    }
}

/// Record the results of querying Nibble for nearby stars as the Sphere method does.
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param log Open stream to log file.
void Query::trial_sphere(Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    Benchmark beta(ch, seed, WORKING_FOV);
    Sphere::Parameters p;
    p.table_name = SPHERE_TABLE;
    p.query_expected = 10000;

    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int i = 0; i < QUERY_SAMPLES; i++) {
            beta.stars = generate_n_stars(ch, 3, seed);
            beta.focus = beta.stars[0];

            // Vary our area and moment sigma.
            beta.shift_light(3, ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i))), beta.error_models.clear();
            p.sigma_a = QS_MIN;
            p.sigma_i = QS_MIN;

            // Log our label values, and get our result set.
            Sphere::label_trio b = {(double) beta.stars[0].get_label(), (double) beta.stars[1].get_label(),
                    (double) beta.stars[2].get_label()};
            double a_i = Trio::spherical_area(beta.stars[0], beta.stars[1], beta.stars[2]);
            double i_i = Trio::spherical_moment(beta.stars[0], beta.stars[1], beta.stars[2],
                                                Sphere::Parameters().moment_td_h);
            std::vector<Sphere::label_trio> r = Sphere(beta, p).query_for_trio(a_i, i_i);

            // Log our results.
            log << "Sphere," << QS_MIN << "," << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << r.size() << ","
                << set_existence(r, b) << '\n';
        }
    }
}

/// Record the results of querying Nibble for nearby stars as the Pyramid method does (this is identical to the Angle
/// method).
///
/// @param ch Open Nibble connection.
/// @param log Open stream to log file.
void Query::trial_pyramid(Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    Benchmark beta(ch, seed, WORKING_FOV);

    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int i = 0; i < QUERY_SAMPLES; i++) {
            beta.stars = generate_n_stars(ch, 2, seed);
            beta.focus = beta.stars[0];
            beta.shift_light(2, ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i))), beta.error_models.clear();

            // Log our label values, and get our result set.
            ch.select_table(PYRAMID_TABLE);
            Pyramid::label_pair b = {beta.stars[0].get_label(), beta.stars[1].get_label()};
            std::vector<Pyramid::label_pair> r = Pyramid::trial_query(ch, beta.stars[0], beta.stars[1], QS_MIN);

            // Log our results.
            log << "Pyramid," << QS_MIN << "," << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << r.size() << ","
                << set_existence(r, b) << '\n';
        }
    }
}

/// Record the results of querying Nibble for nearby stars as the Summer method does (this is identical to the Planar
/// Triangles method).
///
/// @param ch Open Nibble connection.
/// @param log Open stream to log file.
void Query::trial_coin(Chomp &ch, std::ofstream &log) {
    std::random_device seed;
    Benchmark beta(ch, seed, WORKING_FOV);
    Summer::Parameters p;
    p.table_name = COIN_TABLE;
    p.query_expected = 10000;

    for (int ss_i = 0; ss_i < SS_ITER; ss_i++) {
        for (int i = 0; i < QUERY_SAMPLES; i++) {
            beta.stars = generate_n_stars(ch, 3, seed);
            beta.focus = beta.stars[0];

            // Vary our area and moment sigma.
            beta.shift_light(3, ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i))), beta.error_models.clear();
            p.sigma_a = QS_MIN;
            p.sigma_i = QS_MIN;

            // Log our label values, and get our result set.
            Summer::label_trio b = {beta.stars[0].get_label(), beta.stars[1].get_label(), beta.stars[2].get_label()};
            double a_i = Trio::planar_area(beta.stars[0], beta.stars[1], beta.stars[2]);
            double i_i = Trio::planar_moment(beta.stars[0], beta.stars[1], beta.stars[2]);
            std::vector<Summer::label_trio> r = Summer(beta, p).query_for_trios(a_i, i_i);

            // Log our results.
            log << "Summer," << QS_MIN << "," << ((ss_i == 0) ? 0 : SS_MULT * pow(10, ss_i)) << "," << r.size() << ","
                << set_existence(r, b) << '\n';
        }
    }
}
