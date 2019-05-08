/// @file star.cpp
/// @author Glenn Galvizo
///
/// Source file for Star class, which represents three-dimensional star vectors.

#define _USE_MATH_DEFINES

#include <iomanip>
#include <math/star.h>


#include "math/star.h"
#include "math/random-draw.h"

const int Star::NO_LABEL = 0;
const double Star::NO_MAGNITUDE = -30.0;

/// Constructor. Sets the i, j, and k components, as well as the catalog ID and the magnitude of the star.
Star::Star (const double i, const double j, const double k, const int label, const double m) : Vector3(i, j, k) {
    this->label = label, this->m = m;
}
[[deprecated]] Star::Star () : Vector3(0, 0, 0) { this->m = this->label = 0; }

/// Wrap the given GMath vector with a label and magnitude (inside a Star object).
Star Star::wrap (Vector3 v, int label, double m) { return {v.data[0], v.data[1], v.data[2], label, m}; }

std::ostream &operator<< (std::ostream &os, const Star &s) {
    os << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed << "(" << s.data[0] << ":"
       << s.data[1] << ":" << s.data[2] << ":" << s.label << ":" << s.m << ")";
    return os;
}
double Star::operator[] (const unsigned int n) const { return data[n]; }

int Star::get_label () const { return this->label; }
double Star::get_magnitude () const { return this->m; }
Vector3 Star::get_vector () const { return {this->data[0], this->data[1], this->data[2]}; }

/// Generate a random star with normalized components. Using C++11 random functions.
Star Star::chance () {
    double a[] = {RandomDraw::draw_real(-1.0, 1.0), RandomDraw::draw_real(-1.0, 1.0), RandomDraw::draw_real(-1.0, 1.0)};
    return wrap(Vector3::Normalized(Vector3(a[0], a[1], a[2])), NO_LABEL, NO_MAGNITUDE);
}

/// Generate a random star with normalized components. Using C++11 random functions. Instead of assigning a catalog ID
/// of 0, the user can assign one of their own.
Star Star::chance (const int label) {
    Star s = chance();
    s.label = label;
    return s;
}

/// Determines if the angle between rho and beta is within theta degrees.
bool Star::within_angle (const Vector3 &s_1, const Vector3 &s_2, const double theta) {
    return Vector3::Angle(s_1, s_2) * (180.0 / M_PI) < theta;
}

/// Overloaded function. Determines if the angle between all given stars are within theta degrees.
bool Star::within_angle (const list &s_l, const double theta) {
    // For single element lists (or blank lists), this defaults to true.
    if (s_l.size() < 2) return true;

    // Fancy for loop wrapping... (: All distinct combination pairs of s_l.
    for (unsigned int i = 0, j = 1; i < s_l.size() - 1; j = (j < s_l.size() - 1) ? j + 1 : 1 + ++i) {
        if (!within_angle(s_l[i], s_l[j], theta)) {
            return false;
        }
    }

    return true;
}

Star Star::define_label (const Star &s, int label) { return {s.data[0], s.data[1], s.data[2], label, s.m}; }
Star Star::reset_label (const Star &s) { return define_label(s, NO_LABEL); }
