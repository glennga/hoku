/// @file rotation.h
/// @author Glenn Galvizo
///
/// Header file for Rotation class, which represents rotations on 3D star vectors using quaternions.

#ifndef HOKU_ROTATION_H
#define HOKU_ROTATION_H

#include "star.h"

/// The rotation class uses the vector functions in the star class to form quaternions. Rotations allow us to
/// simulate a true lost-in-space condition.
class Rotation
{
    friend class TestRotation;

public:
    /// Force default constructor. All components will start at zero.
    Rotation() = default;

    static Star rotate(const Star &, const Rotation &);

    static Rotation identity();
    static Rotation chance();

    static Rotation rotation_across_frames(const Star::pair &, const Star::pair &);

private:
    /// Matrix alias, by using a 3-element array of 3D vectors.
    using matrix = std::array<Star, 3>;

private:
    Rotation(const double, const Star &, const bool = false);

    static Rotation matrix_to_quaternion(const matrix &);
    static matrix matrix_multiply_transpose(const matrix &, const matrix &);

private:
    /// W component, or the sole real component of a quaternion.
    double w = 1;

    /// I component (element 0) of quaternion. Defaults to zero (identity quaternion).
    double i = 0;

    /// J component (element 1) of quaternion. Defaults to zero (identity quaternion).
    double j = 0;

    /// K component (element 2) of quaternion. Defaults to zero (identity quaternion).
    double k = 0;
};

#endif /* HOKU_ROTATION_H */
