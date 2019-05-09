/// @file random-draw.cpp
/// @author Glenn Galvizo
///
/// Source file for RandomDraw namespace, which which draws random numbers from various distributions.

#include "math/random-draw.h"

double RandomDraw::draw_real (double floor, double ceiling) {
    std::uniform_real_distribution<double> d(floor, ceiling);
    return d(mersenne_twister);
}

double RandomDraw::draw_normal (double mu, double sigma) {
    std::normal_distribution<double> d(mu, sigma);
    return d(mersenne_twister);
}

int RandomDraw::draw_integer (int floor, int ceiling) {
    std::uniform_int_distribution<int> d(floor, ceiling);
    return d(mersenne_twister);
}