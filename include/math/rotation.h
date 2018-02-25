/// @file rotation.h
/// @author Glenn Galvizo
///
/// Header file for Rotation class, which represents rotations on 3D star vectors using quaternions.

#ifndef HOKU_ROTATION_H
#define HOKU_ROTATION_H

#include "math/star.h"
#include "third-party/gmath/Quaternion.hpp"

/// @brief Class to represent quaternions, with methods to solve Wahba's problem.
///
/// The rotation class contains functions to operate with quaternions and rotation matrices. Also contained here are
/// several solutions to Wahba's problem: determining an attitude given vector observations (stars) in an inertial
/// frame and a body frame.
///
/// @example
/// @code{.cpp}
/// // Rotate {1, 1, 1} by a random rotation.
/// std::cout << Star::rotate(Star(1, 1, 1), Rotation::chance()) << std::endl;
///
/// Star a = Star::chance(), b = Star::chance(), c, d;
/// Rotation e = Rotation::chance(), f;
///
/// // Star C is Star A rotated by Quaternion E. Star D is Star B rotated by Quaternion E.
/// c = Rotation::rotate(a, e);
/// d = Rotation::rotate(b, e);
///
///
/// // Result should show the same star.
/// std::cout << Rotation::rotate(a, e) << " : " Rotation::rotate(a, f) << std::endl;
/// @endcode
class Rotation : public Quaternion {
  public:
    Rotation(double w, double i, double j, double k);
    static Rotation wrap (const Quaternion &q);
    
    /// Alias for a function that solves Wahba's problem (e.g. TRIAD, QUEST, etc...).
    using wahba_function = Rotation (*) (const Star::list &, const Star::list &);
  
  public:
    friend std::ostream &operator<< (std::ostream &os, const Rotation &q);
    
    static Star rotate (const Star &s, const Rotation &q);
    static Star slerp (const Star &s, const Vector3 &f, const double t);
    static Star shake (const Star &s, double sigma);
    
    static Rotation identity ();
    static Rotation chance ();
    
    static Rotation triad (const Star::list &v, const Star::list &w);
    static Rotation q_exact (const Star::list &v, const Star::list &w);
    static Rotation quest (const Star::list &v, const Star::list &w);
};

#endif /* HOKU_ROTATION_H */
