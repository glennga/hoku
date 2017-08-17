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

/// Geometric hash function. Determines the position of C and D stars based on the local coordinate systtem formed by
/// A and B stars.
///
/// @param s Quad of stars to construct asterism of.
/// @param w_n Size of square to project stars onto.
Asterism::points_cd Asterism::hash (const stars &s, const double w_n) {
    double y_k, x_k;
    
    // Determine A, B, C, and D stars.
    Asterism m(s, w_n);
    
    // Determine X and Y scale factors.
    y_k = m.b[1] - m.a[1], x_k = m.b[0] - m.a[0];
    
    // Treat point A as the origin. Point B is defined as (1, 1). Determine coordinates of C and D.
    m.c_prime = Mercator((m.c[0] - m.a[0]) / x_k, (m.c[1] - m.a[1]) / y_k, w_n);
    m.d_prime = Mercator((m.d[0] - m.a[0]) / x_k, (m.d[1] - m.a[1]) / y_k, w_n);
    
    // If CD properties are not met, try switching C and D.
    if (!m.cd_property_met()) {
        Mercator t = m.c_prime;
        m.c_prime = m.d_prime;
        m.d_prime = t;
    }
    
    // If CD properties are still not met, return a hash code of [0, 0, 0, 0].
    if (!m.cd_property_met()) {
        m.c_prime = Mercator::zero();
        m.d_prime = Mercator::zero();
    }
    
    return {m.c_prime[0], m.c_prime[1], m.d_prime[0], m.d_prime[1]};
}

