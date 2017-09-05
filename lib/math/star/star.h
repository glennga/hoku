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
/// Star s_1, s_2 = Star::chance(), s_3 = Star::zero(), s_4(-10, 10, 4, 0, true);
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
    Star (const double, const double, const double, const int = 0, const bool = false);
    Star ();
    
    std::string str () const;
    
    double operator[] (const unsigned int) const;
    int get_hr () const;
    
    Star operator+ (const Star &) const;
    Star operator- (const Star &) const;
    
    Star operator* (const double) const;
    
    double norm () const;
    
    Star as_unit () const;
    
    static bool is_equal (const Star &, const Star &, const double = STAR_EQUALITY_PRECISION_DEFAULT);
    bool operator== (const Star &) const;
    
    static Star zero ();
    
    static Star chance ();
    static Star chance (const int);
    
    static double dot (const Star &, const Star &);
    static Star cross (const Star &, const Star &);
    
    static double angle_between (const Star &, const Star &);
    static bool within_angle (const Star &, const Star &, const double);
    static bool within_angle (const list &, const double);
    
    static Star reset_hr (const Star &);
  
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
