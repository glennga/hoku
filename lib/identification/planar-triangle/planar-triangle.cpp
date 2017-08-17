/// @file planar-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for PlanarTriangle class, which matches a set of body vectors (stars) to their inertial counter-parts in
/// the database.

#include "planar-triangle.h"

/// Constructor. Sets the benchmark data, fov, and focus. Sets the parameters and working table. Constructs the
/// quadtree and saves the root.
///
/// @param input Working Benchmark instance. We are **only** copying the star set, focus star, and the fov.
/// @param parameters Parameters to use for identification.
PlanarTriangle::PlanarTriangle (const Benchmark &input, const Parameters &parameters) {
    input.present_image(this->input, this->focus, this->fov);
    this->parameters = parameters;
    
    ch.select_table(this->parameters.table_name);
    q_root = std::make_shared<QuadNode>(QuadNode::load_tree(this->parameters.bsc5_quadtree_w));
}

/// Generate the triangle table given the specified FOV and table name. This find the planar area and polar moment
/// between each distinct permutation of trios, and only stores them if they fall within the corresponding
/// field-of-view.
///
/// @param fov Field of view limit (degrees) that all pairs must be within.
/// @param table_name Name of the table to generate.
/// @return 0 when finished.
int Plane::generate_triangle_table (const int fov, const std::string &table_name) {
    Nibble nb;
    SQLite::Transaction initial_transaction(*nb.db);
    
    nb.create_table(table_name, "hr_a INT, hr_b INT, hr_c INT, a FLOAT, i FLOAT");
    initial_transaction.commit();
    nb.select_table(table_name);
    
    // (i, j, k) are distinct, where no (i, j, k) = (j, k, i), (j, i, k), ....
    Star::list all_stars = nb.all_bsc5_stars();
    for (unsigned int i = 0; i < all_stars.size() - 2; i++) {
        SQLite::Transaction transaction(*nb.db);
        std::cout << "\r" << "Current *A* Star: " << all_stars[i].get_hr();
        for (unsigned int j = i + 1; j < all_stars.size() - 1; j++) {
            for (unsigned int k = j + 1; k < all_stars.size(); k++) {
                
                // Only insert if the angle between all stars are separated by fov degrees or less.
                if (Star::angle_between(all_stars[i], all_stars[j]) < fov
                    && Star::angle_between(all_stars[j], all_stars[k]) < fov
                    && Star::angle_between(all_stars[k], all_stars[i]) < fov) {
                    double a_t = Trio::planar_area(all_stars[i], all_stars[j], all_stars[k]);
                    double i_t = Trio::planar_moment(all_stars[i], all_stars[j], all_stars[k]);
                    
                    nb.insert_into_table("hr_a, hr_b, hr_c, a, i",
                                         {(double) all_stars[i].get_hr(), (double) all_stars[j].get_hr(),
                                             (double) all_stars[k].get_hr(), a_t, i_t});
                }
            }
        }
        // Commit every star I change.
        transaction.commit();
    }
    
    // Create an index for area searches. We aren't searching for polar moments.
    return nb.polish_table("a");
}

/// Given a trio of body stars, find matching trios of inertial stars using their respective planar areas and polar
/// moments.
///
/// @param hr_b Index trio of stars in body (B) frame.
/// @return 1D vector of a trio of Star(0, 0, 0) if stars are not within the fov or if no matches currently exist.
/// Otherwise, vector of trios whose areas and moments are close.
std::vector<Trio::stars> Plane::match_stars (const index_trio &hr_b) {
    std::vector<hr_trio> match_hr;
    std::vector<Trio::stars> matched_stars;
    Trio::stars b_stars{this->input[hr_b[0]], this->input[hr_b[1]], this->input[hr_b[2]]};
    
    // Do not attempt to find mathes if all stars are not within fov.
    if (Star::angle_between(b_stars[0], b_stars[1]) > this->fov
        || Star::angle_between(b_stars[1], b_stars[2]) > this->fov
        || Star::angle_between(b_stars[2], b_stars[0]) > this->fov) {
        return {{Star::zero(), Star::zero(), Star::zero()}};
    }
    
    // Search for the current trio. If this is empty, then break early.
    match_hr = this->query_for_trio(Trio::planar_area(b_stars[0], b_stars[1], b_stars[2]),
                                    Trio::planar_moment(b_stars[0], b_stars[1], b_stars[2]));
    if (match_hr[0][0] == -1 && match_hr[0][1] == -1 && match_hr[0][2] == -1) {
        return {{Star::zero(), Star::zero(), Star::zero()}};
    }
    
    // Grab stars themselves from HR numbers found in matches. Return these matches.
    matched_stars.reserve(match_hr.size());
    for (const hr_trio &t : match_hr) {
        matched_stars.push_back({ch.query_bsc5((int) t[0]), ch.query_bsc5((int) t[1]), ch.query_bsc5((int) t[2])});
    }
    
    return matched_stars;
}