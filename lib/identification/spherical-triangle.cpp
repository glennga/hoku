/// @file spherical-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for SphericalTriangle class, which matches a set of body vectors (stars) to their inertial counter-parts
/// in the database.

#include <algorithm>

#include "math/trio.h"
#include "identification/spherical-triangle.h"

/// Default tree depth for calculating the spherical moment.
const int Sphere::DEFAULT_TD_H = 3;

/// Exact number of query stars required for query experiment.
const unsigned int Sphere::QUERY_STAR_SET_SIZE = 3;

/// Default parameters for the spherical triangle identification method.
const Identification::Parameters Sphere::DEFAULT_PARAMETERS = {DEFAULT_SIGMA_QUERY, DEFAULT_SIGMA_QUERY,
    DEFAULT_SIGMA_QUERY, DEFAULT_SIGMA_4, DEFAULT_SQL_LIMIT, DEFAULT_NO_REDUCTION, DEFAULT_FAVOR_BRIGHT_STARS,
    DEFAULT_NU_MAX, DEFAULT_NU, DEFAULT_F, "SPHERE_20"};

/// Constructor. Sets the benchmark data and fov. Sets the parameters and working table.
///
/// @param input Working Benchmark instance. We are **only** copying the star set and the fov.
/// @param parameters Parameters to use for identification.
SphericalTriangle::SphericalTriangle (const Benchmark &input, const Parameters &parameters) : BaseTriangle() {
    input.present_image(this->big_i, this->fov);
    this->parameters = parameters;

    this->parameters.nu = (parameters.nu == nullptr) ? std::make_shared<unsigned int>(0) : parameters.nu;
    ch.select_table(this->parameters.table_name);
}

/// Generate the triangle table given the specified FOV and table name. This find the area and polar moment
/// between each distinct permutation of trios, and only stores them if they fall within the corresponding
/// field-of-view.
///
/// @param cf Configuration reader holding all parameters to use.
/// @return TABLE_ALREADY_EXISTS if the table already exists. Otherwise, 0 when finished.
int SphericalTriangle::generate_table (INIReader &cf) {
    return generate_triangle_table(cf, "sphere", Trio::spherical_area,
                                   [] (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3) {
                                       return Trio::spherical_moment(b_1, b_2, b_3, DEFAULT_TD_H);
                                   });
}

/// Given a trio of body stars, find matching trios of inertial stars using their respective spherical areas and polar
/// moments.
///
/// @param c Index trio of stars ('combination' of I) in body frame.
/// @return NO_CANDIDATE_STARS_FOUND if stars are not within the fov or if no matches currently exist.
/// Otherwise, vector of trios whose areas and moments are close.
std::vector<Star::trio> Sphere::query_for_trios (const index_trio &c) {
    return base_query_for_trios(c, Trio::spherical_area,
                                [] (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3) {
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
        throw std::runtime_error(std::string("Input list does not have exactly three b."));
    }
    
    return e_query(Trio::spherical_area(s[0], s[1], s[2]), Trio::spherical_moment(s[0], s[1], s[2]));
}

/// Find the **best** matching pair to the first three stars in our benchmark using the appropriate triangle table.
/// Assumes noise is normally distributed, searches using epsilon (3 * sigma_a) and a basic bounded query. Input image
/// is used. We require the following be defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_query
///     - sql_limit
///     - nu
///     - nu_max
/// @endcode
///
/// @return NO_CONFIDENT_R if we cannot query anything. Otherwise, a single match configuration found by the
/// spherical triangle method.
Star::list Sphere::reduce () {
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
/// @return NO_CONFIDENT_A if an identification cannot be found exhaustively. EXCEEDED_NU_MAX if an
/// identification cannot be found within a certain number of query picks. Otherwise, body stars b with the attached
/// labels of the inertial pair r.
Star::list Sphere::identify () {
    return e_identify();
}