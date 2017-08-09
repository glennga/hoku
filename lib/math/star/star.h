/// @file star.h
/// @author Glenn Galvizo
///
/// Header file for Star class, which represents three-dimensional star vectors.

#ifndef HOKU_STAR_H
#define HOKU_STAR_H

#define _USE_MATH_DEFINES

#include <sstream>
#include <iomanip>
#include <array>
#include <random>

/// The star class is really a 3D vector class in disguise, with methods focusing toward rotation and angular
/// separation. This class is the basis for all of the Hoku research.
class Star
{
private:
    friend class TestStar;

public:
    /// List type, defined as a vector of Stars.
    using list = std::vector<Star>;

    /// Pair type, defined as a 2-element array of Stars.
    using pair = std::array<Star, 2>;

private:
    /// Precision default for is_equal and '==' methods.
    constexpr static double STAR_EQUALITY_PRECISION_DEFAULT = 0.000000000001;

public:
    Star(const double, const double, const double, const int = 0, const bool = false);
    Star();

    std::string str() const;

    double operator[](const int) const;
    int get_hr() const;

    Star operator+(const Star &) const;
    Star operator-(const Star &) const;

    Star operator*(const double) const;

    double norm() const;

    Star as_unit() const;

    static bool is_equal(const Star &, const Star &, const double = STAR_EQUALITY_PRECISION_DEFAULT);
    bool operator==(const Star &) const;

    static Star zero();

    static Star chance();
    static Star chance(const int);

    static double dot(const Star &, const Star &);
    static Star cross(const Star &, const Star &);

    static double angle_between(const Star &, const Star &);
    static bool within_angle(const Star &, const Star &, const double);

    static Star reset_hr(const Star &);

private:
    /// I Component (element 0) of 3D vector.
    double i;

    /// J component (element 1) of 3D vector.
    double j;

    /// K component (element 2) of 3D vector.
    double k;

    /// Harvard Revised number for the star.
    int hr;
};

#endif /* HOKU_STAR_H */
