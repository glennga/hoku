/// @file chomp.cpp
/// @author Glenn Galvizo
///
/// Source file for Chomp class, which is a child class of Nibble and provides more specific functions that facilitate
/// the retrieval and storage of various lookup tables.

#include "storage/chomp.h"

/// Constructor. This dynamically allocates a database connection object to nibble.db. If the database does not exist,
/// it is created. We then proceed to load all stars into RAM from both tables.
Chomp::Chomp () {
    const int FLAGS = SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE;
    this->db = std::make_unique<SQLite::Database>(DATABASE_LOCATION, FLAGS);
    
    generate_hip_table();
    generate_bright_table();
    load_all_stars();
}

/// Helper method for catalog table generation methods. Read the star catalog data and compute the {i, j, k} components
/// given a line from the ASCII catalog.
///
/// @param entry String containing line from Hipparcos ASCII catalog.
/// @return Array of components in order {alpha, delta, i, j, k, m}.
std::array<double, 6> Chomp::components_from_line (const std::string &entry) {
    std::array<double, 6> components = {0, 0, 0, 0, 0, 0};
    
    try {
        // Read right ascension. Convert to degrees.
        double alpha = (180.0 / M_PI) * stof(entry.substr(15, 13), nullptr);
        
        // Read declination. Convert to degrees.
        double delta = (180.0 / M_PI) * stof(entry.substr(29, 13), nullptr);
        
        // Convert to cartesian w/ r = 1. Reduce to unit vector.
        Star star_entry(1.0 * cos((M_PI / 180.0) * delta) * cos((M_PI / 180.0) * alpha),
                        1.0 * cos((M_PI / 180.0) * delta) * sin(M_PI / 180.0 * alpha),
                        1.0 * sin((M_PI / 180.0) * delta), 0, true);
        
        // Parse apparent magnitude.
        double m = strtof(entry.substr(129, 7).c_str(), nullptr);
        components = {alpha, delta, star_entry[0], star_entry[1], star_entry[2], m};
    }
    catch (std::exception &e) {
        // Ignore entries without recorded alpha or delta.
    }
    
    return components;
}

/// Parse the right ascension, declination, visual magnitude, and catalog ID for each star. The i, j, and k components
/// are converted from the star's alpha, delta, and an assumed parallax = 1.
///
/// @return -1 if the bright stars table already exists. 0 otherwise.
int Chomp::generate_bright_table () {
    std::ifstream catalog(HIP_CATALOG_LOCATION);
    if (!catalog.is_open()) {
        throw "Catalog file cannot be opened";
    }
    
    SQLite::Transaction transaction(*db);
    if (create_table(BRIGHT_TABLE, "alpha FLOAT, delta FLOAT, i FLOAT, j FLOAT, k FLOAT, m FLOAT, label INT") == -1) {
        return -1;
    }
    
    // We skip the header here.
    for (int i = 0; i < 5; i++) {
        std::string entry;
        getline(catalog, entry);
    }
    
    // Insert into bright stars table.
    double label = 1;
    for (std::string entry; getline(catalog, entry);) {
        std::array<double, 6> c = components_from_line(entry);
        
        // Only insert if magnitude < 6.0 (visible light).
        if (c[5] < 6.0 && !std::equal(c.begin() + 1, c.end(), c.begin())) {
            insert_into_table("alpha, delta, i, j, k, m, label", tuple_d {c[0], c[1], c[2], c[3], c[4], c[5], label});
        }
        label += 1;
    }
    transaction.commit();
    
    // Polish table. Sort by catalog ID.
    return polish_table("label");
}

/// Parse the right ascension, declination, visual magnitude, and catalog ID for each star. The i, j, and k components
/// are converted from the star's alpha, delta, and an assumed parallax = 1.
///
/// @return -1 if the general stars table already exists. 0 otherwise.
int Chomp::generate_hip_table () {
    std::ifstream catalog(HIP_CATALOG_LOCATION);
    if (!catalog.is_open()) {
        throw "Catalog file cannot be opened";
    }
    
    SQLite::Transaction transaction(*db);
    if (create_table("HIP", "alpha FLOAT, delta FLOAT, i FLOAT, j FLOAT, k FLOAT, m FLOAT, label INT") == -1) {
        return -1;
    }
    
    // We skip the header here.
    for (int i = 0; i < 5; i++) {
        std::string entry;
        getline(catalog, entry);
    }
    
    // Insert into general stars table. There is no magnitude discrimination here.
    double label = 1;
    for (std::string entry; getline(catalog, entry);) {
        std::array<double, 6> c = components_from_line(entry);
        insert_into_table("alpha, delta, i, j, k, m, label", tuple_d {c[0], c[1], c[2], c[3], c[4], c[5], label});
        label += 1;
    }
    transaction.commit();
    
    // Polish table. Sort by catalog ID.
    return polish_table("label");
}

/// Search the general stars table for the star with the matching catalog ID. Return a star with the vector components
/// and catalog ID of this search.
///
/// @param label Catalog ID of the star to return.
/// @return Zero star if the star does not exist. Star with the components of the matching catalog ID entry otherwise.
Star Chomp::query_hip (int label) {
    std::string t_table = this->table;
    
    select_table(HIP_TABLE);
    tuples_d results = search_table("i, j, k, m", "label = " + std::to_string(label), 1, 1);
    
    // Keep our previous table.
    select_table(t_table);
    return results.empty() ? Star::zero() : Star(results[0][0], results[0][1], results[0][2], label, results[0][3]);
}

/// Accessor for all_bright_stars list.
///
/// @return Array with all stars in the Nibble table with a magnitude less than 6.0.
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
    select_table(BRIGHT_TABLE);
    SQLite::Statement query_b(*db, "SELECT i, j, k, label, m FROM " + BRIGHT_TABLE);
    while (query_b.executeStep()) {
        this->all_bright_stars.emplace_back(
            Star(query_b.getColumn(0).getDouble(), query_b.getColumn(1).getDouble(), query_b.getColumn(2).getDouble(),
                 query_b.getColumn(3).getInt(), query_b.getColumn(4).getDouble()));
    }
    
    // Select all for general stars stars, and load this into RAM.
    select_table(HIP_TABLE);
    SQLite::Statement query_h(*db, "SELECT i, j, k, label, m FROM " + HIP_TABLE);
    while (query_h.executeStep()) {
        this->all_hip_stars.emplace_back(
            Star(query_h.getColumn(0).getDouble(), query_h.getColumn(1).getDouble(), query_h.getColumn(2).getDouble(),
                 query_h.getColumn(3).getInt(), query_h.getColumn(4).getDouble()));
    }
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
    int s_l = (*db).execAndGet(std::string("SELECT MAX(rowid) FROM ") + table).getInt();
    SQLite::Transaction table_transaction(*db);
    int k_hat = 0;
    
    // Load the entire table into RAM.
    tuples_d s_vector = search_table(focus_column, (unsigned) s_l);
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
    
    // Index the K-Vector column.
    SQLite::Transaction index_transaction(*db);
    (*db).exec("CREATE INDEX " + table + "_" + focus_column + " ON " + table + "(k_value)");
    index_transaction.commit();
    
    return 0;
}

/// Create the K-Vector for the given for the given table using the specified focus column.
///
/// @param focus Name of the column to construct K-Vector with.
/// @return 0 when finished.
int Chomp::create_k_vector (const std::string &focus) {
    sort_table(focus);
    SQLite::Transaction transaction(*db);
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
    create_table(table + "_KVEC", "k_value INT");
    transaction.commit();
    
    select_table(original_table);
    return build_k_vector_table(focus, m, q);
}

/// Search a table for the specified fields given a focus column using the K-Vector method. Searches for all results
/// between y_a and y_b. The results are a 1D array that holds a fixed number of items in succession.
///
/// *REQUIRES Chomp::create_k_vector TO HAVE BEEN RUN BEFOREHAND**
///
/// @param focus Table must be sorted and built upon this column. This is the search column.
/// @param fields The columns to search for in the given table.
/// @param y_a Lower bound of the focus to search for.
/// @param y_b Upper bound of the focus to search for.
/// @param expected Expected number of results to be returned. Better to overshoot.
/// @return List of results (in form of tuples), in order of that queried from Nibble.
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
    sql = "rowid BETWEEN " + std::to_string(s_endpoints.front()[0]) + " AND " + std::to_string(s_endpoints.back()[0]);
    return search_table(fields, sql, expected);
}
