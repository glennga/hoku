/// @file random-draw.h
/// @author Glenn Galvizo
///
/// Header file for RandomDraw namespace, which draws random numbers from various distributions.

#ifndef HOKU_RANDOM_DRAW_H
#define HOKU_RANDOM_DRAW_H

#include <random>

/// The RandomDraw namespace holds functions to generate distributed random numbers.
namespace RandomDraw {
    int draw_integer (int floor, int ceiling);
    double draw_real (double floor, double ceiling);
    double draw_normal (double mean, double deviation);
    
    namespace {
        /// Random device used to seed mersenne twister.
        std::random_device seed;
        
        /// Mersenne twister used for all random distributions.
        std::mt19937_64 mersenne_twister (seed());
    }
}

#endif /* HOKU_RANDOM_DRAW_H */
