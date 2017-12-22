/// @file planar-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for PlanarTriangle class, which matches a set of body vectors (stars) to their inertial counter-parts in
/// the database.

#include "identification/planar-triangle.h"

/// Constructor. Sets the benchmark data and fov. Sets the parameters and working table. Constructs the
/// quadtree and saves the root.
///
/// @param input Working Benchmark instance. We are **only** copying the star set and the fov.
/// @param parameters Parameters to use for identification.
PlanarTriangle::PlanarTriangle (const Benchmark &input, const Parameters &parameters) {
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
/// @return 0 when finished.
int PlanarTriangle::generate_plane_table (const double fov, const std::string &table_name) {
    return BaseTriangle::generate_triangle_table(fov, table_name, Trio::planar_area, Trio::planar_moment);
}

/// Given a trio of body stars, find matching trios of inertial stars using their respective planar areas and polar
/// moments.
///
/// @param i_b Index trio of stars in body (B) frame.
/// @return 1D vector of a trio of Star(0, 0, 0) if stars are not within the fov or if no matches currently exist.
/// Otherwise, vector of trios whose areas and moments are close.
std::vector<Trio::stars> Plane::match_stars (const index_trio &i_b) {
    return m_stars(i_b, Trio::planar_area, Trio::planar_moment);
}

/// Find the matching pairs using the appropriate triangle table and by comparing areas and polar moments.
///
/// @param ch Open Nibble connection with Chomp.
/// @param s_1 Star one to query with.
/// @param s_2 Star two to query with.
/// @param s_3 Star three to query with.
/// @param sigma_query Theta must be within 3 * sigma_query to appear in results.
/// @return Vector of likely matches found by the planar triangle method.
std::vector<Plane::label_trio> Plane::experiment_query (Chomp &ch, const Star &s_1, const Star &s_2, const Star &s_3,
                                                        double sigma_query) {
    double area = Trio::planar_area(s_1, s_2, s_3), moment = Trio::planar_moment(s_1, s_2, s_3);
    
    // Set our parameters.
    Parameters p;
    p.sigma_query = sigma_query;
    
    return Plane(Benchmark::black(), p).e_query(area, moment);
}

/// Check all possible configuration of star trios and return quaternion corresponding to the set with the largest
/// number of reference to body matches.
///
/// @param ch Open Nibble connection with Chomp.
/// @param input Benchmark containing the stars in the given image.
/// @param candidates All stars found near the inertial pair.
/// @param r Inertial (frame R) pair of stars that match the body pair.
/// @param b Body (frame B) pair of stars that match the inertial pair.
/// @param sigma_overlay Star must be within 3 * sigma_overlay of the resulting inertial rotation.
/// @return The quaternion associated with the largest set of matching stars across the body and inertial in both
/// pairing configurations.
Rotation Plane::experiment_alignment (Chomp &ch, const Benchmark &input, const Star::list &candidates,
                                      const Trio::stars &r, const Trio::stars &b, double sigma_overlay) {
    Parameters p;
    p.sigma_overlay = sigma_overlay;
    
    return Plane(input, p).e_alignment(candidates, r, b);
}

/// Find the **best** matching pair to the first three stars in our benchmark using the appropriate triangle table.
/// Assumes noise is normally distributed, searches using epsilon (3 * sigma_a) and a basic bounded query.
///
/// @param input The set of benchmark data to work with.
/// @param p Adjustments to the identification process.
/// @return [0, 0, 0] if no match is found. Otherwise, a single match for the first three stars.
Plane::label_trio Plane::experiment_reduction (const Benchmark &input, const Parameters &p) {
    return Plane(input, p).e_reduction();
}

/// Find the rotation from the images in our current benchmark to our inertial frame (i.e. the catalog).
///
/// @param input The set of benchmark data to work with.
/// @param p Adjustments to the identification process.
/// @return The identity rotation if no rotation can be found. Otherwise, the rotation from our current benchmark to
/// the catalog.
Rotation Plane::experiment_attitude (const Benchmark &input, const Parameters &p) {
    return Plane(input, p).e_attitude();
}

/// Match the stars found in the current benchmark to those in the Nibble database. The child class should wrap this
/// function as 'experiment_crown' to mimic the other methods.
///
/// @param input The set of benchmark data to work with.
/// @param p Adjustments to the identification process.
/// @return Empty list if an image match cannot be found in "time". Otherwise, a vector of body stars with their
/// inertial catalog IDs that qualify as matches.
Star::list Plane::experiment_crown (const Benchmark &input, const Parameters &p) {
    return Plane(input, p).e_crown();
}