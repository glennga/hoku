/// @file hoku.cpp
/// @author Glenn Galvizo
///
/// Source file for Hoku class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#include "identification/hoku.h"

/// Constructor. Sets the benchmark data, fov, parameters, and current working table.
///
/// @param input Working Benchmark instance. We are **only** copying the star set and the fov.
Hoku::Hoku(const Benchmark &input, const Parameters &p) {
    input.present_image(this->input, this->fov);
    this->parameters = p;

    this->ch.select_table(parameters.table_name);
}

/// Generate the asterim table given the specified FOV and table name. This finds all asterisms between each distinct
/// permutation of quads of stars, and only stores them if they fall within the corresponding field-of-view.
///
/// @param fov Field of view limit (degrees) that all pairs must be within.
/// @param table_name Name of the table to generate.
/// @return 0 when finished.
int Hoku::generate_asterism_table(const double fov, const std::string &table_name) {
    Chomp ch;
    SQLite::Transaction initial_transaction(*ch.db);
    ch.create_table(table_name, "label_a INT, label_b INT, label_c INT, label_d INT, cx FLOAT, cy FLOAT, dx FLOAT, "
            "dy FLOAT");
    initial_transaction.commit();
    ch.select_table(table_name);

    // (i, j, k, m) are distinct, where no (i, j, k, m) = (j, k, m, i), (k, j, m, i), ...
    Star::list all_stars = ch.bright_as_list();
    for (unsigned int i = 0; i < all_stars.size() - 3; i++) {
        SQLite::Transaction transaction(*ch.db);
        std::cout << "\r" << "Current *I* Star: " << all_stars[i].get_label();
        for (unsigned int j = i + 1; j < all_stars.size() - 2; j++) {
            for (unsigned int k = j + 1; k < all_stars.size() - 1; k++) {
                for (unsigned int m = k + 1; m < all_stars.size(); m++) {
                    Asterism::points_cd h_t = Asterism::hash({all_stars[i], all_stars[j], all_stars[k], all_stars[m]});

                    // Check if the hash returned is valid, and if all stars are within FOV degrees.
                    if (Star::within_angle({all_stars[i], all_stars[j], all_stars[k], all_stars[m]}, fov) &&
                            h_t[0] + h_t[1] + h_t[2] + h_t[3] != 0) {
                        ch.insert_into_table("label_a, label_b, label_c, label_d, cx, cy, dx, dy",
                                             Nibble::tuple_d {(double) all_stars[i].get_label(),
                                                     (double) all_stars[j].get_label(),
                                                     (double) all_stars[k].get_label(),
                                                     (double) all_stars[m].get_label(),
                                                     h_t[0], h_t[1], h_t[2], h_t[3]});
                    }
                }
            }
        }

        // Commit every star I change.
        transaction.commit();
    }

    return ch.polish_table("cx");
}
