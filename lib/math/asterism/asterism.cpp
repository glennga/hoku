/// @file asterism.h
/// @author Glenn Galvizo
///
/// Source file for Asterism class, which represents two-dimensional projections of three-dimensional unit vectors
/// (i.e. Stars).

#include "asterism.h"

/// Constructor. Projects stars with width w_n and finds points A, B, C, and D.
///
/// @param s Quad of stars to construct asterism of.
/// @param w_n Size of square to project stars onto.
Asterism::Asterism (const stars &s, const double w_n) {
    points projected = {Mercator(s[0], w_n), Mercator(s[1], w_n), Mercator(s[2], w_n), Mercator(s[3], w_n)};
    double d_max = 0, index_a = 0, index_b = 0;
    
    // Find every distinct pair from the asterism. 4 choose 2 permutations. (Fancy for loop layering below!!)
    for (int i = 0, j = 1; i < 4 - 1; (j < 4 - 1) ? j++ : j = ++i + 1) {
        // Find the pair of points whose separation distance is the largest.
        double d = Mercator::distance_between(projected[i], projected[j]);
        if (d > d_max) {
            index_a = i, index_b = j;
            d_max = d;
        }
    }
    
    // Set points A and B. Points C and D are those that aren't A and B.
    a = projected[index_a], b = projected[index_b];
    for (int i = 0; i < 4; i++) {
        if (c[0] == 0 && c[1] == 0) {
            // Find star C first. Star D is found last.
            c = (i == index_a || i == index_b) ? c : projected[i];
        }
        else {
            d = (i == index_a || i == index_b) ? d : projected[i];
        }
    }
}

/// Test if all three properties for stars C and D are met.
///
/// Property one: C and D are within the circle formed by A and B on it's diameter.
/// Property two: C_x >= D_x.
/// Property three: C_x + D_x < 1.
///
/// @return True if all properties above are met. False otherwise.
bool Asterism::cd_property_met () {
    //  In AB frame, the center of the circle is always (0.5, 0.5).
    Mercator center(0.5, 0.5, 1);
    
    // Distance to this center should always be less than 0.5.
    if (Mercator::distance_between(center, c_prime) > 0.5 || Mercator::distance_between(center, d_prime) > 0.5) {
        return false;
    }
    
    // Check properties two and three.
    return c_prime[0] >= d_prime[0] && c_prime[0] + d_prime[0] < 1;
}

/// Determines the position of C and D stars based on the local coordinate system formed by A and B stars.
///
/// @param w_n Size of square to project stars onto.
/// @return The list [0][0][0][0] if an asterism cannot be formed. The projected coordinates of the C' and D' stars:
/// C'_x, C'_y, D'_x, D'_y otherwise.
Asterism::points_cd Asterism::compute_cd_prime (const double w_n) {
    // Determine the X and Y scale factors from A and B stars.
    double x_k = b[1] - a[1], y_k = b[0] - a[0];
    
    // Treat point A as the origin. Point B is defined as (1, 1). Determine coordinates of C and D.
    c_prime = Mercator((c[0] - a[0]) / x_k, (c[1] - a[1]) / y_k, w_n, c.get_hr());
    d_prime = Mercator((d[0] - a[0]) / x_k, (d[1] - a[1]) / y_k, w_n, d.get_hr());
    
    // If CD properties are not met, try switching C and D.
    if (!cd_property_met()) {
        Mercator t_prime = c_prime, t = c;
        
        c_prime = d_prime, d_prime = t_prime;
        c = d, d = t;
    }
    
    // If CD properties are still not met, set stars appropriately and return a hash code of [0, 0, 0, 0].
    if (!cd_property_met()) {
        auto set_zero = [] (Mercator &m) -> void {
            m = Mercator::zero();
        };
        
        set_zero(c), set_zero(d), set_zero(c_prime), set_zero(d_prime);
    }
    
    return {c_prime[0], c_prime[1], d_prime[0], d_prime[1]};
}

/// Static wrapper for the CD computation. Determines the position of C and D stars based on the local coordinate
/// system formed by A and B stars.
///
/// @param s Quad of stars to construct asterism of.
/// @param w_n Size of square to project stars onto.
/// @return The list [0][0][0][0] if an asterism cannot be formed. The projected coordinates of the C' and D' stars: 
/// C'_x, C'_y, D'_x, D'_y otherwise.
Asterism::points_cd Asterism::hash (const stars &s, const double w_n) {
    // Determine A, B, C, and D stars.
    Asterism m(s, w_n);
    
    // Find and return C' and D' coordinates.
    return m.compute_cd_prime(w_n);
}

/// Given a quad of stars, return them in the order A, B, C, and D respectively.
/// 
/// @param s Quad of stars to find order of.
/// @param w_n Size of square to project stars onto.
/// @return A quad of zero stars if the CD property cannot be met. The same stars in the order of A, B, C, and D 
/// otherwise.
Asterism::stars Asterism::find_abcd (const stars &s, const double w_n) {
    Asterism m(s, w_n);
    stars s_abcd = {Star::zero(), Star::zero(), Star::zero(), Star::zero()};
    
    // We run the computation if the hash generated switches C and D.
    points_cd h = m.compute_cd_prime(w_n);
    
    // If this is a 0-list, return quad of zero stars.
    if (h[0] + h[1] + h[2] + h[3] == 0) {
        return s_abcd;
    }
    
    // Otherwise, we return the same stars in the appropriate order.
    for (const Mercator &p : {m.a, m.b, m.c, m.d}) {
        for (unsigned int i = 0; i < s.size(); i++) {
            s_abcd[i] = (p.get_hr() == s[i].get_hr()) ? s[i] : s_abcd[i];
        }
    }
    
    return s_abcd;
}

/// Find the center of the star set. Used for indexing the results of "hash".
///
/// @param s Quad of stars to determine the center of.
/// @return The center star of the quad.
Star Asterism::center (const stars &s) {
    std::array<double, 3> c;
    
    // For all three dimensions, record the average of that dimension.
    for (int i = 0; i < 3; i++) {
        c[i] = (s[0][i] + s[1][i] + s[2][i] + s[3][i]) / 4;
    }
    
    return Star(c[0], c[1], c[2]).as_unit();
}

