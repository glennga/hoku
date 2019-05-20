/// @file identification.cpp
/// @author Glenn Galvizo
///
/// Source file for Identification class, which holds all common data between all identification processes.

#include <numeric>

#include "math/random-draw.h"
#include "benchmark/benchmark.h"
#include "identification/identification.h"

const int Identification::NO_CONFIDENT_R_EITHER = -3;
const int Identification::NO_CONFIDENT_A_EITHER = -2;
const int Identification::EXCEEDED_NU_MAX_EITHER = -1;
const int Identification::TABLE_ALREADY_EXISTS = -1;

Identification::Identification (const std::shared_ptr<Benchmark> &be, const std::shared_ptr<Chomp> &ch,
                                const double epsilon_1, const double epsilon_2, const double epsilon_3,
                                const double epsilon_4, const unsigned int nu_max, const std::string &identifier,
                                const std::string &table_name) {
    this->epsilon_1 = epsilon_1, this->epsilon_2 = epsilon_2, this->epsilon_3 = epsilon_3, this->epsilon_4 = epsilon_4;
    this->identifier = identifier, this->table_name = table_name;
    this->nu_max = nu_max, this->nu = 0;
    this->be = be, this->ch = ch;

    this->ch->select_table(this->table_name);
}

unsigned int Identification::get_nu () { return this->nu; }

/// Rotate every point with the given rotation and check if the angle of separation between any two stars is within a
/// given limit sigma.
Star::list Identification::find_positive_overlay (const Star::list &big_i, const Star::list &big_p, const Rotation &q,
                                                  double epsilon) {
    Star::list big_p_c = big_p, m;

    std::shuffle(big_p_c.begin(), big_p_c.end(), RandomDraw::mersenne_twister);
    std::for_each(big_p_c.begin(), big_p_c.end(), [&] (const Star &p_i) -> void {
        Star r_prime = Rotation::rotate(p_i, q);

        for (const Star &i_i : big_i) {
            if (Star::within_angle(r_prime, i_i, epsilon)) {
                m.emplace_back(Star(i_i[0], i_i[1], i_i[2], r_prime.get_label()));

                // Break early, we have found our match.
                break;
            }
        }
    });

    return m;
}
