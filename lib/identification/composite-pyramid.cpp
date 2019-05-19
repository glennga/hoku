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


int Composite::generate_table (const std::shared_ptr<Chomp> &ch, double fov, const std::string &table_name) {
    return Plane::generate_table(ch, fov, table_name);
}

Composite::labels_list_list Composite::query_for_trios (const double a, const double i) {
    std::vector<labels_list> big_r_ell;
    Nibble::tuples_d matches;

    // Query for candidates using all fields.
    matches = ch->simple_bound_query(
            {"a", "i"},
            "label_a, label_b, label_c, i",
            {a - epsilon_1, i - epsilon_2},
            {a + epsilon_1, i + epsilon_2},
            500
    );
    nu++;

    // Next, search this trio for stars matching the moment condition.
    big_r_ell.reserve(matches.size());
    std::for_each(matches.begin(), matches.end(), [&big_r_ell] (const Chomp::tuple_d &t) -> void {
        big_r_ell.emplace_back(labels_list{static_cast<int> (t[0]), static_cast<int> (t[1]), static_cast<int>(t[2])});
    });

    return big_r_ell;
}

bool Composite::verification (const Star::trio &r, const Star::trio &b) {
    // Select a random star E. This must not exist in the current body trio.
    Star b_e;
    do {
        b_e = (be->get_image()->at(RandomDraw::draw_integer(0, b.size())));
    } while (std::find(b.begin(), b.end(), b_e) != b.end());

    // Find all star trios between eij, eik, and ejk.
    auto find_trios = [this, &b_e, &b] (const int m, const int n) -> labels_list_list {
        return this->query_for_trios(Trio::planar_area(b_e, b[m], b[n]), Trio::planar_moment(b_e, b[m], b[n]));
    };
    labels_list_list big_r_eij_ell = find_trios(0, 1), big_r_eik_ell = find_trios(0, 2);
    labels_list_list big_r_ejk_ell = find_trios(1, 2);

    // Determine the star E in the catalog using common stars.
    Star::list big_t_e = common(big_r_eij_ell, big_r_eik_ell, big_r_ejk_ell, Star::list{});

    // If there isn't exactly one star, exit here.
    if (big_t_e.size() != 1 || big_t_e.empty()) {
        std::cout << "[COMPOSITE] Verification failed." << std::endl;
        return false;
    }

    // If this star is near our R set in the catalog, then this test has passed.
    std::cout << "[COMPOSITE] Verification passed." << std::endl;
    return Star::within_angle({r[0], r[1], r[2], big_t_e[0]}, be->get_fov());
}

/// Given a trio of indices from the input set, determine the matching catalog IDs that correspond to each star.
/// Two verification steps occur: the singular element test and the fourth star test. If these are not met, then the
/// error trio is returned.
Composite::TriosEither Composite::find_catalog_stars (const Star::trio &b_f) {
    std::cout << "[COMPOSITE] Finding catalog stars." << std::endl;
    labels_list_list big_r_ell = this->query_for_trios(Trio::planar_area(b_f[0], b_f[1], b_f[2]),
                                                       Trio::planar_moment(b_f[0], b_f[1], b_f[2]));

    if (big_r_ell.size() != 1) return TriosEither{{}, NO_CONFIDENT_R_FOUND_EITHER};

    // Otherwise, perform the identification (DMT performed below).
    std::array<Star::list, 6> big_m = {}, big_a = {};
    Star::list big_p = ch->nearby_bright_stars(ch->query_hip(big_r_ell[0][0]), be->get_fov(),
                                               static_cast<unsigned int>(3 * be->get_image()->size()));
    nu++;

    // Generate all permutations of <0, 1, 2>.
    std::array<BaseTriangle::index_trio, 6> big_a_c = {
            BaseTriangle::STARTING_INDEX_TRIO,
            BaseTriangle::index_trio{0, 2, 1},
            BaseTriangle::index_trio{1, 0, 2},
            BaseTriangle::index_trio{1, 2, 0},
            BaseTriangle::index_trio{2, 0, 1},
            BaseTriangle::index_trio{2, 1, 0}
    };

    // Determine the rotation to take frame R to B.
    for (unsigned int i = 0; i < 6; i++) {
        std::array<int, 3> j = {big_r_ell[0][big_a_c[i][0]], big_r_ell[0][big_a_c[i][1]], big_r_ell[0][big_a_c[i][2]]};
        Rotation q = Rotation::triad(
                {b_f[0], b_f[1], b_f[2]},
                {ch->query_hip(j[0]), ch->query_hip(j[1]), ch->query_hip(j[2])}
        );

        big_m[i] = find_positive_overlay(*be->get_image(), big_p, q, this->epsilon_4);
        big_a[i] = {ch->query_hip(j[0]), ch->query_hip(j[1]), ch->query_hip(j[2])};
    }

    // Return map set corresponding to the largest match (messy lambda and iterator stuff below D:).
    if (big_a[0].size() == big_a[1].size() == big_a[2].size() == big_a[3].size() == big_a[4].size()
        == big_a[5].size())
        return TriosEither{{}, NO_CONFIDENT_R_FOUND_EITHER};
    else {
        Star::list max_a = big_a[
                std::max_element(big_m.begin(), big_m.end(), [] (const Star::list &lhs, const Star::list &rhs) {
                    return lhs.size() < rhs.size();
                }) - big_m.begin()];
        return TriosEither{Star::trio{max_a[0], max_a[1], max_a[2]}, 0};
    }
}

std::vector<Identification::labels_list> Composite::query () {
    std::vector<labels_list> big_r_ell = {};
    Nibble::tuples_d matches;

    // First, search for trio of stars matching area condition.
    double a = Trio::planar_area(
            be->get_image()->at(0),
            be->get_image()->at(1),
            be->get_image()->at(2));
    double i = Trio::planar_moment(
            be->get_image()->at(0),
            be->get_image()->at(1),
            be->get_image()->at(2));
    matches = ch->simple_bound_query(
            {"a", "i"},
            "label_a, label_b, label_c, i",
            {a - epsilon_1, i - epsilon_2},
            {a + epsilon_1, i + epsilon_2},
            500);

    // Next, transform all stars into candidate set labels.
    big_r_ell.reserve(matches.size());
    std::for_each(matches.begin(), matches.end(), [&big_r_ell] (const Chomp::tuple_d &t) -> void {
        big_r_ell.emplace_back(labels_list{
                static_cast<int> (t[0]),
                static_cast<int> (t[1]),
                static_cast<int>(t[2])
        });
    });

    // Return the trios.
    return big_r_ell;
}

Composite::StarsEither Composite::reduce () {
    ch->select_table(this->table_name);
    nu = 0;

    for (unsigned int dj = 1; dj < be->get_image()->size() - 1; dj++) {
        for (unsigned int dk = 1; dk < be->get_image()->size() - dj - 1; dk++) {
            for (unsigned int di = 0; di < be->get_image()->size() - dj - dk - 1; di++) {
                int i = di, j = di + dj, k = j + dk;

                // Practical limit: exit early if we have iterated through too many comparisons without match.
                if (nu > this->nu_max) return StarsEither{{}, NO_CONFIDENT_R_EITHER};

                labels_list_list big_r_ell = this->query_for_trios(
                        Trio::planar_area(be->get_image()->at(i),
                                          be->get_image()->at(j),
                                          be->get_image()->at(k)),
                        Trio::planar_area(be->get_image()->at(i),
                                          be->get_image()->at(j),
                                          be->get_image()->at(k))
                );

                if (big_r_ell.size() != 1) continue;
                return StarsEither{Star::list{
                        ch->query_hip(big_r_ell[0][0]),
                        ch->query_hip(big_r_ell[0][1]),
                        ch->query_hip(big_r_ell[0][2])
                }, 0};
            }
        }
    }

    return StarsEither{{}, NO_CONFIDENT_R_EITHER};
}