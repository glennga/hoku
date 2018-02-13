/// @file spherical-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for SphericalTriangle class, which matches a set of body vectors (stars) to their inertial counter-parts
/// in the database.

#include <algorithm>

#include "math/trio.h"
#include "identification/spherical-triangle.h"

/// Default parameters for the spherical triangle identification method.
const Identification::Parameters Sphere::DEFAULT_PARAMETERS = {DEFAULT_SIGMA_QUERY, DEFAULT_SQL_LIMIT,
    DEFAULT_PASS_R_SET_CARDINALITY, DEFAULT_FAVOR_BRIGHT_STARS, DEFAULT_SIGMA_OVERLAY, DEFAULT_NU_MAX, DEFAULT_NU,
    DEFAULT_F, "SPHERE_20"};

/// Constructor. Sets the benchmark data and fov. Sets the parameters and working table.
///
/// @param input Working Benchmark instance. We are **only** copying the star set and the fov.
/// @param parameters Parameters to use for identification.
SphericalTriangle::SphericalTriangle (const Benchmark &input, const Parameters &parameters) : BaseTriangle() {
    input.present_image(this->input, this->fov);
    this->parameters = parameters;
    
    ch.select_table(this->parameters.table_name);
}

/// Generate the triangle table given the specified FOV and table name. This find the area and polar moment
/// between each distinct permutation of trios, and only stores them if they fall within the corresponding
/// field-of-view.
///
/// @param fov Field of view limit (degrees) that all pairs must be within.
/// @param table_name Name of the table to generate.
/// @return TABLE_ALREADY_EXISTS if the table already exists. Otherwise, 0 when finished.
int SphericalTriangle::generate_table (const double fov, const std::string &table_name) {
    return generate_triangle_table(fov, table_name, Trio::spherical_area,
                                   [] (const Star &b_1, const Star &b_2, const Star &b_3) {
                                       return Trio::spherical_moment(b_1, b_2, b_3, DEFAULT_TD_H);
                                   });
}

/// Given a trio of body stars, find matching trios of inertial stars using their respective spherical areas and polar
/// moments.
///
/// @param i_b Index trio of stars in body (B) frame.
/// @return NO_CANDIDATE_STARS_FOUND if stars are not within the fov or if no matches currently exist.
/// Otherwise, vector of trios whose areas and moments are close.
std::vector<Star::trio> Sphere::match_stars (const index_trio &i_b) {
    return m_stars(i_b, Trio::spherical_area, [] (const Star &b_1, const Star &b_2, const Star &b_3) {
        return Trio::spherical_moment(b_1, b_2, b_3, DEFAULT_TD_H);
    });
}

/// Find the matching pairs using the appropriate triangle table and by comparing areas and polar moments. Input image
/// is not used. We require the following be defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_query
///     - sql_limit
/// @endcode
///
/// @param s Stars to query with. This must be of length = QUERY_STAR_SET_SIZE.
/// @return Vector of likely matches found by the spherical triangle method.
std::vector<Identification::labels_list> Sphere::query (const Star::list &s) {
    if (s.size() != QUERY_STAR_SET_SIZE) {
        throw std::runtime_error(std::string("Input list does not have exactly three stars."));
    }
    
    std::vector<label_trio> h = e_query(Trio::spherical_area(s[0], s[1], s[2]),
                                        Trio::spherical_moment(s[0], s[1], s[2]));
    std::vector<labels_list> h_bar = {};
    
    // Convert our labels to lists.
    std::for_each(h.begin(), h.end(), [&h_bar] (const label_trio &ell) {
        h_bar.emplace_back(labels_list {ell[0], ell[1], ell[2]});
    });
    return h_bar;
}

/// Find the **best** matching pair to the first three stars in our benchmark using the appropriate triangle table.
/// Assumes noise is normally distributed, searches using epsilon (3 * sigma_a) and a basic bounded query. Input image
/// is used. We require the following be defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_query
///     - sql_limit
/// @endcode
///
/// @return NO_CANDIDATES_FOUND if we cannot query anything. Otherwise, a single match configuration found by the
/// spherical triangle method.
Identification::labels_list Sphere::reduce () {
    return e_reduction();
}

/// Find the rotation from the images in our current benchmark to our inertial frame (i.e. the catalog). Input image is
/// used. We require the following be defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_overlay
///     - sql_limit
///     - sigma_query
///     - nu
///     - nu_max
/// @endcode
///
/// @param input The set of benchmark data to work with.
/// @param p Adjustments to the identification process.
/// @return NO_CONFIDENT_IDENTITY if an identification cannot be found exhaustively. EXCEEDED_NU_MAX if an
/// identification cannot be found within a certain number of query picks. Otherwise, body stars b with the attached
/// labels of the inertial pair r.
Star::list Sphere::identify () {
    return e_identify();
}