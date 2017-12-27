/// @file identification.cpp
/// @author Glenn Galvizo
///
/// Source file for Identification class, which holds all common data between all identification processes.

#include "identification/identification.h"

/// Returned when no candidates are found from a query.
const Identification::labels_list Identification::NO_CANDIDATES_FOUND = {-1, -1};

/// Returned when there exists no confident alignment from an alignment trial.
const Star::list Identification::NO_CONFIDENT_ALIGNMENT = {};

/// Returned when we count past our defined max nu from a crown or alignment trial.
const Star::list Identification::EXCEEDED_NU_MAX = {Star::zero()};

/// Returned when there exists no confident match set from a crown trial.
const Star::list Identification::NO_CONFIDENT_MATCH_SET = {};

/// Constructor. We set our field-of-view to the default here.
Identification::Identification () : fov(Benchmark::NO_FOV) {
}

/// Rotate every point the given rotation and check if the angle of separation between any two stars is within a
/// given limit sigma.
///
/// @param candidates All stars to check against the body star set.
/// @param q The rotation to apply to all stars.
/// @return Set of matching stars found in candidates and the body sets.
Star::list Identification::find_matches (const Star::list &candidates, const Rotation &q) {
    Star::list matches, non_matched = this->input;
    double epsilon = 3.0 * this->parameters.sigma_overlay;
    matches.reserve(this->input.size());
    
    for (const Star &candidate : candidates) {
        Star r_prime = Rotation::rotate(candidate, q);
        for (unsigned int i = 0; i < non_matched.size(); i++) {
            if (Star::angle_between(r_prime, non_matched[i]) < epsilon) {
                // Add this match to the list by noting the candidate star's catalog ID.
                matches.emplace_back(
                    Star(non_matched[i][0], non_matched[i][1], non_matched[i][2], candidate.get_label()));
                
                // Remove the current star from the searching set. End the search for this star.
                non_matched.erase(non_matched.begin() + i);
                break;
            }
        }
    }
    
    return matches;
}
