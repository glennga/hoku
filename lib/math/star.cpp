/// @file star.cpp
/// @author Glenn Galvizo
///
/// Source file for Star class, which represents three-dimensional star vectors.

#define _USE_MATH_DEFINES
#include <iomanip>

#include "math/star.h"
#include "math/random-draw.h"

/// Constructor. Sets the i, j, and k components, as well as the catalog ID and the magnitude of the star.
///
/// @param i The i'th component from the observer to the star.
/// @param j The j'th component from the observer to the star.
/// @param k The k'th component from the observer to the star.
/// @param label The catalog ID of the star.
/// @param m Apparent magnitude of the given star.
Star::Star (const double i, const double j, const double k, const int label, const double m) : Vector3(i, j, k) {
    this->label = label, this->m = m;
}

/// Overloaded constructor. Sets the i, j, k, magnitude, and catalog ID of a star to 0.
[[deprecated]] Star::Star () : Vector3(0, 0, 0) {
    this->m = this->label = 0;
}

/// Wrap the given GMath vector with a label and magnitude (inside a Star object).
///
/// @param v GMath vector to wrap.
/// @param label Label to attach to vector.
/// @param m Magnitude to attach to vector.
/// @return A Star object with the components in V, and the given label and m.
Star Star::wrap (Vector3 v, int label, double m) {
    return Star(v.data[0], v.data[1], v.data[2], label, m);
}


/// Place all components of S into the given stream.
///
/// @param os Working stream to place star components into.
/// @param s Star to place into stream.
/// @return The stream we were just passed.
std::ostream &operator<< (std::ostream &os, const Star &s) {
    os << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed << "(" << s.data[0] << ":"
       << s.data[1] << ":" << s.data[2] << ":" << s.label << ":" << s.m << ")";
    return os;
}

/// Access method for the i, j, and k components of the star. Overloads the [] operator.
///
/// @param n Index of {i, j, k} to return.
/// @return INVALID_ELEMENT_ACCESSED if n > 2. Otherwise component at index n of {i, j, k}.
double Star::operator[] (const unsigned int n) const {
    return n > 2 ? INVALID_ELEMENT_ACCESSED : data[n];
}

/// Accessor method for catalog (Hipparcos) ID of the star.
///
/// @return Catalog (Hipparcos) ID of the current star.
int Star::get_label () const {
    return this->label;
}

/// Accessor method for the apparent magnitude of the star.
///
/// @return Apparent magnitude of the current star.
double Star::get_magnitude () const {
    return this->m;
}

/// Generate a random star with normalized components. Using C++11 random functions.
///
/// @return Star with random, normalized components, a catalog ID = NO_LABEL, and a m = NO_MAGNITUDE.
Star Star::chance () {
    double a[] = {RandomDraw::draw_real(-1.0, 1.0), RandomDraw::draw_real(-1.0, 1.0), RandomDraw::draw_real(-1.0, 1.0)};
    return wrap(Vector3::Normalized(Vector3(a[0], a[1], a[2])), NO_LABEL, NO_MAGNITUDE);
}

/// Generate a random star with normalized components. Using C++11 random functions. Instead of assigning a catalog ID
/// of 0, the user can assign one of their own.
///
/// @param label Catalog ID to use with the randomized star.
/// @return Star with random, normalized components and a catalog ID = label.
Star Star::chance (const int label) {
    Star s = chance();
    s.label = label;
    
    return s;
}

/// Determines if the angle between rho and beta is within theta degrees.
///
/// @param s_1 Star to find angle from s_2.
/// @param s_2 Star to find angle from s_1.
/// @param theta The angle between s_1 and s_2 must be between this.
/// @return True if angle between s_1 and s_2 is less than theta. False otherwise.
bool Star::within_angle (const Vector3 &s_1, const Vector3 &s_2, const double theta) {
    return Vector3::Angle(s_1, s_2) * (180.0 / M_PI) < theta;
}

/// Overloaded function. Determines if the angle between all given stars are within theta degrees.
///
/// @param s_l List of stars to determine angle between.
/// @param theta All stars in s_l must be theta degrees from each other.
/// @return True if angle between all stars are less than theta. False otherwise.
bool Star::within_angle (const list &s_l, const double theta) {
    // Fancy for loop wrapping... (: All distinct combination pairs of s_l.
    for (unsigned int i = 0, j = 1; i < s_l.size() - 1; j = (j < s_l.size() - 1) ? j + 1 : 1 + ++i) {
        if (!within_angle(s_l[i], s_l[j], theta)) {
            return false;
        }
    }
    
    return true;
}

/// Return the given star with a new, defined label.
///
/// @param s Star to attach a new catalog ID to.
/// @param label New label to attach to the star.
/// @return Same star with the given label.
Star Star::define_label (const Star &s, int label) {
    return Star(s.data[0], s.data[1], s.data[2], label, s.m);
}

/// Return the given star with a catalog ID of NO_LABEL.
///
/// @param s Star to remove catalog ID from.
/// @return Same star with catalog ID equal to NO_LABEL.
Star Star::reset_label (const Star &s) {
    return define_label(s, NO_LABEL);
}