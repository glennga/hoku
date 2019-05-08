/// @file random-draw.cpp
/// @author Glenn Galvizo
///
/// Source file for RandomDraw namespace, which which draws random numbers from various distributions.

#include "math/random-draw.h"

/// Return a real (floating) number between a specified floor and ceiling.
double RandomDraw::draw_real (double floor, double ceiling) {
    std::uniform_real_distribution<double> d(floor, ceiling);
    return d(mersenne_twister);
}

/// Return a real (floating) number in a normal distribution with the given mean and deviation.
double RandomDraw::draw_normal (double mu, double sigma) {
    std::normal_distribution<double> d(mu, sigma);
    return d(mersenne_twister);
}

/// Return an integer between a specified floor and ceiling.
int RandomDraw::draw_integer (int floor, int ceiling) {
    std::uniform_int_distribution<int> d(floor, ceiling);
    return d(mersenne_twister);
}