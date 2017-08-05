/*
 * @file: mercator.cpp
 *
 * @brief: Source file for Mercator class, which represents two-dimensional projections of
 * three-dimensional unit vectors (Stars).
 */

#include "mercator.h"

/*
 * Constructor. Projects the input star by the given width and records the results.
 *
 * @param s Star to project and store.
 * @param w_n Width to project with.
 */
Mercator::Mercator(const Star &s, const double w_n) {
    project_star(s, w_n);
}

/*
 * Overloaded constructor. Does not project the given points, but stores the values as-is.
 *
 * @param x X coordinate to store.
 * @param y Y coordinate to store.
 * @param w_n Width used to project the given X, Y coordinates.
 * @param hr Harvard revised number to store.
 */
Mercator::Mercator(const double x, const double y, const double w_n, const int hr) {
    this->x = x;
    this->y = y;
    this->w_n = w_n;
    this->hr = hr;
}

/*
 * Find all the stars in the given list that are within the box defined by the current star and the
 * boundary size. The current star is the center of this box, and the size is boundary*boundary.
 *
 * @param t Input stars to reduce.
 * @param a Size of the boundary box.
 * @return All stars in t that are near the current star.
 */
Mercator::list Mercator::reduce_far_points(const Mercator::list &t, const double a) {
    Mercator::list t_within;
    t_within.reserve(t.size());

    for (const Mercator &m : t) {
        bool within_x = m.x < x + (a / 2.0) && m.x > x - (a / 2.0);
        bool within_y = m.y < y + (a / 2.0) && m.y > y - (a / 2.0);

        if (within_x && within_y) { t_within.push_back(m); }
    }

    return t_within;
}

/*
 * Check if the current point is within the corner defined boundary quadrilateral.
 * Corners are defined as such:     corners[0]------corners[1]
 *                                     -                -
 *                                  corners[2]------corners[3]
 *
 * @param corners Corners of the boundary quadrilateral.
 * @return True if within the box. False otherwise.
 */
bool Mercator::is_within_bounds(const quad &corners) const{
    bool within_left_x = this->x > corners[0].x && this->x > corners[2].x;
    bool within_right_x = this->x < corners[1].x && this->x < corners[3].x;
    bool within_top_y = this->y < corners[0].y && this->y < corners[1].y;
    bool within_bottom_y = this->y > corners[2].y && this->y > corners[3].y;

    return within_left_x && within_right_x && within_top_y && within_bottom_y;
}

/*
 * Project the input star by the given width. Converts the star into spherical coordinates, and
 * then projects this point into a square of size w_n*w_n with the center being (0, 0). Original
 * conversion found here: https://stackoverflow.com/a/14457180
 *
 * @param s Star to project.
 * @param w_n Width to project width.
 */
void Mercator::project_star(const Star &s, const double w_n) {
    double theta, phi, merc_n, r = s.norm();

    // determine longitude and latitude
    theta = asin(s[2] / r) * 180.0 / M_PI;
    phi = atan2(s[1], s[0]) * 180.0 / M_PI;

    // project onto cylinder, unravel
    merc_n = log(tan((M_PI / 4) + ((theta * M_PI / 180.0) / 2.0)));
    this->x = ((phi + 180.0) * w_n / 360.0) - w_n / 2;
    this->y = ((0.5 * w_n) - (w_n * merc_n / (2 * M_PI))) - w_n / 2;

    this->w_n = w_n;
    this->hr = s.get_hr();
}

/*
 * Access method for the coordinates, projection width, and the Harvard revised number.
 *
 * @param x Variable to store current X coordinate in.
 * @param y Variable to store current Y coordinate in.
 * @param w_n Variable to store current width projection in.
 * @param hr Variable to store current Harvard revised number in.
 */
void Mercator::present_projection(double &x, double &y, double &w_n, int &hr) const {
    x = this->x;
    y = this->y;
    w_n = this->w_n;
    hr = this->hr;
}

/*
 * Access method for the Harvard revised number.
 *
 * @return Harvard revised number for this point.
 */
int Mercator::get_hr() {
    return hr;
}

/*
 * Find the corners of a box using the current point as the center and 'a' as the width.
 *
 * @param a Width of the box.
 * @return Quad of corners, in order of top-left, top-right, bottom-left, bottom-right.
 */
Mercator::quad Mercator::find_corners(const double a) const {
    return {Mercator(x - a / 2.0, y + a / 2.0, w_n), Mercator(x + a / 2.0, y + a / 2.0, w_n),
            Mercator(x - a / 2.0, y - a / 2.0, w_n), Mercator(x + a / 2.0, y - a / 2.0, w_n)};
}