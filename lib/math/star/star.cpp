/*
 * @file: star.cpp
 *
 * @brief: Source file for Star class, which represents three-dimensional star vectors.
 */

#include "star.h"

/*
 * Constructor. Sets the i, j, and k components, as well as the Harvard Revised number of the Star.
 *
 * @param i The i'th component from the observer to the star.
 * @param j The j'th component from the observer to the star.
 * @param k The k'th component from the observer to the star.
 * @param hr The Harvard Revised number of the star, found in the Yale Bright Star Catalog.
 * @param set_unit If true, normalize the star. Otherwise, directly set the i, j, and k.
 */
Star::Star(const double i, const double j, const double k, const int hr, const bool set_unit) {
    if (!set_unit) {
        this->i = i;
        this->j = j;
        this->k = k;
    } else {
        Star rho = Star(i, j, k).as_unit();
        this->i = rho.i;
        this->j = rho.j;
        this->k = rho.k;
    }
    this->hr = hr;
}

/*
 * Overloaded constructor. Sets the i, j, k, and HR of a star to 0.
 */
Star::Star() {
    this->i = 0;
    this->j = 0;
    this->k = 0;
    this->hr = 0;
}

/*
 * Return all components in the current star as a string object.
 *
 * @return String of components in form of (i/j/k/hr).
 */
std::string Star::as_string() const {
    return std::string("(" + std::to_string(i) + ":" + std::to_string(j) + ":" +
                       std::to_string(k) + ":" + std::to_string(hr) + ")");
}

/*
 * Get method for the i, j, and k components of the star. Overloads the [] operator.
 *
 * @param n Index of <i, j, k> to return.
 * @return 0 if n > 2. Otherwise component at index n of <i, j, k>.
 */
double Star::operator[](const int n) const {
    return n > 2 ? 0 : std::array<double, 3> {i, j, k}[n];
}

/*
 * Get method for Harvard Revised number of the star.
 *
 * @return Harvard Revised number of the current star.
 */
int Star::get_hr() const {
    return this->hr;
}

/*
 * Add star S to the current star. The resultant takes Star S's HR number.
 *
 * @param s Star to add the current star with.
 * @return The summation of the current star and star S.
 */
Star Star::operator+(const Star &s) const {
    return Star(this->i + s.i, this->j + s.j, this->k + s.k, s.hr);
}

/*
 * Subtract star S from the current star. This subtracts the two vector's components.
 *
 * @param s Star to subtract the current star with.
 * @return The resultant of subtracting the current star with star S.
 */
Star Star::operator-(const Star &s) const {
    return Star(this->i - s.i, this->j - s.j, this->k - s.k, s.hr);
}

/*
 * Scale the current star with the constant kappa.
 *
 * @param kappa Every component will be multiplied by this.
 * @return Resultant of the current star scaled by kappa.
 */
Star Star::operator*(const double kappa) const {
    return Star(this->i * kappa, this->j * kappa, this->k * kappa, this->hr);
}

/*
 * Find the magnitude of the current star.
 *
 * @return Magnitude of the current star.
 */
double Star::norm() const {
    return sqrt(pow(this->i, 2) + pow(this->j, 2) + pow(this->k, 2));
}

/*
 * Find the unit vector of the current star.
 *
 * @return Star with normalized components.
 */
Star Star::as_unit() const {
    double norm = this->norm();

    // vector with no length, return original star
    if (this->i + this->j + this->k == 0) {
        return *this;
    }

    return Star(this->i / norm, this->j / norm, this->k / norm, this->hr);
}

/*
 * Determine if the two values's **components** are within epsilon units of each other.
 *
 * @param s_1 Star to check s_2 against.
 * @param s_2 Star to check s_1 against.
 * @param epsilon Error that the differences must be less than.
 * @return True if all components are the same. False otherwise.
 */
bool Star::is_equal(const Star &s_1, const Star &s_2, const double epsilon) {
    double delta_i = fabs(s_1.i - s_2.i);
    double delta_j = fabs(s_1.j - s_2.j);
    double delta_k = fabs(s_1.k - s_2.k);

    return delta_i < epsilon && delta_j < epsilon && delta_k < epsilon;
}

/*
 * Determine if the two value's **components** are within delta units of each other. This function
 * is a wrapper for the is_equal function, and uses the default
 * epsilon = DEFAULT_PRECISION_IS_EQUAL.
 *
 * @param rho Star to check current star against.
 * @return True if all components are the same. False otherwise.
 */
bool Star::operator==(const Star &rho) const {
    return is_equal(rho, *this);
}

/*
 * Generate a random star with normalized components. Using C++11 random functions.
 *
 * @return Star with random, normalized components and a HR = 0.
 */
Star Star::chance() {
    // need to keep seed and engine static to avoid starting w/ same seed
    static std::random_device seed;
    static std::mt19937_64 mersenne_twister(seed());
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    return Star(dist(mersenne_twister), dist(mersenne_twister),
                dist(mersenne_twister), 0).as_unit();
}

/*
 * Generate a random star with normalized components. Using C++11 random functions. Instead of
 * assigning a HR number of 0, the user can assign one of their own.
 *
 * @return Star with random, normalized components and a HR = 0.
 */
Star Star::chance(const int hr) {
    Star s = chance();
    s.hr = hr;

    return s;
}

/*
 * Finds the dot product of s_1 and s_2 stars.
 *
 * @param s_1 Star to dot with s_2.
 * @param s_2 Star to dot with s_1.
 * @return Resultant of s_1 dot s_2.
 */
double Star::dot(const Star &s_1, const Star &s_2) {
    return s_1.i * s_2.i + s_1.j * s_2.j + s_1.k * s_2.k;
}

/*
 * Finds the cross product of s_1 and s_2 stars. The resultant HR = 0;
 *
 * @param s_1 Star to cross with s_2.
 * @param s_2 Star to cross with s_1.
 * @return Resultant of s_1 cross s_2.
 */
Star Star::cross(const Star &s_1, const Star &s_2) {
    return Star(s_1.j * s_2.k - s_1.k * s_2.j,
                s_1.k * s_2.i - s_1.i * s_2.k,
                s_1.i * s_2.j - s_1.j * s_2.i, 0);
}

/*
 * Finds the angle between stars s_1 and s_2. Range of hat(s^1) dot hat(s^2) is [-1.0, 1.0], which
 * is the domain of cos^-1. Need to check if same star before computing angle.
 *
 * @param s_1 Star to find angle from s_2.
 * @param s_2 Star to find angle from s_1.
 * @return Angle between s_1 and s_2 in degrees.
 */
double Star::angle_between(const Star &s_1, const Star &s_2) {
    return (s_1 == s_2) ? 0 : acos(dot(s_1.as_unit(), s_2.as_unit())) * (180.0 / M_PI);
}

/*
 * Determines if the angle between rho and beta is within theta degrees.
 *
 * @param s_1 Star to find angle from s_2.
 * @param s_2 Star to find angle from s_1.
 * @param theta The angle between s_1 and s_2 must be between this.
 * @return True if angle between s_1 and s_2 is less than theta. False otherwise.
 */
bool Star::within_angle(const Star &s_1, const Star &s_2, const double theta) {
    return angle_between(s_1, s_2) < theta;
}

/*
 * Return the given star with a HR number of 0.
 *
 * @param s Star to remove HR number from.
 * @return Same star with HR number equal to 0.
 */
Star Star::reset_hr(const Star &s) {
    return Star(s.i, s.j, s.k, 0);
}
