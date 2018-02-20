/// @file composite-pyramid.cpp
/// @author Glenn Galvizo
///
/// Source file for CompositePyramid class, which matches a set of body vectors (stars) to their inertial
/// counter-parts in the database.

#include <algorithm>

#include "math/random-draw.h"
#include "math/trio.h"
#include "identification/planar-triangle.h"
#include "identification/composite-pyramid.h"

/// Default parameters for the composite pyramid identification method.
const Identification::Parameters Composite::DEFAULT_PARAMETERS = {DEFAULT_SIGMA_QUERY, DEFAULT_SQL_LIMIT,
    DEFAULT_PASS_R_SET_CARDINALITY, DEFAULT_FAVOR_BRIGHT_STARS, DEFAULT_SIGMA_OVERLAY, DEFAULT_NU_MAX, DEFAULT_NU,
    DEFAULT_F, "COMPOSITE_20"};

/// Constructor. Sets the benchmark data, fov, parameters, and current working table. This is identical to the
/// Pyramid constructor, so we use this instead.
///
/// @param input Working Benchmark instance. We are **only** copying the star set and the fov.
CompositePyramid::CompositePyramid (const Benchmark &input, const Parameters &p) : Pyramid(input, p) {
    
}

/// The CompositePyramid method uses the exact same table as the PlanarTriangle method. Wrap PlanarTriangle's
/// 'generate_table' method.
///
/// @param cf Configuration reader holding all parameters to use.
/// @return TABLE_ALREADY_EXISTS if the table already exists. Otherwise, 0 when finished.
int Composite::generate_table (INIReader &cf) {
    return Plane::generate_table(cf, "composite");
}

/// Find all star pairs whose angle of separation is with 3 * query_sigma (epsilon) degrees of each other.
///
/// @param a Planar area to search with.
/// @param i_t Planar polar moment to search with.
/// @return NO_CANDIDATE_TRIOS_FOUND if there exists no queried trios. Otherwise, the list of star lists (of size = 3)
/// that fall within a and i of the given epsilon.
Composite::labels_list_list Composite::query_for_trios (const double a, const double i) {
    double epsilon = 3.0 * this->parameters.sigma_query;
    std::vector<labels_list> big_r_ell = BaseTriangle::NO_CANDIDATE_TRIOS_FOUND;
    Nibble::tuples_d a_match;
    
    // First, search for trio of stars matching area condition.
    a_match = ch.simple_bound_query("a", "label_a, label_b, label_c, i", a - epsilon, a + epsilon,
                                    this->parameters.sql_limit);
    
    // Next, search this trio for stars matching the moment condition.
    big_r_ell.reserve(a_match.size() / 4);
    for (Chomp::tuple_d t : a_match) {
        if (t[3] >= i - epsilon && t[3] < i + epsilon) {
            big_r_ell.push_back({static_cast<int> (t[0]), static_cast<int> (t[1]), static_cast<int>(t[2])});
        }
    }
    
    // If results are found, remove the initialized value of NO_CANDIDATE_TRIOS_FOUND.
    if (big_r_ell.size() > 1) {
        big_r_ell.erase(big_r_ell.begin());
    }
    
    // Favor bright stars if specified. Applied with the FAVOR_BRIGHT_STARS flag.
    if (this->parameters.favor_bright_stars) {
        sort_brightness(big_r_ell);
    }
    return big_r_ell;
}

/// Given the current map trio, select a random star in the image and verify that the identification is correct.
///
/// @param r Current reference star trio.
/// @param b Current body star trio.
/// @return True if the verification has passed. Otherwise, false.
bool Composite::verification (const Star::trio &r, const Star::trio &b) {
    // Select a random star E. This must not exist in the current body trio.
    Star b_e;
    do {
        b_e = big_i[RandomDraw::draw_integer(0, b.size())];
    }
    while (std::find(b.begin(), b.end(), b_e) != b.end());
    
    // Find all star trios between eij, eik, and ejk.
    auto find_trios = [this, &b_e, &b] (const int m, const int n) -> labels_list_list {
        return this->query_for_trios(Trio::planar_area(b_e, b[m], b[m]), Trio::planar_moment(b_e, b[m], b[n]));
    };
    labels_list_list big_r_eij_ell = find_trios(0, 1), big_r_eik_ell = find_trios(0, 2);
    labels_list_list big_r_ejk_ell = find_trios(1, 2), big_r_eij_join_eik_ell;
    
    // Determine the star E in the catalog using common stars.
    std::set_union(big_r_eij_ell.begin(), big_r_eij_ell.end(), big_r_eik_ell.begin(), big_r_eik_ell.end(),
                   std::back_inserter(big_r_eij_join_eik_ell));
    Star::list big_t_e = common(big_r_ejk_ell, big_r_eij_join_eik_ell, Star::list {});
    
    // If there isn't exactly one star, exit here.
    if (big_t_e.size() != 1) {
        return false;
    }
    
    // If this star is near our R set in the catalog, then this test has passed.
    return Star::within_angle({r[0], r[1], r[2], big_t_e[0]}, fov);
}

/// Given a trio of indices from the input set, determine the matching catalog IDs that correspond to each star.
/// Two verification steps occur: the singular element test and the fourth star test. If these are not met, then the
/// error trio is returned.
///
/// @param b_f Trio of stars in our body frame.
/// @return NO_CANDIDATE_TRIANGLE_FOUND if no triangle can be found. Otherwise, the catalog IDs of stars from the
/// inertial frame.
Star::trio Composite::find_catalog_stars (const Star::trio &b_f) {
    labels_list_list big_r_ell = this->query_for_trios(Trio::planar_area(b_f[0], b_f[1], b_f[2]),
                                                       Trio::planar_moment(b_f[0], b_f[1], b_f[2]));
    if (std::equal(big_r_ell.begin(), big_r_ell.end(), BaseTriangle::NO_CANDIDATE_TRIOS_FOUND.begin())) {
        return NO_CONFIDENT_R_FOUND;
    }
    
    // |R| = 1 restriction. If we do not contain exactly one element, break.
    if ((big_r_ell.size() != 1) && this->parameters.pass_r_set_cardinality) {
        return NO_CONFIDENT_R_FOUND;
    }
    
    // Otherwise, perform the identification (DMT performed below).
    std::array<BaseTriangle::index_trio, 6> big_a_c = {BaseTriangle::STARTING_INDEX_TRIO};
    std::array<Star::list, 6> big_m = {}, big_a = {};
    auto ell = [&big_r_ell, &b_f, &big_a_c] (const int i, const int j) -> Star {
        return Star::define_label(b_f[j], big_r_ell[0][big_a_c[i][j]]);
    };
    
    // Generate unique permutations using previously generated trio.
    for (int i = 1; i < 6; i++) {
        // Given i, swap elements 2 and 3 if even, or 1 and 3 if odd.
        big_a_c[i] = (i % 2) == 0 ? BaseTriangle::index_trio {0, 2, 1} : BaseTriangle::index_trio {2, 1, 0};
    }
    
    // Determine the rotation to take frame R to B. Only use r_1 and r_2 to get rotation.
    for (unsigned int i = 0; i < 3; i++) {
        Rotation q = parameters.f({b_f[0], b_f[1]}, {ch.query_hip(big_r_ell[0][big_a_c[i][0]]),
            ch.query_hip(big_r_ell[0][big_a_c[i][1]])});
        
        big_m[i] = find_positive_overlay(
            ch.nearby_bright_stars(ch.query_hip(big_r_ell[0][0]), fov, static_cast<unsigned int>(3 * big_i.size())), q);
        big_a[i] = {ell(i, 0), ell(i, 1), ell(i, 2)};
    }
    
    // Return map set corresponding to the largest match (messy lambda and iterator stuff below D:).
    Star::list max_a = big_a[
        std::max_element(big_m.begin(), big_m.end(), [] (const Star::list &lhs, const Star::list &rhs) {
            return lhs.size() < rhs.size();
        }) - big_m.begin()];
    return Star::trio {max_a[0], max_a[1], max_a[2]};
}

/// Reproduction of the Composite Pyramid method's database querying. Input image is not used. We require the
/// following be defined:
///
/// @code{.cpp}
///     - table_name
///     - sigma_query
///     - sql_limit
/// @endcode
///
/// @param s Stars to query with. This must be of length = QUERY_STAR_SET_SIZE.
/// @return Vector of likely matches found by the composite pyramid method.
std::vector<Identification::labels_list> Composite::query (const Star::list &s) {
    if (s.size() != QUERY_STAR_SET_SIZE) {
        throw std::runtime_error(std::string("Input list does not have exactly three b."));
    }
    double epsilon = 3.0 * this->parameters.sigma_query;
    std::vector<labels_list> big_r_ell = {};
    Nibble::tuples_d a_match;
    
    // First, search for trio of stars matching area condition.
    double a = Trio::planar_area(s[0], s[1], s[2]), i = Trio::planar_moment(s[0], s[1], s[2]);
    a_match = ch.simple_bound_query("a", "label_a, label_b, label_c, i", a - epsilon, a + epsilon,
                                    this->parameters.sql_limit);
    
    // Next, search this trio for stars matching the moment condition.
    big_r_ell.reserve(a_match.size() / 4);
    for (Chomp::tuple_d t : a_match) {
        if (t[3] >= i - epsilon && t[3] < i + epsilon) {
            big_r_ell.push_back({static_cast<int> (t[0]), static_cast<int> (t[1]), static_cast<int>(t[2])});
        }
    }
    
    // Return the trios.
    return big_r_ell;
}