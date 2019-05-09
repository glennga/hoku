/// @file rotation.cpp
/// @author Glenn Galvizo
///
/// Source file for Rotation class, which represents rotations on 3D star vectors using quaternions.

#define _USE_MATH_DEFINES

#include <cmath>
#include <iomanip>

#include "math/random-draw.h"
#include "math/rotation.h"

Rotation::Rotation (const double w, const double i, const double j, const double k) : Quaternion(i, j, k, w) { }
Rotation Rotation::wrap (const Quaternion &q) { return {q.data[3], q.data[0], q.data[1], q.data[2]}; }

std::ostream &operator<< (std::ostream &os, const Rotation &q) {
    os << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed << "(" << q.data[3] << ":"
       << q.data[0] << ":" << q.data[1] << ":" << q.data[2] << ")";
    return os;
}

Star Rotation::rotate (const Star &s, const Rotation &q) {
    return Star::wrap(q * s.get_vector(), s.get_label(), s.get_magnitude());
}

/// Rotate a star away toward another star, given the interpolation parameter.
Star Rotation::slerp (const Star &s, const Vector3 &f, const double t) {
    return Star::wrap(Vector3::SlerpUnclamped(s.get_vector(), f, t), s.get_label(), s.get_magnitude());
}

/// TODO: Does this function still return the same star for noise below 1.0e-5?
/// Rotate the given star s in a random direction, by a random theta who's distribution is varied by the given sigma.
Star Rotation::shake (const Star &s, const double sigma) {
    Star s_end;

    do {  // Require reasonably close vectors to ensure t does not tip below the machine epsilon.
        s_end = Star::chance();
    } while ((180.0 / M_PI) * Vector3::Angle(s_end.get_vector(), s.get_vector()) > 5);

    // Push our star in some random direction. Scale our theta to fit [0, 1] in the opposite direction.
    double t = (Vector3::Angle(s.get_vector(), s_end.get_vector()) == 0) ? 0 :
               ((M_PI / 180.0) * (RandomDraw::draw_normal(0, sigma))
                / Vector3::Angle(s.get_vector(), s_end.get_vector()));

    return slerp(s, s_end, t);
}

Rotation Rotation::identity () { return wrap(Quaternion::Identity()); }

/// Return a unit quaternion with random components. Achieved by find the rotation from one random star to another
/// random star.
Rotation Rotation::chance () {
    return wrap(Quaternion::Normalized(Quaternion::FromToRotation(Star::chance().get_vector(),
                                                                  Star::chance().get_vector())));
}

/// Approach to finds the quaternion across two different frames given pairs of vectors in each frame. This solves
/// Wahba's problem. Solution found here: https://en.wikipedia.org/wiki/Triad_method
///
/// v and w represent 2-element list of stars in frames V and W respectively.
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
