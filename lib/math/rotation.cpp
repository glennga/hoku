/// @file rotation.cpp
/// @author Glenn Galvizo
///
/// Source file for Rotation class, which represents rotations on 3D star vectors using quaternions.

#define _USE_MATH_DEFINES
#include <cmath>
#include <iomanip>
#include "third-party/Eigen/Dense"

#include "math/random-draw.h"
#include "math/rotation.h"

/// Constructor. This is just a wrapper for the base Quaternion constructor.
///
/// @param w W component of the quaternion to set.
/// @param i I component of the quaternion to set.
/// @param j J component of the quaternion to set.
/// @param k K component of the quaternion to set.
Rotation::Rotation (const double w, const double i, const double j, const double k) : Quaternion(i, j, k, w) {
    
}

/// Wrap the given GMath quaternion inside a Rotation object.
///
/// @param q GMath quaternion to wrap.
Rotation Rotation::wrap (const Quaternion &q) {
    return Rotation(q.data[3], q.data[0], q.data[1], q.data[2]);
}

/// Place all components of R into the given stream.
///
/// @param os Working stream to place star components into.
/// @param q Rotation to place into stream.
/// @return The stream we were just passed.
std::ostream &operator<< (std::ostream &os, const Rotation &q) {
    os << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed << "(" << q.data[3] << ":"
       << q.data[0] << ":" << q.data[1] << ":" << q.data[2] << ")";
    return os;
}

/// Rotate the current vector by the given quaternion.
///
/// @param s Star to rotate with rotation q.
/// @param q Quaternion to use with star S.
/// @return The star S rotated by q.
Star Rotation::rotate (const Star &s, const Rotation &q) {
    return Star::wrap(q * s, s.get_label(), s.get_magnitude());
}

/// Rotate a star away toward another star, given the interpolation parameter.
///
/// @param s Star to push away from f.
/// @param f Star to be pushed away from.
/// @param t Interpolation parameter.
/// @return The star S "pushed away" from f.
Star Rotation::slerp (const Star &s, const Vector3 &f, const double t) {
    return Star::wrap(Vector3::SlerpUnclamped(s, f, t), s.get_label(), s.get_magnitude());
}

// TODO: This function returns the same star for noise below 1.0e-5. Not sure why.
/// Rotate the given star s in a random direction, by a random theta who's distribution is varied by the given sigma.
///
/// @param s Star to rotate randomly.
/// @param sigma Standard deviation of the angle to rotate by, in degrees.
/// @return The star S "shaken".
Star Rotation::shake (const Star &s, const double sigma) {
    Star s_end;
    
    // Require reasonably close vectors to ensure t does not tip below the machine epsilon.
    do {
        s_end = Star::chance();
    } while ((180.0 / M_PI) * Vector3::Angle(s_end, s) > 5);
    
    // Push our star in some random direction. Scale our theta to fit [0, 1] in the opposite direction.
    double t = (Vector3::Angle(s, s_end) == 0) ? 0 : ((M_PI / 180.0) * (RandomDraw::draw_normal(0, sigma))
                                                      / Vector3::Angle(s, s_end));
    return slerp(s, s_end, t);
}

/// Return the identity quaternion, as a Rotation object.
/// 
/// @return Identity quaternion: <1, 0, 0, 0>.
Rotation Rotation::identity () {
    return wrap(Quaternion::Identity());
}

/// Return a unit quaternion with random components. Achieved by find the rotation from one random star to another
/// random star.
///
/// @return Random quaternion.
Rotation Rotation::chance () {
    return wrap(Quaternion::Normalized(Quaternion::FromToRotation(Star::chance(), Star::chance())));
}

/// Approach to finds the quaternion across two different frames given pairs of vectors in each frame. This solves
/// Wahba's problem. Solution found here: https://en.wikipedia.org/wiki/Triad_method
///
/// @param v 2 element list of stars in frame V.
/// @param w 2 element list of stars in frame W.
/// @return The quaternion to rotate from frame W to V.
Rotation Rotation::triad (const Star::list &v, const Star::list &w) {
    // Compute triads. Parse them into individual components.
    Vector3 v_1 = Vector3::Normalized(v[0]), w_1 = Vector3::Normalized(w[0]);
    Vector3 v_2 = Vector3::Normalized(Vector3::Cross(v[0], v[1]));
    Vector3 w_2 = Vector3::Normalized(Vector3::Cross(w[0], w[1]));
    Vector3 v_3 = Vector3::Normalized(Vector3::Cross(v_1, v_2));
    Vector3 w_3 = Vector3::Normalized(Vector3::Cross(w_1, w_2));
    
    // Each vector represents a column -> [v_1 : v_2 : v_3]
    Matrix3x3 m_v = Matrix3x3::Transpose(Matrix3x3(v_1, v_2, v_3));
    
    // Multiple V with W^T to find resulting rotation. Return result as quaternion.
    return wrap(Matrix3x3::ToQuaternion(m_v * Matrix3x3(w_1, w_2, w_3)));
}

/// Approach to find the quaternion across two difference frames given vector observations in both. This solves
/// Wahba's problem using SVD to minimize the loss function.
///
/// @param v List of stars in frame V. The condition |v| = |w| must hold.
/// @param w List of stars in frame W. The condition |v| = |w| must hold.
/// @return The quaternion to rotate from frame W to V.
Rotation Rotation::svd (const Star::list &v, const Star::list &w) {
    // Construct the B matrix by summing the outer products (b_i (X) r_i^T). Each observation is of equal weight.
    Matrix3x3 big_b = Matrix3x3::Zero(), big_u, big_v, big_a;
    for (unsigned int i = 0; i < v.size(); i++) {
        big_b += Matrix3x3(v[i].X * w[i], v[i].Y * w[i], v[i].Z * w[i]);
    }
    
    // Perform SVD to get our U and V matrices using Eigen.
    Eigen::Matrix<double, 3, 3> big_b_e, big_u_e, big_v_e;
    big_b_e << big_b.D00, big_b.D01, big_b.D02, big_b.D10, big_b.D11, big_b.D12, big_b.D20, big_b.D21, big_b.D22;
    Eigen::JacobiSVD<Eigen::Matrix<double, 3, 3>> svd(big_b_e, Eigen::ComputeFullU | Eigen::ComputeFullV);
    big_u_e = svd.matrixU(), big_v_e = svd.matrixV();
    
    // Transform this back into GMath.
    big_u = Matrix3x3(big_u_e(0, 0), big_u_e(0, 1), big_u_e(0, 2), big_u_e(1, 0), big_u_e(1, 1), big_u_e(1, 2),
                      big_u_e(2, 0), big_u_e(2, 1), big_u_e(2, 2));
    big_v = Matrix3x3(big_v_e(0, 0), big_v_e(0, 1), big_v_e(0, 2), big_v_e(1, 0), big_v_e(1, 1), big_v_e(1, 2),
                      big_v_e(2, 0), big_v_e(2, 1), big_v_e(2, 2));
    
    // Compute the optimal rotation matrix. Return this as a quaternion.
    double det_uv = Matrix3x3::Determinate(big_u) * Matrix3x3::Determinate(big_v);
    big_a = (big_u * Matrix3x3(Vector3::Right(), Vector3::Up(), Vector3(0, 0, det_uv))) * Matrix3x3::Transpose(big_v);
    return Rotation::wrap(Matrix3x3::ToQuaternion(big_a));
}

/// Approach to find the quaternion across two different frames given vector observations in both. This
/// solves Wahba's problem through minimizing the loss function (or, maximizing the gain function) and determining
/// the eigenvector (representing a quaternion) associated with this set of eigenvalues.
///
/// @param v 2 element list of stars in frame V.
/// @param w 2 element list of stars in frame W.
/// @return The quaternion to rotate from frame W to V.
Rotation Rotation::q_method (const Star::list &v, const Star::list &w) {
    // TODO: Determine what is wrong with this q-method implementation.
    return triad(v, w);
    
    //    // Construct the B matrix by summing the outer products (b_i (X) r_i^T). Each observation is of equal weight.
    //    Matrix3x3 big_b = Matrix3x3::Zero(), big_s_sigma;
    //    for (unsigned int i = 0; i < v.size(); i++) {
    //        big_b += Matrix3x3(v[i].X * w[i], v[i].Y * w[i], v[i].Z * w[i]);
    //    }
    //
    //    // Construct the Z vector, the trace of B, and the S matrix.
    //    Vector3 big_z(big_b.D12 - big_b.D21, big_b.D20 - big_b.D02, big_b.D01 - big_b.D10);
    //    double sigma = big_b.D00 + big_b.D11 + big_b.D22;
    //    big_s_sigma = (big_b + Matrix3x3::Transpose(big_b)) - (sigma * Matrix3x3::Identity());
    //
    //    // Construct the K matrix.
    //    Eigen::Matrix<double, 4, 4> big_k_e;
    //    big_k_e.row(0) << big_s_sigma.D00, big_s_sigma.D01, big_s_sigma.D02, big_z.X;
    //    big_k_e.row(1) << big_s_sigma.D10, big_s_sigma.D11, big_s_sigma.D12, big_z.Y;
    //    big_k_e.row(2) << big_s_sigma.D20, big_s_sigma.D21, big_s_sigma.D22, big_z.Z;
    //    big_k_e.row(3) << big_z.X, big_z.Y, big_z.Z, sigma;
    //
    //    // The eigenvector associated with the largest vector represents the optimal quaternion (maximizing gain function).
    //    Eigen::EigenSolver<Eigen::Matrix<double, 4, 4>> es(big_k_e);
    //    auto lambda_j_e = es.eigenvalues();
    //    auto q_bar_j = es.eigenvectors();
    //
    //    // Determine the index of the largest eigenvalue. Return the eigenvector associated with it.
    //    std::vector<double> lambda_j = {lambda_j_e(0).real(), lambda_j_e(1).real(), lambda_j_e(2).real(),
    //        lambda_j_e(3).real()};
    //    auto i = std::distance(lambda_j.begin(), std::max_element(lambda_j.begin(), lambda_j.end()));
    //
    //    return Rotation(q_bar_j(i, 1).real(), q_bar_j(i, 2).real(), q_bar_j(i, 3).real(), q_bar_j(i, 0).real());
}