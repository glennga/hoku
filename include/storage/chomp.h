/// @file chomp.h
/// @author Glenn Galvizo
///
/// Header file for Chomp class, another class to facilitate the retrieval and storage of various data. The Chomp
/// class is meant to provide more specific functions than Nibble, and also deals with data that does not reside
/// inside a database.

#ifndef HOKU_CHOMP_H
#define HOKU_CHOMP_H

#include "storage/nibble.h"

/// @brief Class for accessing the Hipparcos catalog.
class Chomp : public Nibble {
public:
    class Builder;
    using Nibble::tuples_d;

public:
    int generate_tables (const std::string &catalog_path, const std::string &current_time, double m_bright);
    Star::list bright_as_list ();

    Star query_hip (int label);
    tuples_d simple_bound_query (const std::vector<std::string> &foci, const std::string &fields,
                                 const std::vector<double> &y_a, const std::vector<double> &y_b,
                                 unsigned int expected);

    Star::list nearby_bright_stars (const Vector3 &focus, double fov, unsigned int expected);
    Star::list nearby_hip_stars (const Vector3 &focus, double fov, unsigned int expected);

    static const int TABLE_EXISTS;

private:
    Star::list all_bright_stars;
    Star::list all_hip_stars;
    std::string bright_table;
    std::string hip_table;

    void load_all_stars ();
    static std::array<double, 7> components_from_line (const std::string &entry, double y_t);

    static double year_difference (const std::string &current_time);

    Chomp (const std::string &database_name, const std::string &hip_name, const std::string &bright_name,
           const std::string &catalog_path = "", const std::string &current_time = "", double m_bright = 0);
};

class Chomp::Builder {
public:
    Builder &with_database_name (const std::string &name) {
        this->database_name = name;
        return *this;
    }
    Builder &with_hip_name (const std::string &name) {
        this->hip_name = name;
        return *this;
    }
    Builder &with_bright_name (const std::string &name) {
        this->bright_name = name;
        return *this;
    }
    Builder &using_catalog (const std::string &name) {
        this->catalog_path = name;
        return *this;
    }
    Builder &using_current_time (const std::string &time_string) {
        this->current_time = time_string;
        return *this;
    }
    Builder &limited_by_magnitude(double m) {
        this->m_bright = m;
        return *this;
    }
    Chomp build () { return Chomp(database_name, hip_name, bright_name, catalog_path, current_time, m_bright); }

private:
    std::string database_name;
    std::string current_time;
    std::string catalog_path;
    std::string bright_name;
    std::string hip_name;
    double m_bright;
};

#endif /* HOKU_CHOMP_H */
