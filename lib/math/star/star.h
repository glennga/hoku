/*
 * @file: star.h
 *
 * @brief: Header file for Star class, which represents three-dimensional star vectors.
 */

#ifndef HOKU_STAR_H
#define HOKU_STAR_H

#define _USE_MATH_DEFINES

#include <array>
#include <random>

/*
 * Default precision of is_equal method.
 */
const double DEFAULT_PRECISION_IS_EQUAL = 0.000000000001;


/*
 * @class Star
 * @brief Star class, which is represented by 3-dimensional vectors and an identification number.
 *
 * The star class is really a 3D vector class in disguise, with methods focusing toward rotation
 * and angular separation. This class is the basis for all of the Hoku research.
 */
class Star {
    public:
        // constructor, set components and bsc_id, has unit flag
        Star(const double, const double, const double, const int = 0, const bool = false);
        Star();

        // get methods for i, j, k, and bsc_id methods
        double operator[](const int) const;
        int get_bsc_id() const;

        // add and subtract two vector
        Star operator+(const Star &) const;
        Star operator-(const Star &) const;

        // scale vector with a constant
        Star operator*(const double) const;

        // calculate norm of the current vector
        double norm() const;

        // return the current vector as one with norm = 1.0
        Star as_unit() const;

        // determine if the **components** are within a certain value of each other
        static bool is_equal(const Star &, const Star &,
                             const double = DEFAULT_PRECISION_IS_EQUAL);
        bool operator==(const Star &) const;

        // generate unit vector with random components, overloaded to add own BSC ID
        static Star chance();
        static Star chance(const int);

        // determine dot/cross product of two vectors
        static double dot(const Star &, const Star &);
        static Star cross(const Star &, const Star &);

        // find angle between two vectors, determine if two stars are within a certain angle
        static double angle_between(const Star &, const Star &);
        static bool within_angle(const Star &, const Star &, const double);

        // reset the BSC ID of a star to 0
        static Star without_bsc(const Star &);

#ifndef DEBUGGING_MODE_IS_ON
    private:
#endif
        // individual components of 3D vector
        double i;
        double j;
        double k;

        // BSC5 identification number
        int bsc_id;
};


#endif /* HOKU_STAR_H */
