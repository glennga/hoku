/// @file random-draw.cpp
/// @author Glenn Galvizo
///
/// Source file for RandomDraw namespace, which which draws random numbers from various distributions.

#include "math/random-draw.h"

/// Return a real (floating) number between a specified floor and ceiling.
///
/// @param floor Minimum value of the real distribution.
/// @param ceiling Maximum value of the real distribution.
/// @return Random real number between floor and ceiling.
double RandomDraw::draw_real (double floor, double ceiling) {
    std::uniform_real_distribution<double> d(floor, ceiling);
    return d(mersenne_twister);
}

/// Return a real (floating) number in a normal distribution with the given mean and deviation.
///
/// @param mean Mean of the distribution.
/// @param deviation Deviation of the distribution.
/// @return Random number from described normal distribution.
double RandomDraw::draw_normal (double mean, double deviation) {
    std::normal_distribution<double> d(mean, deviation);
    return d(mersenne_twister);
}

/// Return an integer between a specified floor and ceiling.
///
/// @param floor Minimum value of the distribution.
/// @param ceiling Maximum value of the distribution.
/// @return Random integer between floor and ceiling.
int RandomDraw::draw_integer (int floor, int ceiling) {
    std::uniform_int_distribution<int> d(floor, ceiling);
    return d(mersenne_twister);
}