/// @file star.h
/// @author Glenn Galvizo
///
/// Header file for Star class, which represents three-dimensional star vectors. This is the basis for all of the Hoku
/// research.

#ifndef HOKU_STAR_H
#define HOKU_STAR_H

#include <array>
#include <vector>
#include <limits>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#include "third-party/gmath/Vector3.hpp"

#pragma GCC diagnostic pop

/// @brief Class for 3D vector representation of stars.
class Star : public Vector3 {
public:
    using list = std::vector<Star>;
    using pair = std::array<Star, 2>;
    using trio = std::array<Star, 3>;
    using quad = std::array<Star, 4>;

public:
    Star (double i, double j, double k, int label = NO_LABEL, double m = NO_MAGNITUDE);
    Star ();
    static Star wrap (Vector3 v, int label = NO_LABEL, double m = NO_MAGNITUDE);

    friend std::ostream &operator<< (std::ostream &os, const Star &s);
    double operator[] (unsigned int n) const;

    int get_label () const;
    double get_magnitude () const;
    Vector3 get_vector () const;

    static Star chance ();
    static Star chance (int label);

    static bool within_angle (const Vector3 &s_1, const Vector3 &s_2, double theta);
    static bool within_angle (const list &s_l, double theta);

    static Star define_label (const Star &s, int label);
    static Star reset_label (const Star &s);

    static const double NO_MAGNITUDE;
    static const int NO_LABEL;

private:
    int label;
    double m;
};

#endif /* HOKU_STAR_H */
