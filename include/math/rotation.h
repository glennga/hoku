/// @file rotation.h
/// @author Glenn Galvizo
///
/// Header file for Rotation class, which represents rotations on 3D star vectors using quaternions.

#ifndef HOKU_ROTATION_H
#define HOKU_ROTATION_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#include "third-party/gmath/Matrix3x3.hpp"
#include "third-party/gmath/Quaternion.hpp"

#pragma GCC diagnostic pop

#include "math/star.h"

/// @brief Class to represent quaternions, with a TRIAD method to solve Wahba's problem.
class Rotation : public Quaternion {
public:
    Rotation (double w, double i, double j, double k);
    static Rotation wrap (const Quaternion &q);

    /// Alias for a function that solves Wahba's problem (e.g. TRIAD, SVD, etc...).
    using wahba_function = Rotation (*) (const Star::list &, const Star::list &);

public:
    friend std::ostream &operator<< (std::ostream &os, const Rotation &q);

    static Star rotate (const Star &s, const Rotation &q);
    static Star slerp (const Star &s, const Vector3 &f, double t);
    static Star shake (const Star &s, double sigma);

    static Rotation identity ();
    static Rotation chance ();

    static Rotation triad (const Star::list &v, const Star::list &w);
};

#endif /* HOKU_ROTATION_H */
