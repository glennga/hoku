/// @file rotation.cpp
/// @author Glenn Galvizo
///
/// Source file for Rotation class, which represents rotations on 3D star vectors using quaternions.

#define _USE_MATH_DEFINES
#include <cmath>
#include <iomanip>
#include "third-party/gmath/Matrix3x3.hpp"

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

/// Rotate the given star s in a random direction, by a random theta who's distribution is varied by the given sigma.
///
/// @param s Star to rotate randomly.
/// @param sigma Standard deviation of the angle to rotate by, in degrees.
/// @return The star S "shaken".
Star Rotation::shake (const Star &s, const double sigma) {
    Star s_end = Star::chance();
    
    // Push our star in some random direction. Scale our theta to fit [0, 1] in the opposite direction.
    double t = (Vector3::Angle(s, s_end) == 0) ? 0 : (180.0 / M_PI)
                                                     * (RandomDraw::draw_normal(0, sigma) / Vector3::Angle(s, s_end));
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

/// Deterministic method that finds the quaternion across two different frames given pairs of vectors in each frame.
/// This solves Wahba's problem. Solution found here: https://en.wikipedia.org/wiki/Triad_method
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

/// Statistical approach to find the quaternion across two different frames given vector observations in both. This
/// solves Wahba's problem through least squares minimization of the loss function.
///
/// @param v 2 element list of stars in frame V.
/// @param w 2 element list of stars in frame W.
/// @return The quaternion to rotate from frame W to V.
Rotation Rotation::q_exact (const Star::list &v, const Star::list &w) {
    // TODO: Finish the q-Method.
    return Rotation::triad(v, w);
}

/// Statistical approach to find the quaternion across two different frames given vector observations in both. This
/// solves Wahba's problem through ....
///
/// @param v 2 element list of stars in frame V.
/// @param w 2 element list of stars in frame W.
/// @return The quaternion to rotate from frame W to V.
Rotation Rotation::quest (const Star::list &v, const Star::list &w) {
    // TODO: Finish the QUEST Method.
    return Rotation::triad(v, w);
}