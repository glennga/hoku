/// @file crown.cpp
/// @author Glenn Galvizo
///
/// Source file for all trials. This holds the namespaces of functions that allow us to test various methods and log
/// the data.

#include <algorithm>

#include "experiment/experiment.h"
#include "math/random-draw.h"

double Experiment::Map::percentage_correct (const Identification::StarsEither &b, const Star::list &answers,
                                            const double fov) {
    double count = 0;

    // If we obtain an error result, the percentage correct is 0.
    if (b.error != 0) return 0;

    // Stars must match according to their contents and labels.
    std::for_each(answers.begin(), answers.end(), [&count, &b, &fov] (const Star &s) -> void {
        count += (std::find_if(b.result.begin(), b.result.end(), [&s, &fov] (const Star &b_j) -> bool {
            return Star::within_angle(b_j, s, fov) &&
                   b_j.get_label() == s.get_label();
        }) != b.result.end()) ? 1.0 : 0;
    });

    std::cout << "[EXPERIMENT] Percentage correct: " << count / b.result.size() << std::endl;
    return count / b.result.size();
}