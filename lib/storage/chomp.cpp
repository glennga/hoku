/// @file chomp.cpp
/// @author Glenn Galvizo
///
/// Source file for Chomp class, which is a child class of Nibble and provides more specific functions that facilitate
/// the retrieval and storage of various lookup tables.

#define _USE_MATH_DEFINES

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <libgen.h>

#include "storage/chomp.h"

const int Chomp::TABLE_EXISTS = -1;

Chomp::Chomp (const std::string &database_name, const std::string &hip_name, const std::string &bright_name,
              const std::string &catalog_path, const std::string &current_time, double m_bright) :
        Nibble(database_name) {
    this->bright_table = bright_name;
    this->hip_table = hip_name;

    // Generate the Hipparcos and Bright Hipparcos tables.
    if (!catalog_path.empty()) generate_tables(catalog_path, current_time, m_bright);
    load_all_stars();
}

/// Helper method for catalog table generation methods. Read the star catalog data and compute the {i, j, k} components
/// given a line from the ASCII catalog. Source: http://cdsarc.u-strasbg.fr/viz-bin/Cat?I/311#sRM2.1
///
/// @return Array of components in order {alpha, delta, i, j, k, m, label}.
std::array<double, 7> Chomp::components_from_line (const std::string &entry, const double y_t) {
    std::array<double, 7> components = {0, 0, 0, 0, 0, 0, 0};

    try {
        // Parse the proper motion of the right ascension and declination.
        double pm_alpha = stof(entry.substr(51, 8)), pm_delta = stof(entry.substr(60, 8));

        // Read right ascension. Convert to degrees. Update with correct position.
        double alpha = ((180.0 / M_PI) * stof(entry.substr(15, 13), nullptr)) + (pm_alpha * y_t * (1 / (3600000.0)));

        // Read declination. Convert to degrees. Update with correct position.
        double delta = ((180.0 / M_PI) * stof(entry.substr(29, 13), nullptr)) + (pm_delta * y_t * (1 / (3600000.0)));

        // Convert to cartesian w/ r = 1. Reduce to unit vector.
        double r[] = {(M_PI / 180.0) * alpha, (M_PI / 180.0) * delta};
        Vector3 s = Vector3::Normalized(Vector3(cos(r[0]) * cos(r[1]), sin(r[0]) * cos(r[1]), sin(r[1])));

        // Parse apparent magnitude and label.
        double m = stof(entry.substr(129, 7), nullptr), ell = stof(entry.substr(0, 6), nullptr);

        components = {alpha, delta, s.data[0], s.data[1], s.data[2], m, ell};
    }
    catch (std::exception &e) {
        // Ignore entries without recorded alpha or delta.
    }

    return components;
}

/// Determine the difference in years between the time of the catalog recording (J1991.25) and the given time.
double Chomp::year_difference (const std::string &current_time) {
    std::stringstream time_ss(current_time); // Determine the month (first token) and the year (second token).
    std::vector<std::string> tokens;
    for (std::string token; getline(time_ss, token, '-');) {
        tokens.push_back(token);
    }
    if (tokens.size() != 2) {
        throw std::runtime_error(std::string("'time' in 'hoku.cfg' is not formatted correctly."));
    }

    // Determine the year and month difference. The catalog stores positions at t = J1991.25 (03-1991).
    double y_t = stof(tokens[1], nullptr) - 1991, m_t = stof(tokens[0], nullptr) - 3;
    return y_t + m_t;
}

/// Parse the right ascension, declination, visual magnitude, and catalog ID for each star. The i, j, and k components
/// are converted from the star's alpha, delta and are moved to their proper location given the current time.
int Chomp::generate_tables (const std::string &catalog_path, const std::string &current_time, double m_bright) {
    auto create_table = [this, catalog_path, current_time, m_bright] (bool is_bright) {
        std::ifstream catalog(catalog_path);
        if (!catalog.is_open()) throw std::runtime_error(std::string("Catalog file cannot be opened."));

        SQLite::Transaction transaction(*conn);
        if (this->create_table(
                (is_bright) ? bright_table : hip_table,
                "alpha FLOAT, "
                "delta FLOAT, "
                "i FLOAT, "
                "j FLOAT, "
                "k FLOAT, "
                "m FLOAT, "
                "label INT"
        ) == TABLE_NOT_CREATED_RET)
            return TABLE_EXISTS;

        // We skip the header here.
        for (int i = 0; i < 5; i++) {
            std::string entry;
            getline(catalog, entry);
        }

        // Insert into bright stars table. Account for the proper motion of each star.
        double y_t = year_difference(current_time);
        for (std::string entry; getline(catalog, entry);) {
            std::array<double, 7> c = components_from_line(entry, y_t);

            // Only insert if magnitude < m_bright (visible light by detector).
            if (!is_bright || (c[5] < m_bright && !std::equal(c.begin() + 1, c.end(), c.begin()))) {
                insert_into_table("alpha, delta, i, j, k, m, label", tuple_d{c[0], c[1], c[2], c[3], c[4], c[5], c[6]});
            }
        }
        transaction.commit();

        return sort_and_index("label");
    };

    return create_table(true) + create_table(false);
}

/// Search the Hipparcos catalog in memory (all_hip_stars) for a star with the matching catalog ID. If the star does
/// not exist, than an exception is thrown. This is meant to discourage the use of guessing stars through labels.
Star Chomp::query_hip (int label) {
    for (const Star &s : all_hip_stars) {
        if (s.get_label() == label) return s;
    }

    throw std::runtime_error("Star does exist with the label: " + std::to_string(label) + ".");
}

Star::list Chomp::bright_as_list () { return this->all_bright_stars; }
Star::list Chomp::nearby_bright_stars (const Vector3 &focus, const double fov, const unsigned int expected) {
    Star::list nearby;
    nearby.reserve(expected);

    for (const Star &candidate : all_bright_stars) {
        if (Star::within_angle(focus, candidate, fov)) nearby.push_back(candidate);
    }

    return nearby;
}
Star::list Chomp::nearby_hip_stars (const Vector3 &focus, const double fov, const unsigned int expected) {
    Star::list nearby;
    nearby.reserve(expected);

    for (const Star &candidate : all_hip_stars) {
        if (Star::within_angle(focus, candidate, fov)) nearby.push_back(candidate);
    }

    return nearby;
}

void Chomp::load_all_stars () {
    // Reserve space for our tables. Note that we assume our HIP and BRIGHT tables have already been generated.
    SQLite::Statement query_b_ell(
            *conn,
            "SELECT COUNT(*) "
            "FROM " + bright_table
    );
    while (query_b_ell.executeStep()) this->all_bright_stars.reserve(query_b_ell.getColumn(0).getInt());
    SQLite::Statement query_h_ell(
            *conn,
            "SELECT COUNT(*) "
            "FROM " + hip_table
    );
    while (query_h_ell.executeStep()) this->all_hip_stars.reserve(query_h_ell.getColumn(0).getInt());

    // Select all for bright stars, and load this into RAM.
    select_table(bright_table);
    SQLite::Statement query_b(
            *conn,
            "SELECT i, j, k, label, m "
            "FROM " + bright_table
    );

    while (query_b.executeStep()) {
        this->all_bright_stars.emplace_back(
                Star(query_b.getColumn(0).getDouble(), query_b.getColumn(1).getDouble(),
                     query_b.getColumn(2).getDouble(),
                     query_b.getColumn(3).getInt(), query_b.getColumn(4).getDouble()));
    }

    // Select all for general stars stars, and load this into RAM.
    select_table(hip_table);
    SQLite::Statement query_h(
            *conn,
            "SELECT i, j, k, label, m "
            "FROM " + hip_table
    );
    while (query_h.executeStep()) {
        this->all_hip_stars.emplace_back(
                Star(query_h.getColumn(0).getDouble(), query_h.getColumn(1).getDouble(),
                     query_h.getColumn(2).getDouble(),
                     query_h.getColumn(3).getInt(), query_h.getColumn(4).getDouble()));
    }
}

/// Search a table for the specified fields given foci columns using a simple bound query. Searches for all results
/// between y_a and y_b.
Nibble::tuples_d Chomp::simple_bound_query (const std::vector<std::string> &foci, const std::string &fields,
                                            const std::vector<double> &y_a, const std::vector<double> &y_b,
                                            unsigned int expected) {
    std::ostringstream condition;
    select_table(current_table);

    condition << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed;
    for (unsigned int i = 0; i < foci.size(); i++) {
        condition << foci[i] << " BETWEEN " << y_a[i] << " AND " << y_b[i] << " ";
        if (i < foci.size() - 1) condition << " AND ";
    }

    Nibble::tuples_d result = search_table(fields, condition.str(), expected);
    return (result.empty()) ? Nibble::tuples_d{} : result;
}
