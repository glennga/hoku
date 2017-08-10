/// @file rotation.cpp
/// @author Glenn Galvizo
///
/// Source file for Rotation class, which represents rotations on 3D star vectors using quaternions.

#include "rotation.h"

/// Private constructor. Sets the individual components.
///
/// @param w The scalar component of the quaternion.
/// @param v The vector component of the quaternion, represented using a Star object.
/// @param as_unit Flag to normalize the quaternion.
Rotation::Rotation(const double w, const Star &v, const bool as_unit)
{
    if (as_unit)
    {
        double norm = sqrt(pow(w, 2) + pow(v[0], 2) + pow(v[1], 2) + pow(v[2], 2));
        this->w = w / norm, this->i = v[0] / norm, this->j = v[1] / norm, this->k = v[2] / norm;
    }
    else this->w = w, this->i = v[0], this->j = v[1], this->k = v[2];
}

/// Given a rotation matrix as an array of stars, return the quaternion equivalent. Solution found here:
/// http://www.euclideanspac.com/maths/geometry/rotations/conversions/matrixToQuaternion/
///
/// @param r Array of stars to represent as a quaternion (i.e. rotation matrix R).
/// @return Quaternion equivalent of r.
Rotation Rotation::matrix_to_quaternion(const matrix &r)
{
    double w = 0.5 * sqrt(r[0][0] + r[1][1] + r[2][2] + 1);

    // Result returned is normalized. 
    return Rotation(w, Star((r[2][1] - r[1][2]) / (4 * w), (r[0][2] - r[2][0]) / (4 * w),
                            (r[1][0] - r[0][1]) / (4 * w)), true);
}

/// Given two 3x3 matrices A and B, determine the A * B^T (the transpose of B). This is mathematically equivalent
/// to the matrix C, where an element ij = A[i] dot B[j].
///
/// @param a 3x3 matrix to multiply with matrix B^T.
/// @param b 3x3 matrix to transpose and multiply with matrix A.
/// @return The result of A * B^T.
Rotation::matrix Rotation::matrix_multiply_transpose(const matrix &a, const matrix &b)
{
    return {Star(Star::dot(a[0], b[0]), Star::dot(a[0], b[1]), Star::dot(a[0], b[2])),
            Star(Star::dot(a[1], b[0]), Star::dot(a[1], b[1]), Star::dot(a[1], b[2])),
            Star(Star::dot(a[2], b[0]), Star::dot(a[2], b[1]), Star::dot(a[2], b[2]))};
}

/// Find the quaternion across two different frames given pairs of vectors in each frame. Solution found here: 
/// https://en.wikipedia.org/wiki/Triad_method
///
/// @param r Pair of stars in frame V.
/// @param b Pair of stars in frame W.
/// @return The quaternion to rotate from frame V (r set) to W (b set).
Rotation Rotation::rotation_across_frames(const Star::pair &r, const Star::pair &b)
{
    // Compute triads. Parse them into individual components.
    Star v_1 = r[0].as_unit(), w_1 = b[0].as_unit();
    Star v_2 = (Star::cross(r[0].as_unit(), r[1].as_unit())).as_unit();
    Star w_2 = (Star::cross(b[0].as_unit(), b[1].as_unit())).as_unit();
    Star v_3 = (Star::cross(r[0].as_unit(), v_2)).as_unit();
    Star w_3 = (Star::cross(b[0].as_unit(), w_2)).as_unit();

    // Each vector represents a column -> [v_1 : v_2 : v_3]
    matrix v = {Star(v_1[0], v_2[0], v_3[0]), Star(v_1[1], v_2[1], v_3[1]), Star(v_1[2], v_2[2], v_3[2])};
    matrix w = {Star(w_1[0], w_2[0], w_3[0]), Star(w_1[1], w_2[1], w_3[1]), Star(w_1[2], w_2[2], w_3[2])};

    // Multiple V with W^T to find resulting rotation. Return result as quaternion.
    return matrix_to_quaternion(matrix_multiply_transpose(v, w));
}

/// Rotate the current vector by the given quaternion. Converts the quaternion into a rotation matrix to multiply
/// with the column vector S. 
///
/// Alternate solution exists here that sticks to quaternions, but I believe the same amount of operations are 
/// performed: https://math.stackexchange.com/a/535223
///
/// @param s Star to rotate with rotation q.
/// @param q Quaternion to use with star S.
/// @return The star S rotated by q.
Star Rotation::rotate(const Star &s, const Rotation &q)
{
    Star a_1n(pow(q.w, 2) + pow(q.i, 2) - pow(q.j, 2) - pow(q.k, 2), 2.0 * (q.i * q.j - q.w * q.k),
              2.0 * (q.i * q.k + q.w * q.j));
    Star a_2n(2.0 * (q.j * q.i + q.w * q.k), pow(q.w, 2) - pow(q.i, 2) + pow(q.j, 2) - pow(q.k, 2),
              2.0 * (q.j * q.k - q.w * q.i));
    Star a_3n(2.0 * (q.k * q.i - q.w * q.j), 2.0 * (q.k * q.j + q.w * q.i), 
              pow(q.w, 2) - pow(q.i, 2) - pow(q.j, 2) + pow(q.k, 2));

    // Form the rotation matrix. Dot with given star.
    return Star(Star::dot(s, a_1n), Star::dot(s, a_2n), Star::dot(s, a_3n), s.get_hr());
}

/// Return the identity quaternion, as a Rotation object.
/// 
/// @return Identity quaternion: <1, 0, 0, 0>.
Rotation Rotation::identity()
{
    return Rotation(1, Star::zero());
}

/// Return a unit quaternion with random components. Achieved by find the rotation from one random star to another
/// random star.
///
/// @return Random quaternion.
Rotation Rotation::chance()
{
    Star p = Star::chance(), q = Star::chance();
    return Rotation(sqrt(1.0 + Star::dot(p, q)), Star::cross(p, q), true);
}
