/*
 * @file: rotation.cpp
 *
 * @brief: Source file for Rotation class, which represents rotations on 3D star vectors using
 * quaternions.
 */

#include "rotation.h"

/*
 * Private constructor. Sets the individual components.
 *
 * @param w The scalar component of the quaternion.
 * @param gamma The vector component of the quaternion, represented using a Star object.
 * @param as_unit Flag to normalize the quaternion.
 */
Rotation::Rotation(const double w, const Star &gamma, const bool as_unit) {
    std::array<double, 3> gamma_components = gamma.components_as_array();

    if (as_unit) {
        double ell = sqrt(pow(w, 2) + pow(gamma_components[0], 2) +
                          pow(gamma_components[1], 2) + pow(gamma_components[2], 2));

        this->w = w / ell;
        this->x = gamma_components[0] / ell;
        this->y = gamma_components[1] / ell;
        this->z = gamma_components[2] / ell;
        this->gamma = Star(x, y, z, gamma.get_bsc_id());
    } else {
        this->w = w;
        this->x = gamma_components[0];
        this->y = gamma_components[1];
        this->z = gamma_components[2];
        this->gamma = gamma;
    }
}

/*
 * Given a rotation matrix as an array of stars, return the quaternion equivalent. Solution found
 * here: http://www.euclideanspac.com/maths/geometry/rotations/conversions/matrixToQuaternion/
 *
 * @param psi Array of stars to represent as a quaternion.
 * @return Quaternion equivalent of psi.
 */
Rotation Rotation::matrix_to_quaternion(const std::array<Star, 3> &psi) {
    std::array<double, 3> psi_0 = psi[0].components_as_array();
    std::array<double, 3> psi_1 = psi[1].components_as_array();
    std::array<double, 3> psi_2 = psi[2].components_as_array();

    double w = 0.5 * sqrt(psi_0[0] + psi_1[1] + psi_2[2] + 1);

    // result is normalized
    return Rotation(w, Star((psi_2[1] - psi_1[2]) / (4 * w),
                            (psi_0[2] - psi_2[0]) / (4 * w),
                            (psi_1[0] - psi_0[1]) / (4 * w)), true);
}

/*
 * Given two 3x3 matrices chi and xi, determine chi * xi^T (the transpose of xi). Matrices are
 * represented as such in the given input:
 *
 * [Star a, Star b, Star c] -----> |a.i, a.j, a.k|
 *                                 |b.i, b.j, b.k|
 *                                 |c.i, c.j, c.k|
 *
 * @param chi 3x3 matrix to multiply with xi^T.
 * @param xi 3x3 matrix to transpose and multiply with chi.
 * @return The result of chi * xi^T.
 */
std::array<Star, 3> Rotation::matrix_multiply_transpose(const std::array<Star, 3> &chi,
                                                        const std::array<Star, 3> &xi) {
    return {Star(Star::dot(chi[0], xi[0]), Star::dot(chi[0], xi[1]), Star::dot(chi[0], xi[2])),
            Star(Star::dot(chi[1], xi[0]), Star::dot(chi[1], xi[1]), Star::dot(chi[1], xi[2])),
            Star(Star::dot(chi[2], xi[0]), Star::dot(chi[2], xi[1]), Star::dot(chi[2], xi[2]))};
}

/*
 * Find the quaternion across two different frames given pairs of vectors in each frame.
 * Solution found here: https://en.wikipedia.org/wiki/Triad_method
 *
 * @param rho Pair of stars in frame A.
 * @param beta Pair of stars in frame B.
 * @return The quaternion to rotate from frame A (rho) to B (beta).
 */
Rotation Rotation::rotation_across_frames(const std::array<Star, 2> &rho,
                                          const std::array<Star, 2> &beta) {
    // compute triads, parse them into individual components
    Star zeta_1 = (Star::cross(rho[0].as_unit(), rho[1].as_unit())).as_unit();
    Star eta_1 = (Star::cross(beta[0].as_unit(), beta[1].as_unit())).as_unit();
    Star zeta_2 = (Star::cross(rho[0].as_unit(), zeta_1.as_unit())).as_unit();
    Star eta_2 = (Star::cross(beta[0].as_unit(), eta_1.as_unit())).as_unit();

    std::array<double, 3> zeta_a = rho[0].as_unit().components_as_array();
    std::array<double, 3> zeta_b = zeta_1.components_as_array();
    std::array<double, 3> zeta_c = zeta_2.components_as_array();
    std::array<double, 3> eta_a = beta[0].as_unit().components_as_array();
    std::array<double, 3> eta_b = eta_1.components_as_array();
    std::array<double, 3> eta_c = eta_2.components_as_array();

    // each vector represents a column -> [zeta_a : zeta_b : zeta_c]
    std::array<Star, 3> triad_zeta = {Star(zeta_a[0], zeta_b[0], zeta_c[0]),
                                      Star(zeta_a[1], zeta_b[1], zeta_c[1]),
                                      Star(zeta_a[2], zeta_b[2], zeta_c[2])};
    std::array<Star, 3> triad_eta = {Star(eta_a[0], eta_b[0], eta_c[0]),
                                     Star(eta_a[1], eta_b[1], eta_c[1]),
                                     Star(eta_a[2], eta_b[2], eta_c[2])};

    // multiply zeta with eta^T to find resulting rotation, return result as quaternion
    return matrix_to_quaternion(Rotation::matrix_multiply_transpose(triad_zeta, triad_eta));
}

/*
 * Rotate the current vector by the given quaternion. Converts the quaternion into a rotation
 * matrix to multiply with the column vector rho.
 *
 * Alternate solution exists here that sticks to quaternions, but I believe the same amount
 * of operations are performed: https://math.stackexchange.com/a/535223
 *
 * @param rho Star to rotate with phi.
 * @param phi Rotation to use with rho.
 * @return The rotated star rho.
 */
Star Rotation::rotate(const Star &rho, const Rotation &phi) {
    Star i_bar(pow(phi.w, 2) + pow(phi.x, 2) - pow(phi.y, 2) - pow(phi.z, 2),
               2.0 * (phi.x * phi.y - phi.w * phi.z),
               2.0 * (phi.x * phi.z + phi.w * phi.y));
    Star j_bar(2.0 * (phi.y * phi.x + phi.w * phi.z),
               pow(phi.w, 2) - pow(phi.x, 2) + pow(phi.y, 2) - pow(phi.z, 2),
               2.0 * (phi.y * phi.z - phi.w * phi.x));
    Star k_bar(2.0 * (phi.z * phi.x - phi.w * phi.y),
               2.0 * (phi.z * phi.y + phi.w * phi.x),
               pow(phi.w, 2) - pow(phi.x, 2) - pow(phi.y, 2) + pow(phi.z, 2));

    // form the rotation matrix, dot with given star
    double i_hat = Star::dot(rho, i_bar);
    double j_hat = Star::dot(rho, j_bar);
    double k_hat = Star::dot(rho, k_bar);
    return Star(i_hat, j_hat, k_hat, rho.get_bsc_id());
}

/*
 * Return the identity quaternion, as a Rotation object.
 *
 * @return Identity quaternion.
 */
Rotation Rotation::identity() {
    return Rotation(1, Star(0, 0, 0));
}

/*
 * Return a unit quaternion with random components. Achieved by find the rotation from one random
 * star to another random star.
 *
 * @return Random quaternion.
 */
Rotation Rotation::chance() {
    Star rho = Star::chance(), beta = Star::chance();
    return Rotation(sqrt(1.0 + Star::dot(rho, beta)), Star::cross(rho, beta), true);
}
