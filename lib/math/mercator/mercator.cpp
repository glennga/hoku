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
 * @param w Width to project with.
 */
Mercator::Mercator(const Star &s, const double w) {
    project_star(s, w);
}

/*
 * Overloaded constructor. Does not project the given points, but stores the values as-is.
 *
 * @param x X coordinate to store.
 * @param y Y coordinate to store.
 */
Mercator::Mercator(const double x, const double y) {
    this->x = x;
    this->y = y;
}

/*
 * Find all the stars in the given list that are within the box defined by the current star and the
 * boundary size. The current star is the center of this box, and the size is boundary*boundary.
 *
 * @param t Input stars to reduce.
 * @param a Size of the boundary box.
 * @return All stars in t that are near the current star.
 */
Mercator::mercator_list Mercator::reduce_far_stars(const mercator_list &t, const double a) {
    mercator_list t_within;
    t_within.reserve(t.size());

    for (const Mercator &m : t) {
        bool within_x = m.x < x + (a / 2.0) && m.x > x - (a / 2.0);
        bool within_y = m.y < y + (a / 2.0) && m.y > y - (a / 2.0);

        if (within_x && within_y) { t_within.push_back(m); }
    }

    return t_within;
}

/*
 * Project the input star by the given width. Converts the star into spherical coordinates, and
 * then projects this point into a square of size w*w with the center being (0, 0). Original
 * conversion found here: https://stackoverflow.com/a/14457180
 *
 * @param s Star to project.
 * @param w Width to project width.
 */
void Mercator::project_star(const Star &s, const double w) {
    double theta, phi, merc_n, r = s.norm();

    // detemine longitude and latitude
    theta = asin(s[2] / r) * 180.0 / M_PI;
    phi = atan2(s[1], s[0]) * 180.0 / M_PI;

    // project onto cylinder, unravel
    merc_n = log(tan((M_PI / 4) + ((theta * M_PI / 180.0) / 2.0)));
    this->x = ((phi + 180.0) * w / 360.0) - w / 2;
    this->y = ((0.5 * w) - (w * merc_n / (2 * M_PI))) - w / 2;

    this->w = w;
    this->hr = hr;
}