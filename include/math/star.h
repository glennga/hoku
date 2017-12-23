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
///
/// @example
/// @code{.cpp}
/// // Define stars (in order): {0, 0, 0}, {random, random, random}, {0, 0, 0}, {-0.680414, 0.680414, 0.272166}
/// std::random_device seed;
/// Star s_1, s_2 = Star::chance(seed), s_3 = Star::zero(), s_4(-10, 10, 4, 0, 0, true);
///
/// // Cross stars {-2, -1, 0} and {3, 2, 1} to produce {-1, 2, -1}.
/// printf("%s", Star::cross(Star(-2, -1, 0), Star(3, 2, 1)).str());
///
/// // Add star {1, 1, 1} to star {5, 5, 5}. Subtract result by {2, 2, 2} to get {4, 4, 4}.
/// printf("%s", (Star(5, 5, 5) + Star(1, 1, 1) - Star(2, 2, 2)).str());
///
/// // Determine angle between Star {2, 3, 5} and {5, 6, 7} to get 0.9744339542.
/// printf("%s", (Star::angle_between(Star(2, 3, 5), Star(5, 6, 7)).str());
/// @endcode
class Star {
  public:
    /// List type, defined as a vector of Stars.
    using list = std::vector<Star>;
    
    /// Pair type, defined as a 2-element array of Stars.
    using pair = std::array<Star, 2>;
    
  public:
    Star (double i, double j, double k, int label = 0, double m = -30.0, bool set_unit = false);
    Star ();
    
    std::string str () const;
    
    double operator[] (unsigned int n) const;
    int get_label () const;
    double get_magnitude () const;
    
    Star operator+ (const Star &s) const;
    Star operator- (const Star &s) const;
    Star operator* (double kappa) const;
    
    double norm () const;
    Star as_unit () const;
    
    static const double STAR_EQUALITY_PRECISION_DEFAULT;
    static bool is_equal (const Star &s_1, const Star &s_2, double epsilon = STAR_EQUALITY_PRECISION_DEFAULT);
    bool operator== (const Star &s) const;
    
    static Star zero ();
    static Star chance (std::random_device &seed);
    static Star chance (std::random_device &seed, int label);
    
    static double dot (const Star &s_1, const Star &s_2);
    static Star cross (const Star &s_1, const Star &s_2);
    
    static double angle_between (const Star &s_1, const Star &s_2);
    static bool within_angle (const Star &s_1, const Star &s_2, double theta);
    static bool within_angle (const list &s_l, double theta);
    
    static Star reset_label (const Star &s);
    
    static const double INVALID_ELEMENT_ACCESSED;
    static const int NO_LABEL;
    

#if !defined ENABLE_TESTING_ACCESS
    private:
#endif
    /// I Component (element 0) of 3D vector.
    double i;
    
    /// J component (element 1) of 3D vector.
    double j;
    
    /// K component (element 2) of 3D vector.
    double k;
    
    /// Catalog specific ID for the given star.
    int label;
    
    /// Apparent magnitude for the given star.
    double m;
};

#endif /* HOKU_STAR_H */
