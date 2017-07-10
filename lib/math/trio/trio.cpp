/*
 * @file: trio.cpp
 *
 * @brief: Source file for Trio class, which retrieves attributes based off of three Stars.
 */

#include "trio.h"

/*
 * Private constructor. Sets the individual stars.
 *
 * @param a Star A of the trio.
 * @param b Star B of the trio.
 * @param c Star C of the trio.
 */
Trio::Trio(const Star &a, const Star &b, const Star &c) {
    this->a = a;
    this->b = b;
    this->c = c;
}

/*
 * Compute the planar side lengths of the triangle.
 *
 * @return Side lengths in order ab_bar, bc_bar, ca_bar.
 */
std::array<double, 3> Trio::planar_lengths() const {
    double ab_bar = (this->a - this->b).norm();
    double bc_bar = (this->b - this->c).norm();
    double ca_bar = (this->c - this->a).norm();

//    std::array<double, 3> ells = {ab_bar, bc_bar, ca_bar};
    return {ab_bar, bc_bar, ca_bar};
}

/*
 * Compute the spherical side lengths (angular separation) of the triangle.
 *
 * @return Side lengths in order ab_bar, bc_bar, ca_bar.
 */
std::array<double, 3> Trio::spherical_lengths() const {
    double ab_bar = Star::angle_between(this->a, this->b);
    double bc_bar = Star::angle_between(this->b, this->c);
    double ca_bar = Star::angle_between(this->c, this->a);

//    std::array<double, 3> ells = {ab_bar, bc_bar, ca_bar};
    return {ab_bar, bc_bar, ca_bar};
}

/*
 * Find the triangle's semi perimeter (perimeter * 0.5) given the side lengths.
 *
 * @param ab_bar Length from star A to B.
 * @param bc_bar Length from star B to C.
 * @param ca_bar Length from star C to A.
 * @return The semi perimeter.
 */
double Trio::semi_perimeter(const double ab_bar, const double bc_bar, const double ca_bar) {
    return 0.5 * (ab_bar + bc_bar + ca_bar);
}

/*
 * Treat the space between the trio as a planar triangle. Find the area of this triangle using
 * Heron's formula.
 *
 * @param a Star A of the trio.
 * @param b Star B of the trio.
 * @param c Star C of the trio.
 * @return The planar area.
 */
double Trio::planar_area(const Star &a, const Star &b, const Star &c) {
    std::array<double, 3> tau = Trio(a, b, c).planar_lengths();
    double s = Trio::semi_perimeter(tau[0], tau[1], tau[2]);

    return sqrt(s * (s - tau[0]) * (s - tau[1]) * (s - tau[2]));
}

/*
 * Treat the space between the trio as a planar triangle. Find the polar moment of this triangle.
 *
 * @param a Star A of the trio.
 * @param b Star B of the trio.
 * @param c Star C of the trio.
 * @return The planar polar moment.
 */
double Trio::planar_moment(const Star &a, const Star &b, const Star &c) {
    std::array<double, 3> tau = Trio(a, b, c).planar_lengths();

    // square the current side lengths
    std::array<double, 3> tau_hat = {tau[0] * tau[0], tau[1] * tau[1], tau[2] * tau[2]};

    return Trio::planar_area(a, b, c) * (tau_hat[0] + tau_hat[1] + tau_hat[2]) * (1 / 36.0);
}

/*
 * The three stars are connected with great arcs facing inward (modeling extended sphere of
 * Earth), forming a spherical triangle. Find the surface area of this.
 *
 * @param a Star A of the trio.
 * @param b Star B of the trio.
 * @param c Star C of the trio.
 * @return The spherical area.
 */
double Trio::spherical_area(const Star &a, const Star &b, const Star &c) {
    std::array<double, 3> tau = Trio(a, b, c).spherical_lengths();
    double s = Trio::semi_perimeter(tau[0], tau[1], tau[2]);
    double deg2rad = M_PI / 180.0;

    // determine inner component of square root
    double iota = tan(deg2rad * s / 2.0) * tan(deg2rad * (s - tau[0]) / 2.0);
    iota *= tan(deg2rad * (s - tau[1]) / 2.0) * tan(deg2rad * (s - tau[2]) / 2.0);

    // convert back to degrees
    return 4.0 * atan(sqrt(iota)) * (1.0 / deg2rad);
}

/*
 * Determine the centroid of a **planar** triangle formed by the given three stars. It's use is
 * appropriate for the spherical triangles as we only require the angle between these calculated
 * centroids.
 *
 * @param a Star A of the trio.
 * @param b Star B of the trio.
 * @param c Star C of the trio.
 * @return Star with the components of ABC centroid.
 */
Star Trio::planar_centroid() const {
    std::array<double, 3> a_tau = this->a.components_as_array();
    std::array<double, 3> b_tau = this->b.components_as_array();
    std::array<double, 3> c_tau = this->c.components_as_array();

    double i_c = (1 / 3.0) * (a_tau[0] + b_tau[0] + c_tau[0]);
    double j_c = (1 / 3.0) * (a_tau[1] + b_tau[1] + c_tau[1]);
    double k_c = (1 / 3.0) * (a_tau[2] + b_tau[2] + c_tau[2]);

    return Star(i_c, j_c, k_c);
}

/*
 * Cut the current triangle into one smaller, retaining the focus coordinate and half the 
 * distance from rho & beta. If focus is equal to <0, 0, 0>, then find the middle fourth of the 
 * triangle.
 * 
 * @param rho New trio has midpoint between focus-rho edge 
 * @param beta New trio has midpoint between focus-beta edge.
 * @param eta New trio has midpoint between beta-rho edge, if focus = <0, 0, 0>
 * @param focus Star to retain with cut. Equal to eta.
 * @return Closer trio of stars.
 */
Trio Trio::cut_triangle(const Star &rho, const Star &beta, const Star &eta, const Star &focus) {
    std::array<double, 3> rho_eta = (rho + eta).components_as_array();
    std::array<double, 3> beta_eta = (beta + eta).components_as_array();
    std::array<double, 3> rho_beta = (rho + beta).components_as_array();

    Star b = Star(rho_eta[0] / 2.0, rho_eta[1] / 2.0, rho_eta[2] / 2.0);
    Star c = Star(beta_eta[0] / 2.0, beta_eta[1] / 2.0, beta_eta[2] / 2.0);

    // if focus is <0, 0, 0>, then find the middle triangle
    Star a = Star(0, 0, 0);
    if (!Star::is_equal(focus, a)) {
        a = focus;
    } else {
        a = Star(rho_beta[0] / 2.0, rho_beta[1] / 2.0, rho_beta[2] / 2.0);
    }

    return Trio(a, b, c);
}

/*
 * Recursively determine the spherical moment of a trio. This is a divide-and-conquer approach,
 * creating four smaller triangles with every depth increase. Upon reaching our base case, we
 * return the area of this new triangle 'dA' multiplied with the square of the angle between the
 * root trio's centroid and the current centroid = theta^2.
 *
 * @param kappa Centroid of the h=0 trio.
 * @param h_k Maximum depth of moment calculation process. Larger h_k = more accurate, slower.
 * @param h_n Current depth of moment calculation.
 * @return The spherical polar moment.
 */
double Trio::recurse_spherical_moment(const Star &kappa, const int h_k, const int h_n) {
    if (h_k == h_n) {
        double theta = Star::angle_between(kappa, this->planar_centroid());
        return Trio::spherical_area(this->a, this->b, this->c) * theta * theta;
    } else {
        // divide the triangle into four equal parts
        Trio mu_a = Trio::cut_triangle(this->a, this->b, this->c, this->a);
        Trio mu_b = Trio::cut_triangle(this->a, this->b, this->c, this->b);
        Trio mu_c = Trio::cut_triangle(this->a, this->b, this->c, this->c);
        Trio mu_k = Trio::cut_triangle(this->a, this->b, this->c);

        return mu_a.recurse_spherical_moment(kappa, h_k, h_n + 1) +
               mu_b.recurse_spherical_moment(kappa, h_k, h_n + 1) +
               mu_c.recurse_spherical_moment(kappa, h_k, h_n + 1) +
               mu_k.recurse_spherical_moment(kappa, h_k, h_n + 1);
    }
}


/*
 * The three stars are connected with great arcs facing inward (modeling extended sphere of
 * Earth), forming a spherical triangle. Find the polar moment of this. This is a wrapper for the
 * recurse_spherical_moment method.
 *
 * @param a Star A of the trio.
 * @param b Star B of the trio.
 * @param c Star C of the trio.
 * @param h_k Maximum depth of moment calculation process. Larger h_k = more accurate, slower.
 * @return The spherical polar moment.
 */
double Trio::spherical_moment(const Star &a, const Star &b, const Star &c, const int h_k) {
    Trio mu(a, b, c);

    // we start at root, h = 0
    return mu.recurse_spherical_moment(mu.planar_centroid(), h_k, 0);
}

