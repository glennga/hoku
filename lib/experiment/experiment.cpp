/// @file crown.cpp
/// @author Glenn Galvizo
///
/// Source file for all trials. This holds the namespaces of functions that allow us to test various methods and log
/// the data.

#include <algorithm>

#include "experiment/experiment.h"
#include "math/random-draw.h"

/// Generate the benchmark to be used for identification. Restricts which stars to include based on the given camera
/// sensitivity (m_bar).
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param big_i Reference to location of the body stars.
/// @param big_c Reference to location of the corresponding catalog stars.
/// @param center Reference to location of the focus star.
/// @param fov Double of the field-of-view stars can exist from the focus.
/// @param m_bar Minimum magnitude that all stars must be under.
void Experiment::present_benchmark (Chomp &ch, Star::list &big_i, Star::list &big_c, Star &center, const double fov,
                                    const double m_bar) {
    Rotation q = Rotation::chance();
    std::vector<Star::pair> big_ic = {};
    
    // We require at-least five stars to exist here.
    do {
        center = Star::chance();
        
        big_ic.reserve(static_cast<unsigned>(fov * 4));
        for (const Star &s : ch.nearby_hip_stars(center, fov / 2.0, static_cast<unsigned int> (fov * 4))) {
            if (s.get_magnitude() < m_bar) {
                big_ic.emplace_back(Star::pair {Rotation::rotate(s, q), s});
            }
        }
    }
    while (big_ic.size() < 5);
    
    // Rotate after I has been found.
    center = Rotation::rotate(center, q);
    
    // Shuffle our star lists.
    std::shuffle(big_ic.begin(), big_ic.end(), RandomDraw::mersenne_twister);
    
    // Insert the new image and catalog.
    big_i.clear(), big_c.clear();
    big_i.reserve(big_ic.size()), big_c.reserve(big_ic.size());
    std::for_each(big_ic.begin(), big_ic.end(), [&big_i, &big_c] (const Star::pair &ic) {
        big_i.emplace_back(ic[0]), big_c.emplace_back(ic[1]);
    });
}

/// Return true if the given body set exists somewhere in a collection of reference sets.
///
/// @param big_r_ell Reference to the reference frame sets. An in-sort place will occur for each element.
/// @param big_i_ell Reference to the body frame set. An in-sort place will occur.
/// @return True if 'b' exists somewhere in r.
bool Experiment::Query::set_existence (std::vector<Identification::labels_list> &big_r_ell,
                                       Identification::labels_list &big_i_ell) {
    for (Identification::labels_list &r_ell : big_r_ell) {
        std::sort(r_ell.begin(), r_ell.end()), std::sort(big_i_ell.begin(), big_i_ell.end());
        if (std::equal(r_ell.begin(), r_ell.end(), big_i_ell.begin())) {
            return true;
        }
    }
    
    return false;
}

/// Generate N random stars that fall within the specified field-of-view. Rotate this result by some random quaternion.
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param n Number of stars to generate.
/// @param center Reference to center used to query Nibble with.
/// @param fov Double of the field-of-view stars can exist from the focus.
/// @return List of n stars who are within fov degrees from each other.
Star::list Experiment::Query::generate_n_stars (Chomp &ch, const unsigned int n, Star &center, const double fov) {
    // Find all stars near some random center.
    center = Star::chance();
    Star::list s;
    do {
        s = ch.nearby_bright_stars(center, fov / 2.0, static_cast<unsigned int> (fov * 4));
        std::shuffle(s.begin(), s.end(), RandomDraw::mersenne_twister);
    }
    while (s.size() < n);
    
    // Truncate to the appropriate size.
    s.resize(n);
    
    // Rotate all stars by a random quaternion.
    Rotation q = Rotation::chance();
    std::transform(s.begin(), s.end(), s.begin(), [&q] (const Star &s_i) {
        return Rotation::rotate(s_i, q);
    });
    center = Rotation::rotate(center, q);
    
    return s;
}

/// Determine if the stars a body subset matches the given inertial labels.
///
/// @param big_c All inertial stars used in reduction trial.
/// @param r Returned stars found after reduction step.
/// @return Percentage of correctly identified stars in r.
double Experiment::Reduction::percentage_correct (const Star::list &big_c, const Star::list &r) {
    double result = 0;
    
    // Count the number of correct stars in r.
    std::for_each(r.begin(), r.end(), [&result, &big_c] (const Star &r_i) -> void {
        result += (std::find_if(big_c.begin(), big_c.end(), [&r_i] (const Star &c) -> bool {
            return c == r_i && c.get_label() == r_i.get_label();
        }) != big_c.end()) ? 1.0 : 0;
    });
    
    return result / r.size();
}

/// Determine if the stars specified in b_ell (our inertial set) correctly matches our body set.
///
/// @param big_i All body stars. Check if b_ell correctly matches stars in this set.
/// @param b Subset of our body, but with predicted catalog labels.
/// @return True if the predicted catalog labels matches the ground truth labels (body set).
double Experiment::Map::percentage_correct (const Star::list &big_i, const Star::list &b) {
    unsigned int count = 0;
    
    // Stars must match according to their contents and labels.
    std::for_each(big_i.begin(), big_i.end(), [&count, &b] (const Star &s) -> void {
        count += (std::find_if(b.begin(), b.end(), [&s] (const Star &b_j) -> bool {
            return b_j == s && b_j.get_label() == s.get_label();
        }) != b.end()) ? 1 : 0;
    });
    
    return count / static_cast<double>(b.size());
}

/// Determine the performance of the overlay classifier by determining it's confusion matrix entries.
///
/// @param big_i_prime Stars in image with attached catalog labels (result from FPO method).
/// @param big_i All image stars. Check for all stars in P that exist in I.
/// @param es Number of extra stars that exist in input presented to FPO.
/// @param tn Reference to the location to hold all true negatives.
/// @param fp Reference to the location to hold all false positives.
/// @param fn Reference to the location to hold all false negatives.
/// @param tp Reference to the location to hold all true positives.
void Experiment::Overlay::count_correct (const Star::list &big_i_prime, const Star::list &big_i, const int es,
                                         double &tn, double &fp, double &fn, double &tp) {
    tn = fp = fn = tp = 0;

    // Determine the true positives (in I', in I).
    std::for_each(big_i.begin(), big_i.end(), [&big_i_prime, &tp] (const Star &s_i) -> void {
        tp += (std::find_if(big_i_prime.begin(), big_i_prime.end(), [&s_i] (const Star &s_prime_i) {
            return s_i.get_label() == s_prime_i.get_label();
        }) != big_i_prime.end()) ? 1 : 0;
    });
    
    // Determine the false positives (in I', not in I).
    std::for_each(big_i_prime.begin(), big_i_prime.end(), [&big_i, &fp] (const Star &s_prime_i) -> void {
        fp += (std::find_if(big_i.begin(), big_i.end(), [&s_prime_i] (const Star &s_i) {
            return s_i.get_label() == s_prime_i.get_label();
        }) == big_i.end()) ? 1 : 0;
    });
    
    // Determine the false negatives (not in I', in I).
    std::for_each(big_i.begin(), big_i.end(), [&big_i_prime, &fn] (const Star &s_i) -> void {
        fn += (std::find_if(big_i_prime.begin(), big_i_prime.end(), [&s_i] (const Star &s_prime_i) {
            return s_i.get_label() == s_prime_i.get_label();
        }) == big_i_prime.end()) ? 1 : 0;
    });
    
    // Determine the true negatives (not in I', not in I).
    tn = (big_i.size() + es) - (tp + fp + fn);
}