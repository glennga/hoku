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

#include "storage/chomp.h"

/// Standard machine epsilon for doubles. This represents the smallest possible change in precision.
const double Chomp::DOUBLE_EPSILON = std::numeric_limits<double>::epsilon();

/// Returned from query method if the specified star does not exist.
const Star Chomp::NONEXISTENT_STAR = Star::wrap(Vector3::Zero());

/// Returned from bound query methods if the stars do not exist.
const Nibble::tuples_d Chomp::RESULTANT_EMPTY = {};

/// Constructor. This dynamically allocates a database connection object to nibble.db. If the database does not exist,
/// it is created. We then proceed to load all stars into RAM from both tables.
Chomp::Chomp () : Nibble() {
    // Parse the table names.
    INIReader cf(std::string(std::getenv("HOKU_PROJECT_PATH")) + std::string("/CONFIG.ini"));
    this->bright_table = cf.Get("table-names", "bright", "");
    this->hip_table = cf.Get("table-names", "hip", "");
    
    // Generate the Hipparcos and Bright Hipparcos tables.
    generate_table(cf, true);
    generate_table(cf, false);
    load_all_stars();
}

/// Overloaded constructor. This loads a single, specified table into memory. All future queries must involve solely
/// this table. The HIP and HIP_BRIGHT tables will still be stored into the list.
///
/// @param table_name Name of the table to load into memory.
/// @param focus Name of the focus column to polish table with.
Chomp::Chomp (const std::string &table_name, const std::string &focus) : Nibble(table_name, focus) {
    Chomp ch_t;
    
    // Parse the table names (for consistency).
    INIReader cf(std::string(std::getenv("HOKU_PROJECT_PATH")) + std::string("/CONFIG.ini"));
    this->bright_table = cf.Get("table-names", "bright", "");
    this->hip_table = cf.Get("table-names", "hip", "");
    
    // Store the star lists. These are used by Chomp query methods.
    this->all_bright_stars = ch_t.all_bright_stars;
    this->all_hip_stars = ch_t.all_hip_stars;
}

/// Helper method for catalog table generation methods. Read the star catalog data and compute the {i, j, k} components
/// given a line from the ASCII catalog. Source: http://cdsarc.u-strasbg.fr/viz-bin/Cat?I/311#sRM2.1
///
/// @param entry String containing line from Hipparcos ASCII catalog.
/// @param y_t Difference in years from the catalog to the current date.
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

/// Determine the difference in years between the time of the catalog recording (J1991.25) and the time specified in
/// the configuration file.
///
/// @param cf Configuration reader holding all parameters to use.
/// @return Difference in years from J1991.25 and the month & year specified in CONFIG.ini.
double Chomp::year_difference (INIReader &cf) {
    std::string time_e = cf.Get("hardware", "time", "");
    if (time_e == "") {
        throw std::runtime_error(std::string("'time' in 'CONFIG.ini' is not formatted correctly."));
    }
    
    // Determine the month (first token) and the year (second token).
    std::stringstream time_ss(time_e);
    std::vector<std::string> tokens;
    for (std::string token; getline(time_ss, token, '-');) {
        tokens.push_back(token);
    }
    if (tokens.size() != 2) {
        throw std::runtime_error(std::string("'time' in 'CONFIG.ini' is not formatted correctly."));
    }
    
    // Determine the year and month difference. The catalog stores positions at t = J1991.25 (03-1991).
    double y_t = stof(tokens[1], nullptr) - 1991, m_t = stof(tokens[0], nullptr) - 3;
    return y_t + m_t;
}

/// Parse the right ascension, declination, visual magnitude, and catalog ID for each star. The i, j, and k components
/// are converted from the star's alpha, delta and are moved to their proper location given the current time.
///
/// @param cf Configuration reader holding all parameters to use.
/// @param m_flag If true, generate restrict the magnitude of each entry and insert only bright stars.
/// @return TABLE_EXISTS if the bright stars table already exists. 0 otherwise.
int Chomp::generate_table (INIReader &cf, bool m_flag) {
    std::ifstream catalog(PROJECT_LOCATION + "/data/hip2.dat");
    if (!catalog.is_open()) {
        throw std::runtime_error(std::string("Catalog file cannot be opened."));
    }
    
    SQLite::Transaction transaction(*conn);
    if (create_table((m_flag) ? bright_table : hip_table,
                     "alpha FLOAT, delta FLOAT, i FLOAT, j FLOAT, k FLOAT, m FLOAT, label INT") == TABLE_NOT_CREATED) {
        return TABLE_EXISTS;
    }
    
    // We skip the header here.
    for (int i = 0; i < 5; i++) {
        std::string entry;
        getline(catalog, entry);
    }
    
    // Insert into bright stars table. Account for the proper motion of each star.
    double y_t = year_difference(cf), m_bright = cf.GetReal("hardware", "m-bright", 6.0);
    for (std::string entry; getline(catalog, entry);) {
        std::array<double, 7> c = components_from_line(entry, y_t);
        
        // Only insert if magnitude < m_bright (visible light by detector).
        if (!m_flag || (c[5] < m_bright && !std::equal(c.begin() + 1, c.end(), c.begin()))) {
            insert_into_table("alpha, delta, i, j, k, m, label", tuple_d {c[0], c[1], c[2], c[3], c[4], c[5], c[6]});
        }
    }
    transaction.commit();
    
    // Polish table. Sort by catalog ID.
    return polish_table("label");
}

/// Search the Hipparcos catalog in memory (all_hip_stars) for a star with the matching catalog ID.
///
/// @param label Catalog ID of the star to return.
/// @return NONEXISTENT_STAR if the star does not exist. Star with the components of the matching catalog ID entry
/// otherwise.
Star Chomp::query_hip (int label) {
    for (const Star &s : all_hip_stars) {
        if (s.get_label() == label) {
            return s;
        }
    }
    return NONEXISTENT_STAR;
}

/// Accessor for all_bright_stars list.
///
/// @return List with all stars in the Nibble table with a magnitude less than 6.0.
Star::list Chomp::bright_as_list () {
    return this->all_bright_stars;
}

/// Given a focus star and a field of view limit, find all bright stars around the focus.
///
/// @param focus Star to search around.
/// @param fov Limit a star must be separated from the focus by.
/// @param expected Expected number of stars around the focus. Better to overshoot.
/// @return Array with all nearby bright stars.
Star::list Chomp::nearby_bright_stars (const Star &focus, const double fov, const unsigned int expected) {
    Star::list nearby;
    nearby.reserve(expected);
    
    for (const Star &candidate : all_bright_stars) {
        if (Star::within_angle(focus, candidate, fov)) {
            nearby.push_back(candidate);
        }
    }
    
    return nearby;
}

/// Given a focus star and a field of view limit, find all stars in the Hipparcos catalog around the focus.
///
/// @param focus Star to search around.
/// @param fov Limit a star must be separated from the focus by.
/// @param expected Expected number of star around the focus. Better to overshoot.
/// @return Array with all nearby stars (no magnitude restriction).
Star::list Chomp::nearby_hip_stars (const Star &focus, const double fov, const unsigned int expected) {
    Star::list nearby;
    nearby.reserve(expected);
    
    for (const Star &candidate : all_hip_stars) {
        if (Star::within_angle(focus, candidate, fov)) {
            nearby.push_back(candidate);
        }
    }
    
    return nearby;
}

/// Load all of the stars in BSC5 to star_list. Assumes that both the bright star and general star tables were
/// generated.
void Chomp::load_all_stars () {
    // Reserve space for the list.
    this->all_bright_stars.reserve(BRIGHT_TABLE_LENGTH);
    this->all_hip_stars.reserve(HIP_TABLE_LENGTH);
    
    // Select all for bright stars, and load this into RAM.
    select_table(bright_table);
    SQLite::Statement query_b(*conn, "SELECT i, j, k, label, m FROM " + bright_table);
    while (query_b.executeStep()) {
        this->all_bright_stars.emplace_back(
            Star(query_b.getColumn(0).getDouble(), query_b.getColumn(1).getDouble(), query_b.getColumn(2).getDouble(),
                 query_b.getColumn(3).getInt(), query_b.getColumn(4).getDouble()));
    }
    
    // Select all for general stars stars, and load this into RAM.
    select_table(hip_table);
    SQLite::Statement query_h(*conn, "SELECT i, j, k, label, m FROM " + hip_table);
    while (query_h.executeStep()) {
        this->all_hip_stars.emplace_back(
            Star(query_h.getColumn(0).getDouble(), query_h.getColumn(1).getDouble(), query_h.getColumn(2).getDouble(),
                 query_h.getColumn(3).getInt(), query_h.getColumn(4).getDouble()));
    }
}

/// Search a table for the specified fields given a focus column using a simple bound query. Searches for all results
/// between y_a and y_b.
///
/// @param focus Our search attribute.
/// @param fields The attributes to search for in the given table.
/// @param y_a Lower bound of the focus to search for.
/// @param y_b Upper bound of the focus to search for.
/// @param limit Maximum number of results to retrieve.
/// @return RESULTANT_EMPTY if there exists no results returned. Otherwise, A list of results (in form of tuples),
/// in order of that queried from Nibble.
Nibble::tuples_d Chomp::simple_bound_query (const std::string &focus, const std::string &fields, const double y_a,
                                            const double y_b, const unsigned int limit) {
    std::ostringstream condition;
    
    select_table(table);
    condition << focus << " BETWEEN " << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed;
    condition << y_a << " AND " << y_b;
    Nibble::tuples_d result = search_table(fields, condition.str(), limit * 3, limit);
    
    return (result.empty()) ? RESULTANT_EMPTY : result;
}

/// A helper method for the create_k_vector function. Build the K-Vector table for the given table using the
/// specified focus column and Z-equation description. K(i) = j represents how many elements j in our s_vector (the
/// focus column) are below Z(i).
///
/// @param focus_column Name of column to act as S-Vector.
/// @param m Slope parameter of Z equation.
/// @param q Y-Intercept parameter of Z equation.
/// @return 0 when finished.
int Chomp::build_k_vector_table (const std::string &focus_column, const double m, const double q) {
    int s_l = (*conn).execAndGet(std::string("SELECT MAX(rowid) FROM ") + table).getInt();
    SQLite::Transaction table_transaction(*conn);
    int k_hat = 0;
    
    // Load the entire table into RAM.
    tuples_d s_vector = search_table(focus_column, (unsigned) s_l);
    std::string original_table = table;
    select_table(table + "_KVEC");
    
    // Load K-Vector into table, K(i) = j where s(j) <= z(i) < s(j + 1).
    for (int i = 1; i <= s_l; i++) {
        for (int k = k_hat; k <= s_l; k++) {
            if (s_vector[std::max(0, k - 1)][0] >= m * i + q) {
                insert_into_table("k_value", tuple_i {k});
                
                // We remember our previous k, and continue here for i + 1.
                k_hat = k;
                break;
            }
        }
        
        std::cout << "\r" << "Current *I* Entry: " << i;
    }
    table_transaction.commit();
    std::cout << std::endl;
    
    // Index the K-Vector column and the original table.
    SQLite::Transaction index_transaction(*conn);
    (*conn).exec("CREATE INDEX " + table + "_" + focus_column + " ON " + table + "(k_value)");
    (*conn).exec(
        "CREATE INDEX " + original_table + "_" + focus_column + " ON " + original_table + "(" + focus_column + ")");
    index_transaction.commit();
    
    return 0;
}

/// Create the K-Vector for the given for the given table using the specified focus column. Note that this technique
/// works best for linearly distributed data (i.e. the angles between each combination of two stars).
///
/// @param focus Name of the column to construct K-Vector with.
/// @return TABLE_NOT_CREATED if the table already exists. Otherwise, 0 when finished.
int Chomp::create_k_vector (const std::string &focus) {
    sort_table(focus);
    SQLite::Transaction transaction(*conn);
    std::string original_table = this->table;
    
    // Search for last and first element of sorted table.
    std::string sql_for_max_id = std::string("(SELECT MAX(rowid) FROM ") + table + ")";
    double focus_n = search_single(focus, "rowid = " + sql_for_max_id);
    double focus_0 = search_single(focus, "rowid = 1");
    
    // Determine Z equation, this creates slightly steeper line.
    double n = search_single("MAX(rowid)");
    double m = (focus_n - focus_0 + (2.0 * DOUBLE_EPSILON)) / (int) (n - 1);
    double q = focus_0 - m - DOUBLE_EPSILON;
    
    // Sorted table is s-vector. Create k-vector and build the k-vector table.
    if (create_table(table + "_KVEC", "k_value INT") == TABLE_NOT_CREATED) {
        return TABLE_NOT_CREATED;
    }
    transaction.commit();
    
    select_table(original_table);
    return build_k_vector_table(focus, m, q);
}

/// Search a table for the specified fields given a focus column using the K-Vector method. Searches for all results
/// between y_a and y_b.
///
/// *REQUIRES Chomp::create_k_vector TO HAVE BEEN RUN BEFOREHAND**
///
/// @param focus Table must be sorted and built upon this column. This is the search column.
/// @param fields The columns to search for in the given table.
/// @param y_a Lower bound of the focus to search for.
/// @param y_b Upper bound of the focus to search for.
/// @param expected Expected number of results to be returned. Better to overshoot.
/// @return Empty list if k_value does not exist. Otherwise, an empty list of results (in form of tuples), in order of
/// that queried from Nibble.
Nibble::tuples_d Chomp::k_vector_query (const std::string &focus, const std::string &fields, const double y_a,
                                        const double y_b, const unsigned int expected) {
    tuples_d s_endpoints;
    std::string sql, s_table = table;
    
    // Search for last and first element of sorted table.
    std::string sql_for_max_id = std::string("(SELECT MAX(rowid) FROM ") + table + ")";
    double focus_n = search_single(focus, "rowid = " + sql_for_max_id);
    double focus_0 = search_single(focus, "rowid = 1");
    
    // Determine Z equation, this creates slightly steeper line.
    double n = search_single("MAX(rowid)");
    double m = (focus_n - focus_0 + (2.0 * DOUBLE_EPSILON)) / (int) (n - 1);
    double q = focus_0 - m - DOUBLE_EPSILON;
    
    // Determine indices of search, and perform search.
    double j_b = floor((y_a - q) / m);
    double j_t = ceil((y_b - q) / m);
    
    // Get index to s-vector (original table).
    select_table(s_table + "_KVEC");
    sql = "rowid BETWEEN " + std::to_string((int) j_b) + " AND " + std::to_string((int) j_t);
    s_endpoints = search_table("k_value", sql, expected / 2);
    
    select_table(s_table);
    
    if (!s_endpoints.empty()) {
        sql =
            "rowid BETWEEN " + std::to_string(s_endpoints.front()[0]) + " AND " + std::to_string(s_endpoints.back()[0]);
        return search_table(fields, sql, expected);
    }
    else {
        return {};
    }
}