/// @file star.cpp
/// @author Glenn Galvizo
///
/// Source file for Star class, which represents three-dimensional star vectors.

#include "math/star.h"

/// Constructor. Sets the i, j, and k components, as well as the catalog ID of the Star.
///
/// @param i The i'th component from the observer to the star.
/// @param j The j'th component from the observer to the star.
/// @param k The k'th component from the observer to the star.
/// @param label The catalog ID of the star.
/// @param m Apparent magnitude of the given star.
/// @param set_unit If true, normalize the star. Otherwise, directly set the i, j, and k.
Star::Star (const double i, const double j, const double k, const int label, const double m, const bool set_unit) {
    if (!set_unit) {
        this->i = i, this->j = j, this->k = k;
    }
    else {
        Star s = Star(i, j, k).as_unit();
        this->i = s.i, this->j = s.j, this->k = s.k;
    }
    
    this->label = label;
    this->m = m;
}

/// Overloaded constructor. Sets the i, j, k, magnitude, and catalog ID of a star to 0.
Star::Star () {
    this->i = this->j = this->k = this->m = this->label = 0;
}

/// Return all components in the current star as a string object.
///
/// @return String of components in form of (i:j:k:m:hr).
std::string Star::str () const {
    std::stringstream components;
    
    // Need to use stream here to set precision.
    components << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed << "(" << i << ":" << j
               << ":" << k << ":" << label << ":" << m << ")";
    return components.str();
}

/// Access method for the i, j, and k components of the star. Overloads the [] operator.
///
/// @param n Index of {i, j, k} to return.
/// @return 0 if n > 2. Otherwise component at index n of {i, j, k}.
double Star::operator[] (const unsigned int n) const {
    return n > 2 ? 0 : std::array<double, 3> {i, j, k}[n];
}

/// Accessor method for catalog ID of the star.
///
/// @return Catalog ID of the current star.
int Star::get_label () const {
    return this->label;
}

/// Accessor method for the apparent magnitude of the star.
///
/// @return Apparent magnitude of the current star.
double Star::get_magnitude () const {
    return this->m;
}

/// Add star S to the current star. The resultant takes Star S's catalog ID.
///
/// @param s Star to add the current star with.
/// @return The summation of the current star and star S.
Star Star::operator+ (const Star &s) const {
    return {this->i + s.i, this->j + s.j, this->k + s.k, s.label};
}

/// Subtract star S from the current star. This subtracts the two vector's components.
///
/// @param s Star to subtract the current star with.
/// @return The resultant of subtracting the current star with star S.
Star Star::operator- (const Star &s) const {
    return {this->i - s.i, this->j - s.j, this->k - s.k, s.label};
}

/// Scale the current star with the constant kappa.
///
/// @param kappa kappa Every component will be multiplied by this.
/// @return Resultant of the current star scaled by kappa.
Star Star::operator* (const double kappa) const {
    return {this->i * kappa, this->j * kappa, this->k * kappa, this->label};
}

/// Find the magnitude of the current star (L2 norm).
///
/// @return Magnitude of the current star.
double Star::norm () const {
    return sqrt(pow(this->i, 2) + pow(this->j, 2) + pow(this->k, 2));
}

/// Find the unit vector of the current star.
///
/// @return Star with normalized components.
Star Star::as_unit () const {
    double norm = this->norm();
    
    // Vector with no length, return original star.
    if (this->i + this->j + this->k == 0) {
        return *this;
    }
    
    return {this->i / norm, this->j / norm, this->k / norm, this->label};
}

/// Determine if the two values's **components** are within epsilon units of each other.
///
/// @param s_1 Star to check s_2 against.
/// @param s_2 Star to check s_1 against.
/// @param epsilon Error that the differences must be less than.
/// @return True if all components are the same. False otherwise.
bool Star::is_equal (const Star &s_1, const Star &s_2, const double epsilon) {
    return fabs(s_1.i - s_2.i) < epsilon && fabs(s_1.j - s_2.j) < epsilon && fabs(s_1.k - s_2.k) < epsilon;
}

/// Determine if the two value's **components** are within delta units of each other. This function is a wrapper for
/// the is_equal function, and uses the default epsilon = STAR_EQUALITY_PRECISION_DEFAULT.
///
/// @param s Star to check current star against.
/// @return True if all components (besides m and hr) are the same. False otherwise.
bool Star::operator== (const Star &s) const {
    return is_equal(s, *this);
}

/// Return a star with all components set to zero.
///
/// @return Star with components {0, 0, 0} and hr = 0.
Star Star::zero () {
    return {0, 0, 0, 0};
}

/// Generate a random star with normalized components. Using C++11 random functions.
///
/// @param seed Random device to use when generating star.
/// @return Star with random, normalized components and a catalog ID = 0.
Star Star::chance (std::random_device &seed) {
    std::mt19937_64 mersenne_twister(seed());
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    
    return Star(dist(mersenne_twister), dist(mersenne_twister), dist(mersenne_twister), 0).as_unit();
}

/// Generate a random star with normalized components. Using C++11 random functions. Instead of assigning a catalog ID
/// of 0, the user can assign one of their own.
///
/// @param seed Random device to use when generating star.
/// @param label Catalog ID to use with the randomized star.
/// @return Star with random, normalized components and a catalog ID = 0.
Star Star::chance (std::random_device &seed, const int label) {
    Star s = chance(seed);
    s.label = label;
    
    return s;
}

/// Finds the dot product of s_1 and s_2 stars.
///
/// @param s_1 Star to dot with s_2.
/// @param s_2 Star to dot with s_1.
/// @return Resultant of s_1 dot s_2.
double Star::dot (const Star &s_1, const Star &s_2) {
    return s_1.i * s_2.i + s_1.j * s_2.j + s_1.k * s_2.k;
}

/// Finds the cross product of s_1 and s_2 stars. The resultant catalog ID = 0;
///
/// @param s_1 Star to cross with s_2.
/// @param s_2 Star to cross with s_1.
/// @return Resultant of s_1 cross s_2.
Star Star::cross (const Star &s_1, const Star &s_2) {
    return {s_1.j * s_2.k - s_1.k * s_2.j, s_1.k * s_2.i - s_1.i * s_2.k, s_1.i * s_2.j - s_1.j * s_2.i, 0};
}

/// Finds the angle between stars s_1 and s_2. Range of hat(s^1) dot hat(s^2) is [-1.0, 1.0], which is the
/// domain is the domain of cos^-1. Need to check if same star before computing angle.
///
/// @param s_1 Star to find angle from s_2.
/// @param s_2 Star to find angle from s_1.
/// @return Angle between s_1 and s_2 in degrees.
double Star::angle_between (const Star &s_1, const Star &s_2) {
    return (s_1 == s_2) ? 0 : acos(dot(s_1.as_unit(), s_2.as_unit())) * (180.0 / M_PI);
}

/// Determines if the angle between rho and beta is within theta degrees.
///
/// @param s_1 Star to find angle from s_2.
/// @param s_2 Star to find angle from s_1.
/// @param theta The angle between s_1 and s_2 must be between this.
/// @return True if angle between s_1 and s_2 is less than theta. False otherwise.
bool Star::within_angle (const Star &s_1, const Star &s_2, const double theta) {
    return angle_between(s_1, s_2) < theta;
}

/// Overloaded function. Determines if the angle between all given stars are within theta degrees.
///
/// @param s_l List of stars to determine angle between.
/// @param theta All stars in s_l must be theta degrees from each other.
/// @return True if angle between all stars are less than theta. False otherwise.
bool Star::within_angle (const list &s_l, const double theta) {
    // Fancy for loop wrapping... (: All distinct permutations of s_l.
    for (unsigned int i = 0, j = 1; i < s_l.size() - 1; j = (j < s_l.size() - 1) ? j + 1 : 1 + ++i) {
        if (!within_angle(s_l[i], s_l[j], theta)) {
            return false;
        }
    }
    
    return true;
}

/// Return the given star with a catalog ID of 0.
///
/// @param s Star to remove catalog ID from.
/// @return Same star with catalog ID equal to 0.
Star Star::reset_label (const Star &s) {
    return {s.i, s.j, s.k, 0};
}
