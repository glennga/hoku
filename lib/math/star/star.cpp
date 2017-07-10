/*
 * @file: star.cpp
 *
 * @brief: Source file for Star class, which represents three-dimensional star vectors.
 */

#include "star.h"

/*
 * Constructor. Sets the i, j, and k components, as well as the bsc_id of the Star.
 *
 * @param i The i'th component from the observer to the star.
 * @param j The j'th component from the observer to the star.
 * @param k The k'th component from the observer to the star.
 * @param set_unit If true, normalize the star. Otherwise, directly set the i, j, and k.
 * @param bsc_id The ID number of the star, found in the Yale Bright Star Catalog.
 */
Star::Star(const double i, const double j, const double k, const int bsc_id,
           const bool set_unit) {
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
    this->bsc_id = bsc_id;
}

/*
 * Get method for the i, j, and k components of the star. Overloads the [] operator.
 *
 * @param n Index of <i, j, k> to return.
 * @return 0 if n > 2. Otherwise component at index n of <i, j, k>.
 */
double Star::operator[](const int n) const {
    return n > 2 ? 0 : std::array<double, 3> {i, j, k} [n];
}

/*
 * Get method for bsc_id of the star.
 *
 * @return BSC ID of the current star.
 */
int Star::get_bsc_id() const {
    return this->bsc_id;
}

/*
 * Add rho to the current star. The resultant takes rho's bsc_id.
 *
 * @param rho Star to add the current star with.
 * @return The summation of the current star and rho.
 */
Star Star::operator+(const Star &rho) const {
    double i_sigma = this->i + rho.i;
    double j_sigma = this->j + rho.j;
    double k_sigma = this->k + rho.k;
    return Star(i_sigma, j_sigma, k_sigma, rho.bsc_id);
}

/*
 * Subtract rho from the current star. This simply subtracts the two vector's components.
 *
 * @param rho Star to subtract the current star with.
 * @return The resultant of subtracting the current star with rho.
 */
Star Star::operator-(const Star &rho) const {
    double i_sigma = this->i - rho.i;
    double j_sigma = this->j - rho.j;
    double k_sigma = this->k - rho.k;
    return Star(i_sigma, j_sigma, k_sigma, rho.bsc_id);
}

/*
 * Scale the current star with alpha.
 *
 * @param alpha Every component will be multiplied by this.
 * @return Resultant of the current star scaled by alpha.
 */
Star Star::operator*(const double alpha) const {
    return Star(this->i * alpha, this->j * alpha, this->k * alpha, this->bsc_id);
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
    double i, j, k;
    double norm = this->norm();

    // vector with no length, return original star
    if (this->i + this->j + this->k == 0) {
        return *this;
    }

    i = this->i / norm;
    j = this->j / norm;
    k = this->k / norm;
    return Star(i, j, k, this->bsc_id);
}

/*
 * Determine if the two values's **components** are within delta units of each other.
 *
 * @param rho Star to check beta against.
 * @param beta Star to check rho against.
 * @param delta Error that the differences must be less than.
 * @return True if all components are the same. False otherwise.
 */
bool Star::is_equal(const Star &rho, const Star &beta, const double delta) {
    double i_delta = fabs(rho.i - beta.i);
    double j_delta = fabs(rho.j - beta.j);
    double k_delta = fabs(rho.k - beta.k);

    return i_delta < delta && j_delta < delta && k_delta < delta;
}

/*
 * Determine if the two value's **components** are within delta units of each other. This function
 * is a wrapper for the is_equal function, and uses the default delta = DEFAULT_PRECISION_IS_EQUAL.
 *
 * @param rho Star to check current star against.
 * @return True if all components are the same. False otherwise.
 */
bool Star::operator==(const Star &rho) const {
    return Star::is_equal(rho, *this);
}

/*
 * Generate a random star with normalized components. Using C++11 random functions.
 *
 * @return True if all components are the same. False otherwise.
 */
Star Star::chance() {
    // need to keep random device static to avoid starting w/ same seed
    static std::random_device rd;
    static std::mt19937_64 mt(rd());
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    return Star(dist(mt), dist(mt), dist(mt), 0).as_unit();
}

/*
 * Generate a random star with normalized components. Using C++11 random functions. Instead of
 * assigning a BSC ID of 0, the user can assign one of their own.
 *
 * @return True if all components are the same. False otherwise.
 */
Star Star::chance(const int bsc_id) {
    Star rho = Star::chance();
    rho.bsc_id = bsc_id;

    return rho;
}

/*
 * Finds the dot product of rho and beta stars.
 *
 * @param rho Star to dot with beta.
 * @param beta Star to dot with rho.
 * @return Resultant of rho dot beta.
 */
double Star::dot(const Star &rho, const Star &beta) {
    double i_hat = rho.i * beta.i;
    double j_hat = rho.j * beta.j;
    double k_hat = rho.k * beta.k;

    return i_hat + j_hat + k_hat;
}

/*
 * Finds the cross product of rho and beta stars. The resultant bsc_id = 0;
 *
 * @param rho Star to cross with beta.
 * @param beta Star to cross with rho.
 * @return Resultant of rho cross beta.
 */
Star Star::cross(const Star &rho, const Star &beta) {
    double cross_i = rho.j * beta.k - rho.k * beta.j;
    double cross_j = rho.k * beta.i - rho.i * beta.k;
    double cross_k = rho.i * beta.j - rho.j * beta.i;

    return Star(cross_i, cross_j, cross_k, 0);
}

/*
 * Finds the angle between stars rho and beta.
 *
 * @param rho Star to find angle from beta.
 * @param beta Star to find angle from rho.
 * @return Angle between rho and beta in degrees.
 */
double Star::angle_between(const Star &rho, const Star &beta) {
    double gamma = Star::dot(rho.as_unit(), beta.as_unit());
    double theta = acos(std::max(-1.0, std::min(gamma, 1.0)));
    return theta * (180.0 / M_PI);
}

/*
 * Determines if the angle between rho and beta is within theta degrees.
 *
 * @param rho Star to find angle from beta.
 * @param beta Star to find angle from rho.
 * @param theta The angle between rho and beta must be between this.
 * @return True if angle between rho and beta is less than theta. False otherwise.
 */
bool Star::within_angle(const Star &rho, const Star &beta, const double theta) {
    return Star::angle_between(rho, beta) < theta;
}

/*
 * Return the given star with a BSC ID of 0.
 *
 * @param rho Star to clean.
 * @return Same star with star ID equal to 0.
 */
Star Star::without_bsc(const Star &rho) {
    return Star(rho.i, rho.j, rho.k, 0);
}
