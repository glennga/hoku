/// @file random-draw.h
/// @author Glenn Galvizo
///
/// Header file for RandomDraw namespace, which draws random numbers from various distributions.

#ifndef HOKU_RANDOM_DRAW_H
#define HOKU_RANDOM_DRAW_H

#include <random>

/// @brief Namespace to generate random numbers.
///
/// The random draw namespace holds functions to generate distributed random numbers: integers, real numbers, and
/// normal real numbers.
///
/// @example
/// @code{.cpp}
/// // Draw a random integer between -10 and 10.
/// std::cout << RandomDraw::draw_integer(-10, 10) << std::endl;
///
/// // Draw a real number from a normal distribution N(0, 0.5^2).
/// std::cout << RandomDraw::draw_normal(0, 0.5) << std::endl;
/// @endcode
namespace RandomDraw {
    int draw_integer (int floor, int ceiling);
    double draw_real (double floor, double ceiling);
    double draw_normal (double mean, double deviation);
    
    namespace {
        /// Random device used to seed mersenne twister.
        std::random_device seed;
    }
    
    /// Mersenne twister used for all random distributions.
    static std::mt19937_64 mersenne_twister (seed());
}

#endif /* HOKU_RANDOM_DRAW_H */
