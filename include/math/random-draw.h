/// @file random-draw.h
/// @author Glenn Galvizo
///
/// Header file for RandomDraw namespace, which draws random numbers from various distributions.

#ifndef HOKU_RANDOM_DRAW_H
#define HOKU_RANDOM_DRAW_H

#include <random>

/// @brief Namespace to generate random numbers.
namespace RandomDraw {
    int draw_integer (int floor, int ceiling);
    double draw_real (double floor, double ceiling);
    double draw_normal (double mu, double sigma);

    namespace { std::random_device seed; /* NOLINT(cert-err58-cpp) */ }
    static std::mt19937_64 mersenne_twister(seed()); // NOLINT(cert-err58-cpp)
}

#endif /* HOKU_RANDOM_DRAW_H */
