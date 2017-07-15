/*
 * @file: rotation.h
 *
 * @brief: Header file for Rotation class, which represents rotations on 3D star vectors using
 * quaternions.
 */

#ifndef HOKU_ROTATION_H
#define HOKU_ROTATION_H

#include "star.h"

/*
 * @class Rotation
 * @brief Rotation class, which facilitates general 3D rotations.
 *
 * The rotation class uses the vector functions in the star class to form quaternions. Rotations
 * allow us to simulate a true lost-in-space condition.
 */
class Rotation {
    public:
        // force default constructor, all components start at zero
        Rotation() = default;

        // rotate a star with quaternion
        static Star rotate(const Star &, const Rotation &);

        // identity = no rotation, chance = random rotation
        static Rotation identity();
        static Rotation chance();

        // determine mapping across two frames
        static Rotation rotation_across_frames(const std::array<Star, 2> &,
                                               const std::array<Star, 2> &);

#ifndef DEBUGGING_MODE_IS_ON
        private:
#endif
        // private component setter constructor, user shouldn't deal with components directly
        Rotation(const double, const Star &, const bool = false);

        // convert a rotation matrix to a quaternion
        static Rotation matrix_to_quaternion(const std::array<Star, 3> &);

        // multiply a matrix with another matrix's transpose
        static std::array<Star, 3> matrix_multiply_transpose(const std::array<Star, 3> &,
                                                             const std::array<Star, 3> &);

        // quaternion components
        double w = 1, i = 0, j = 0, k = 0;

};

#endif /* HOKU_ROTATION_H */
