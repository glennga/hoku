/// @file mercator.h
/// @author Glenn Galvizo
///
/// Source file for Mercator class, which represents two-dimensional projections of three-dimensional unit vectors
/// (i.e. Stars).

#include "mercator.h"

/// Constructor. Projects the input star by the given width and records the results.
///
/// @param s Star to project and store.
/// @param w_n Width to project with.
Mercator::Mercator (const Star &s, const double w_n) {
    project_star(s, w_n);
}

/// Overloaded constructor. Does not project the given points, but stores the values as-is.
///
/// @param x X coordinate to store.
/// @param y Y coordinate to store.
/// @param w_n Width used to project the given X, Y coordinates.
/// @param hr Harvard revised number to store.
Mercator::Mercator (const double x, const double y, const double w_n, const int hr) {
    this->x = x, this->y = y, this->w_n = w_n, this->hr = hr;
}

/// Return a point with coordinates (0, 0) and w_n = 0.
///
/// @return (0, 0) point with w_n = 0.
Mercator Mercator::zero() {
    return Mercator(0, 0, 0);
}

/// Access method for the x and y components of the star. Overloads the [] operator.
///
/// @param n Index of {x, y to return.
/// @return 0 if n > 1. Otherwise component at index n of {x, y}.
double Mercator::operator[] (const unsigned int n) const {
    return n > 1 ? 0 : std::array<double, 2> {x, y}[n];
}

/// Determine the length of the line that connects points m_1 and m_2.
///
/// @param m_1 Point one to determine the distance from.
/// @param m_2 Point two to determine the distance from.
/// @return Distance between points m_1 and m_2.
double Mercator::distance_between (const Mercator &m_1, const Mercator &m_2) {
    return sqrt(pow(m_2.y - m_1.y, 2) + pow(m_2.x - m_1.x, 2));
}

/// Return all components in the current point as a string object.
///
/// @return String of components in form of (x:y:w_n:hr).
std::string Mercator::str () const {
    std::stringstream components;
    
    // Need to use stream here to set precision.
    components << std::setprecision(16) << std::fixed << "(";
    components << x << ":" << y << ":" << w_n << ":" << hr << ")";
    return components.str();
}

/// Check if the current point is within the corner defined boundary quadrilateral. Corners are defined as such:
///
/// @code{.cpp}
/// corners[0] ----------------------- corners[1]
///  |-----------------------------------------|
///  |-----------------------------------------|
///  |-----------------------------------------|
///  |-----------------------------------------|
///  |-----------------------------------------|
/// corners[2] ----------------------- corners[3]
/// @endcode
///
/// @param corners Corners of the boundary quadrilateral.
/// @return True if within the box. False otherwise.
bool Mercator::is_within_bounds (const quad &corners) const {
    bool within_left_x = this->x > corners[0].x && this->x > corners[2].x;
    bool within_right_x = this->x < corners[1].x && this->x < corners[3].x;
    bool within_top_y = this->y < corners[0].y && this->y < corners[1].y;
    bool within_bottom_y = this->y > corners[2].y && this->y > corners[3].y;
    
    return within_left_x && within_right_x && within_top_y && within_bottom_y;
}

/// Project the input star by the given width. Converts the star into spherical coordinates, and then projects this
/// point into a square of size w_n*w_n with the center being (0, 0). Original conversion found here:
/// https://stackoverflow.com/a/14457180
///
/// @param s Star to project.
/// @param w_n Width to project with.
void Mercator::project_star (const Star &s, const double w_n) {
    double theta, phi, r = s.norm();
    
    // Determine longitude (theta) and latitude (phi). Convert to degrees.
    theta = asin(s[2] / r) * 180.0 / M_PI;
    phi = atan2(s[1], s[0]) * 180.0 / M_PI;
    
    // Project star onto cylinder. Unravel onto square.
    this->x = ((phi + 180.0) * w_n / 360.0) - w_n / 2;
    this->y = ((0.5 * w_n) - (w_n * log(tan((M_PI / 4) + ((theta * M_PI / 180.0) / 2.0))) / (2 * M_PI))) - w_n / 2;
    
    // Save projection width. Use star's HR.
    this->w_n = w_n, this->hr = s.get_hr();
}

/// Access method for the Harvard revised number.
///
/// @return Harvard revised number for this point.
int Mercator::get_hr () const {
    return hr;
}

/// Find the corners of a box using the current point as the center and 'a' as the width.
///
/// @param a Width of the box.
/// @return Quad of corners, in order of top-left, top-right, bottom-left, bottom-right.
Mercator::quad Mercator::find_corners (const double a) const {
    return {Mercator(x - a / 2.0, y + a / 2.0, w_n), Mercator(x + a / 2.0, y + a / 2.0, w_n),
        Mercator(x - a / 2.0, y - a / 2.0, w_n), Mercator(x + a / 2.0, y - a / 2.0, w_n)};
}