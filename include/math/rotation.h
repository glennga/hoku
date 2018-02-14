/// @file rotation.h
/// @author Glenn Galvizo
///
/// Header file for Rotation class, which represents rotations on 3D star vectors using quaternions.

#ifndef HOKU_ROTATION_H
#define HOKU_ROTATION_H

#include "math/star.h"

/// @brief Class to represent quaternions, with methods to solve Wahba's problem.
///
/// The rotation class contains functions to operate with quaternions and rotation matrices. Also contained here are
/// several solutions to Wahba's problem: determining an attitude given vector observations (stars) in an inertial
/// frame and a body frame.
///
/// @example
/// @code{.cpp}
/// // Rotate {1, 1, 1} by a random rotation.
/// std::cout << Star::rotate(Star(1, 1, 1), Rotation::chance()).str() << std::endl;
///
/// Star a = Star::chance(), b = Star::chance(), c, d;
/// Rotation e = Rotation::chance(), f;
///
/// // Star C is Star A rotated by Quaternion E. Star D is Star B rotated by Quaternion E.
/// c = Rotation::rotate(a, e);
/// d = Rotation::rotate(b, e);
///
/// // F is the rotation to take AB frame to CD. F == E.
/// f = Rotation::rotation_across_frames({a, b}, {c, d});
///
/// // Result should show the same star.
/// std::cout << Rotation::rotate(a, e).str() + " : " Rotation::rotate(a, f).str() << std::endl;
/// @endcode
class Rotation {
  public:
    /// Force default constructor. Default is {1, 0, 0, 0} (identity).
    Rotation () = default;
    
    /// Alias for a function that solves Wahba's problem (e.g. TRIAD, QUEST, etc...).
    using wahba_function = Rotation (*) (const Star::list &, const Star::list &);
  
  public:
    bool operator== (const Rotation &q) const;
    Rotation operator* (const Rotation &q) const;
    
    static Star rotate (const Star &s, const Rotation &q);
    static Star push (const Star &s, const Star &f, double d);
    static Star shake (const Star &s, double sigma);
    
    static Rotation identity ();
    static Rotation chance ();
    
    static Rotation triad (const Star::list &v, const Star::list &w);
    static Rotation q_exact (const Star::list &v, const Star::list &w);
    static Rotation quest (const Star::list &v, const Star::list &w);

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// Matrix alias, by using a 3-element array of 3D vectors.
    using matrix = std::array<Star, 3>;
    
    /// Precision default for '==' method.
    static constexpr double EQUALITY_PRECISION_DEFAULT = 0.000000000001;

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    Rotation (double w, const Star &v, bool as_unit = false);
    
    static Rotation matrix_to_quaternion (const matrix &r);
    static matrix matrix_multiply_transpose (const matrix &a, const matrix &b);

#if !defined ENABLE_TESTING_ACCESS
  private:
#endif
    /// W component, or the sole real component of a quaternion. Defaults to one (identity quaternion).
    double w = 1;
    
    /// I component (element 0) of quaternion. Defaults to zero (identity quaternion).
    double i = 0;
    
    /// J component (element 1) of quaternion. Defaults to zero (identity quaternion).
    double j = 0;
    
    /// K component (element 2) of quaternion. Defaults to zero (identity quaternion).
    double k = 0;
};

#endif /* HOKU_ROTATION_H */
