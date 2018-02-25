/// @file trio.cpp
/// @author Glenn Galvizo
///
/// Source file for Trio class, which retrieves attributes based off of three Stars.

#define _USE_MATH_DEFINES
#include <cmath>

#include "math/trio.h"

/// Private constructor. Sets the individual stars.
///
/// @param b_1 Star B_1 of the trio.
/// @param b_2 Star B_2 of the trio.
/// @param b_3 Star B_3 of the trio.
Trio::Trio (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3) {
    this->b_1 = b_1, this->b_2 = b_2, this->b_3 = b_3;
}

/// Compute the planar side lengths of the triangle.
///
/// @return Side lengths in order a, b, c.
Trio::side_lengths Trio::planar_lengths () const {
    return {Vector3::Magnitude(b_1 - b_2), Vector3::Magnitude(b_2 - b_3), Vector3::Magnitude(b_3 - b_1)};
}

/// Compute the spherical side lengths of the triangle.
///
/// @return Side lengths in order a, b, c in **radians**.
Trio::side_lengths Trio::spherical_lengths () const {
    static auto compute_length = [] (const Vector3 &beta_1, const Vector3 &beta_2) -> double {
        return acos(Vector3::Dot(beta_1, beta_2) / (Vector3::Magnitude(beta_1) * Vector3::Magnitude(beta_2)));
    };
    return {compute_length(b_1, b_2), compute_length(b_2, b_3), compute_length(b_3, b_1)};
}

/// Find the triangle's semi perimeter (perimeter * 0.5) given the side lengths.
///
/// @param a Length from star B_1 to B_2.
/// @param b Length from star B_2 to B_3.
/// @param c Length from star B_3 to B_1.
/// @return The semi perimeter of {B_1, B_2, B_3}.
double Trio::semi_perimeter (const double a, const double b, const double c) {
    return 0.5 * (a + b + c);
}

/// Treat the space between the trio as a planar triangle. Find the area of this triangle using Heron's formula.
///
/// @param b_1 Star B_1 of the trio.
/// @param b_2 Star B_2 of the trio.
/// @param b_3 Star B_3 of the trio.
/// @return The planar area of {B_1, B_2, B_3}.
double Trio::planar_area (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3) {
    side_lengths ell = Trio(b_1, b_2, b_3).planar_lengths();
    double s = semi_perimeter(ell[0], ell[1], ell[2]);
    
    return sqrt(s * (s - ell[0]) * (s - ell[1]) * (s - ell[2]));
}

/// Treat the space between the trio as a planar triangle. Find the polar moment of this triangle.
///
/// @param b_1 Star B_1 of the trio.
/// @param b_2 Star B_2 of the trio.
/// @param b_3 Star B_3 of the trio.
/// @return The planar polar moment of {B_1, B_2, B_3}.
double Trio::planar_moment (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3) {
    side_lengths ell = Trio(b_1, b_2, b_3).planar_lengths();
    return planar_area(b_1, b_2, b_3) * (pow(ell[0], 2) + pow(ell[1], 2) + pow(ell[2], 2)) / 36.0;
}

/// The three stars are connected with great arcs facing inward (modeling extended sphere of Earth), forming a
/// spherical triangle. Find the surface area of this. Based on L'Huilier's Formula and using the excess formula
/// defined in terms of the semi-perimeter.
/// https://en.wikipedia.org/wiki/Spherical_trigonometry#Area_and_spherical_excess
///
/// @param b_1 Star B_1 of the trio.
/// @param b_2 Star B_2 of the trio.
/// @param b_3 Star B_3 of the trio.
/// @return INVALID_TRIO_F if f is NaN or < 0. DUPLICATE_STARS_IN_TRIO if two stars are the same. The spherical
/// area of {B_1, B_2, B_3} otherwise.
double Trio::spherical_area (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3) {
    side_lengths ell = Trio(b_1, b_2, b_3).spherical_lengths();
    
    // If any of the stars are positioned in the same spot, this is a line. There exists no area.
    if (b_1 == b_2 || b_2 == b_3 || b_3 == b_1) {
        return DUPLICATE_STARS_IN_TRIO;
    }
    
    // Determine the inner component of the square root.
    double s = semi_perimeter(ell[0], ell[1], ell[2]);
    double f = tan(0.5 * s) * tan(0.5 * (s - ell[0]));
    f *= tan(0.5 * (s - ell[1])) * tan(0.5 * (s - ell[2]));
    
    // F should a positive number. If this is not the case, then we don't proceed.
    if (f < 0 || std::isnan(f)) {
        return INVALID_TRIO_A;
    }
    
    // Find and return the excess.
    return 4.0 * atan(sqrt(f));
}

/// Determine the centroid of a **planar** triangle formed by the given three stars. It's use is appropriate for the
/// spherical triangles as we only require the angle between these calculated centroids.
///
/// @return Star with the components of {B_1, B_2, B_3} centroid.
Vector3 Trio::planar_centroid () const {
    return {(1 / 3.0) * (this->b_1.X + this->b_2.X + this->b_3.X),
        (1 / 3.0) * (this->b_1.Y + this->b_2.Y + this->b_3.Y), (1 / 3.0) * (this->b_1.Z + this->b_2.Z + this->b_3.Z)};
}

/// Cut the current triangle into one smaller triangle (by ~ 1/4). Determine which corner to keep with k. If k = 3,
/// then find the middle fourth of the triangle.
///
/// @param c_1 Corner 1 of the triangle to cut.
/// @param c_2 Corner 2 of the triangle to cut.
/// @param c_3 Corner 3 of the triangle to cut.
/// @param k Option to cut triangle with. Must exist in space [0, 3].
/// @return Cut trio of stars {c_12, c_23, c_31} if k in space [0, 3]. Otherwise, the original trio.
Trio Trio::cut_triangle (const Vector3 &c_1, const Vector3 &c_2, const Vector3 &c_3, const int k) {
    static auto midpoint = [] (const Vector3 &s_i, const Vector3 &s_j) -> Vector3 {
        return Vector3::Normalized(Vector3::Slerp(s_i, s_j, 0.5));
    };
    
    switch (k) {
        // Case 1: Keep c_1. Cut c_1 - c_2 and c_1 - c_3.
        case 0: return {c_1, midpoint(c_1, c_2), midpoint(c_1, c_3)};
            // Case 2: Keep c_2. Cut c_1 - c_2 and c_2 - c_3.
        case 1: return {midpoint(c_1, c_2), c_2, midpoint(c_2, c_3)};
            // Case 3: Keep c_3. Cut c_1 - c_3 and c_2 - c_3.
        case 2: return {midpoint(c_1, c_3), midpoint(c_2, c_3), c_3};
            // Case 4: Keep no corners. Determine the middle triangle.
        case 3: return {midpoint(c_1, c_2), midpoint(c_1, c_3), midpoint(c_2, c_3)};
            // Return the original trio if k is invalid.
        default: return {c_1, c_2, c_3};
    }
}

/// Recursively determine the spherical moment of a trio. This is a divide-and-conquer approach, creating four
/// smaller triangles with every depth increase. Upon reaching our base case, we return the area of this new
/// triangle 'dA' multiplied with the square of the angle between the root trio's centroid and the current
/// centroid = theta^2.
///
/// @param c Centroid of the td_h=0 trio.
/// @param td_n Maximum depth of moment calculation process. Larger td_n = more accurate, slower.
/// @param td_i Current depth of moment calculation.
/// @return The spherical polar moment.
double Trio::recurse_spherical_moment (const Vector3 &c, const int td_n, const int td_i) {
    if (td_n == td_i) {
        double theta = Vector3::Angle(c, this->planar_centroid());
        return spherical_area(this->b_1, this->b_2, this->b_3) * pow(theta, 2);
    }
    else {
        // Divide the triangle into four equal parts. Recurse for each of these new triangles.
        Trio t_1 = cut_triangle(this->b_1, this->b_2, this->b_3, 0);
        Trio t_2 = cut_triangle(this->b_1, this->b_2, this->b_3, 1);
        Trio t_3 = cut_triangle(this->b_1, this->b_2, this->b_3, 2);
        Trio t_4 = cut_triangle(this->b_1, this->b_2, this->b_3, 3);
        
        return t_1.recurse_spherical_moment(c, td_n, td_i + 1) + t_2.recurse_spherical_moment(c, td_n, td_i + 1)
               + t_3.recurse_spherical_moment(c, td_n, td_i + 1) + t_4.recurse_spherical_moment(c, td_n, td_i + 1);
    }
}

/// The three stars are connected with great arcs facing inward (modeling extended sphere of Earth), forming a
/// spherical triangle. Find the polar moment of this. This is a wrapper for the recurse_spherical_moment method.
///
/// @param b_1 Star B_1 of the trio.
/// @param b_2 Star B_2 of the trio.
/// @param b_3 Star B_3 of the trio.
/// @param td_h Maximum depth of moment calculation process. Larger td_h = more accurate, slower.
/// @return INVALID_TRIO_M if the result is NaN or < 0. The spherical polar moment of {B_1, B_2, B_3}.
double Trio::spherical_moment (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3, const int td_h) {
    Trio t(b_1, b_2, b_3);
    
    // We star at the root, where the current tree depth td_i = 0.
    double t_i = t.recurse_spherical_moment(t.planar_centroid(), td_h, 0);
    
    // Don't return NaN or negative values.
    return (std::isnan(t_i) || t_i < 0) ? INVALID_TRIO_M : t_i;
}

/// Find the angle between two stars b_1 and b_2, using the central star as the new origin.
///
/// @param b_1 Star B_1 of the trio.
/// @param b_2 Star B_2 of the trio.
/// @param central Central star of the trio.
/// @return Angle in degrees between b_1 and b_2, with central as the origin.
double Trio::dot_angle (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &central) {
    // Move the origin of b_1 and b_2 to the newly defined center. Normalize these.
    return (180.0 / M_PI) * Vector3::Angle(Vector3::Normalized(b_1 - central), Vector3::Normalized(b_2 - central));
}
