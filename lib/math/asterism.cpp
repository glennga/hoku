/// @file asterism.h
/// @author Glenn Galvizo
///
/// Source file for Asterism class, which represents two-dimensional projections of three-dimensional unit vectors
/// (i.e. Stars).

#include "math/asterism.h"

/// Returned by hashing functions when an asterism cannot be formed.
const Asterism::points_cd Asterism::MALFORMED_HASH = {0, 0, 0, 0};

/// Returned by ordering functions when order cannot be determined.
const Asterism::stars Asterism::INDETERMINATE_ORDER = {Star::zero(), Star::zero(), Star::zero(), Star::zero()};

/// Constructor. Projects stars with width = 1.0 and finds points A, B, C, and D.
///
/// @param s Quad of stars to construct asterism of.
Asterism::Asterism (const stars &s) {
    points projected = {Mercator(s[0], 1), Mercator(s[1], 1), Mercator(s[2], 1), Mercator(s[3], 1)};
    int index_a = 0, index_b = 0;
    double d_max = 0;
    
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
    
    // Verify points A and B. Transform the coordinate system as required.
    verify_ab_stars();
}

/// Verify that the stars A and B represent the points closest and furthest away from the origin respectively. If
/// not, modify the system until they are.
void Asterism::verify_ab_stars () {
    Mercator origin(0, 0, 1), closest(2, 2, 1);
    
    // Translate coordinate system to positive quadrant.
    auto translate_positive = [] (Mercator &m) -> Mercator {
        return Mercator(m[0] + 0.5, m[1] + 0.5, 1, m.get_label());
    };
    a = translate_positive(a), b = translate_positive(b), c = translate_positive(c), d = translate_positive(d);
    
    // Find the point closest to the origin in future AB frame.
    for (const Mercator &m : {a, b, c, d}) {
        closest = (Mercator::distance_between(origin, m) < Mercator::distance_between(origin, closest)) ? m : closest;
    }
    
    // If the closest point is not the current index_a or index_b, rotate the system 90 degrees and keep it positive.
    if (closest.get_label() != a.get_label() && closest.get_label() != b.get_label()) {
        auto rotate_positive = [] (Mercator &m) -> Mercator {
            return Mercator(m[1], -m[0] + 1, 1, m.get_label());
        };
        a = rotate_positive(a), b = rotate_positive(b), c = rotate_positive(c), d = rotate_positive(d);
    }

    // Now, set the point closest to the origin as A, and the one left as B.
    if (Mercator::distance_between(origin, b) < Mercator::distance_between(origin, a)) {
        Mercator t(a[0], a[1], 1, a.get_label());
        a = b, b = t;
    }
    
    // If star A does not have the smallest X or Y, reflect over the line y=x.
    if (a[0] > b[0] || a[1] > b[1]) {
        auto reflect_yx = [] (Mercator &m) -> Mercator {
            return Mercator(m[1], m[0], 1, m.get_label());
        };
        a = reflect_yx(a), b = reflect_yx(b), c = reflect_yx(c), d = reflect_yx(d);
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
/// @return A MALFORMED_HASH if an asterism cannot be formed. The projected coordinates of the C' and D' stars:
/// C'_x, C'_y, D'_x, D'_y otherwise.
Asterism::points_cd Asterism::compute_cd_prime () {
    // Treat point A as the origin (0, 0). Point B is defined as (1, 1). Determine coordinates of C' and D'.
    c_prime = Mercator((c[0] - a[0]) / (b[0] - a[0]), (c[1] - a[1]) / (b[1] - a[1]), 1, c.get_label());
    d_prime = Mercator((d[0] - a[0]) / (b[0] - a[0]), (d[1] - a[1]) / (b[1] - a[1]), 1, d.get_label());
    
    // If CD properties are not met, try switching C and D.
    if (!cd_property_met()) {
        Mercator t_prime = c_prime, t = c;
        
        c_prime = d_prime, d_prime = t_prime;
        c = d, d = t;
    }
    
    // If CD properties are still not met, set stars appropriately and return a hash code of [0, 0, 0, 0].
    if (!cd_property_met()) {
        return MALFORMED_HASH;
    }
    return {c_prime[0], c_prime[1], d_prime[0], d_prime[1]};
}

/// Static wrapper for the CD computation. Determines the position of C and D stars based on the local coordinate
/// system formed by A and B stars.
///
/// @param s Quad of stars to construct asterism of.
/// @return A MALFORMED_HASH if an asterism cannot be formed. The projected coordinates of the C' and D' stars:
/// C'_x, C'_y, D'_x, D'_y otherwise.
Asterism::points_cd Asterism::hash (const stars &s) {
    // Determine A, B, C, and D stars.
    Asterism m(s);
    
    // Find and return C' and D' coordinates.
    return m.compute_cd_prime();
}

/// Given a quad of stars, return them in the order A, B, C, and D respectively.
/// 
/// @param s Quad of stars to find order of.
/// @return INDETERMINATE_ORDER if the CD property cannot be met. The same stars in the order of A, B, C, and D
/// otherwise.
Asterism::stars Asterism::find_order (const stars &s) {
    Asterism m(s);
    stars s_abcd = INDETERMINATE_ORDER;
    
    // We run the computation if the hash generated switches C and D.
    points_cd h = m.compute_cd_prime();
    
    // If this is a 0-list, return quad of zero stars.
    if (h[0] + h[1] + h[2] + h[3] == 0) {
        return s_abcd;
    }
    
    // Otherwise, we return the same stars in the appropriate order.
    for (const Mercator &p : {m.a, m.b, m.c, m.d}) {
        for (unsigned int i = 0; i < s.size(); i++) {
            s_abcd[i] = (p.get_label() == s[i].get_label()) ? s[i] : s_abcd[i];
        }
    }
    
    return s_abcd;
}

/// Find the center of the star set. Used for indexing the results of "hash". Note that this is **only** used for
/// hashing the asterism, and represents a rough centroid rather than a really accurate one.
///
/// @param s Quad of stars to determine the center of.
/// @return The center star of the quad.
Star Asterism::center (const stars &s) {
    auto average_dim = [&s] (const int n) -> double {
        std::array<double, 4> n_vector = {s[0][n], s[1][n], s[2][n], s[3][n]};
        return std::accumulate(n_vector.begin(), n_vector.end(), 0.0) / 4;
    };
    
    return {average_dim(0), average_dim(1), average_dim(2), 0, true};
}

