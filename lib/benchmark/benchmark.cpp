/// @file benchmark.cpp
/// @author Glenn Galvizo
///
/// Source file for Benchmark class, which generates the input data for star identification testing.

#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <libgen.h>

#include "benchmark/benchmark.h"
#include "math/random-draw.h"

const double Benchmark::NO_FOV = -1;
const double Benchmark::NO_M_BAR = 30.0;
const int Benchmark::NO_N = -1;

Benchmark::Benchmark (const std::shared_ptr<Chomp>& ch, const double fov, const int n, const double m_bar) {
    this->b = std::make_shared<Star::list>(), this->b_answers = std::make_shared<Star::list>();
    this->r = std::make_shared<Star::list>(), this->fov = fov;
    this->generate_stars(ch, n, m_bar);
}

Star Benchmark::operator[] (const unsigned int n) const { return (*this->b)[n]; }
void Benchmark::shuffle () {
    auto dup_e1 = RandomDraw::mersenne_twister, dup_e2 = RandomDraw::mersenne_twister;
    std::shuffle(this->b->begin(), this->b->end(), RandomDraw::mersenne_twister);
    std::shuffle(this->r->begin(), this->r->end(), dup_e1);
    std::shuffle(this->b_answers->begin(), this->b_answers->end(), dup_e2);
}

/// Obtain a set of stars around the current focus vector. This is the 'clean' star set, meaning that all stars in
/// 'stars' accurately represent what is found in the catalog. This is also used to reset a benchmark.
void Benchmark::generate_stars (const std::shared_ptr<Chomp>& ch, const int n, const double m_bar) {
    auto expected = static_cast<unsigned int>(300); // Not too worried about this number.

    do {
        this->b->clear(), this->b_answers->clear(), this->r->clear();
        this->center = Star::chance().get_vector();
        this->q_rb = Rotation::chance();

        // Find nearby stars. Rotate these stars and the center.
        Star::list candidates = ch->nearby_hip_stars(this->center, fov / 2.0, expected);

        std::for_each(candidates.begin(), candidates.end(), [this, &m_bar] (const Star &s) -> void {
            if (s.get_magnitude() <= m_bar) {
                Star rotated = Rotation::rotate(s, this->q_rb);

                this->b_answers->push_back(rotated);
                this->b->push_back(Star::reset_label(rotated));
                this->r->push_back(s);
            }
        });
        this->center = Rotation::rotate(Star::wrap(this->center), this->q_rb).get_vector();

    } while (this->b->size() <= 4);

    if (n != NO_N && this->b->size() > n) { this->r->resize(n), this->b->resize(n), this->b_answers->resize(n); }
    this->shuffle();
}

std::shared_ptr<Star::list> Benchmark::get_image () { return this->b; }
std::shared_ptr<Star::list> Benchmark::get_answers () { return this->b_answers; }
std::shared_ptr<Star::list> Benchmark::get_inertial (){ return this->r; }
Vector3 Benchmark::get_center () { return this->center; }
double Benchmark::get_fov () { return this->fov; }

/// Append n randomly placed vectors that fall within fov/2 degrees of the focus. This models stray light that may
/// randomly wander into the detector, or false positives.
void Benchmark::add_extra_light (const unsigned int n) {
    unsigned int current_n = 0;

    while (current_n < n) {
        Star generated = Star::chance(-current_n - 1);
        if (Star::within_angle(generated, this->center, this->fov / 2.0)) {
            this->r->emplace_back(Star::wrap(Vector3(0, 0, 0), -current_n));
            this->b->emplace_back(generated);
            current_n++;
        }
    }
    this->shuffle();
}

/// Randomly generate n dark spots. All stars in the input that are within psi/2 degrees of the blobs are removed.
/// This model simulates celestial bodies that may block light from the detector.
void Benchmark::remove_light (const unsigned int n, const double psi) {
    unsigned int current_n = 0;
    std::vector<Star> blobs;

    // First, generate the light blocking blobs.
    while (current_n < n) {
        Star generated = Star::chance();
        if (Star::within_angle(generated, this->center, this->fov / 2.0)) {
            blobs.emplace_back(generated);
            current_n++;
        }
    }

    // Second, check if any of the stars fall within psi / 2 of a dark spot.
    for (const Star &blob : blobs) {
        auto s_b = this->b->begin(), s_r = this->r->begin();

        while (s_b != this->b->end() && s_r != this->r->end()) {
            if (Star::within_angle(blob, *s_b, psi / 2.0)) {
                s_b = this->b->erase(s_b);
                s_r = this->r->erase(s_r);
            }
            else {
                ++s_b;
                ++s_r;
            }
        }
    }
    this->shuffle();
}

/// Randomly move n vectors based off the given sigma. Change is normally distributed, with 0 change being the most
/// expected and having a standard deviation of sigma.
void Benchmark::shift_light (const unsigned int n, const double sigma) {
    unsigned int current_n = 0;

    for (unsigned int i = 0; i < this->b->size() && current_n < n; i++) {
        Star candidate;

        // Ensure that the shifted star does not veer out of focus.
        do {
            candidate = Rotation::shake(this->b->at(i), sigma);
        } while (!Star::within_angle(candidate, this->center, this->fov / 2.0));

        // Modify our star set.
        this->b->at(i) = candidate;
        current_n++;
    }
   this->shuffle();
}
