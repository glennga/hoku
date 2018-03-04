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
/// @param center Reference to location of the focus star.
/// @param fov Double of the field-of-view stars can exist from the focus.
/// @param m_bar Minimum magnitude that all stars must be under.
void Experiment::present_benchmark (Chomp &ch, Star::list &big_i, Star &center, const double fov, const double m_bar) {
    Rotation q = Rotation::chance();
    
    // We require at-least five stars to exist here.
    do {
        center = Star::chance();
        big_i.clear(), big_i.reserve(static_cast<unsigned int> (fov * 4));
        
        for (const Star &s : ch.nearby_hip_stars(center, fov / 2.0, static_cast<unsigned int> (fov * 4))) {
            if (s.get_magnitude() < m_bar) {
                big_i.emplace_back(Rotation::rotate(s, q));
            }
        }
    }
    while (big_i.size() < 5);
    
    // Shuffle and rotate the center.
    std::shuffle(big_i.begin(), big_i.end(), RandomDraw::mersenne_twister);
    center = Rotation::rotate(center, q);
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
    Star::list s_c = ch.nearby_bright_stars(center, fov / 2.0, static_cast<unsigned int> (fov * 4)), s;
    std::shuffle(s_c.begin(), s_c.end(), RandomDraw::mersenne_twister);
    
    // Insert n stars to s.
    s.reserve(n);
    for (unsigned int i = 0; i < n; i++) {
        s.emplace_back(s_c[i]);
    }
    
    // Rotate all stars by a random quaternion.
    Rotation q = Rotation::chance();
    for (Star &s_i : s) {
        s_i = Rotation::rotate(s_i, q);
    }
    center = Rotation::rotate(center, q);
    
    return s;
}

/// Determine if the stars a body subset matches the given inertial labels.
///
/// @param big_i All body stars. The first |r_labels| stars will be verified.
/// @param r_ell Labels from the catalog to match.
/// @return True if the inertial labels match the body labels. False otherwise.
bool Experiment::Reduction::is_correctly_identified (const Star::list &big_i,
                                                     const Identification::labels_list &r_ell) {
    Identification::labels_list b_ell, r_ell_copy = r_ell;
    for (unsigned int i = 0; i < r_ell.size(); i++) {
        b_ell.emplace_back(big_i[i].get_label());
    }
    
    std::sort(b_ell.begin(), b_ell.end());
    std::sort(r_ell_copy.begin(), r_ell_copy.end());
    return std::equal(b_ell.begin(), b_ell.end(), r_ell_copy.begin());
}

/// Determine if the stars specified in b_ell (our inertial set) correctly matches our body set.
///
/// @param big_i All body stars. Check if b_ell correctly matches stars in this set.
/// @param b Subset of our body, but with predicted catalog labels.
/// @return True if the predicted catalog labels matches the ground truth labels (body set).
double Experiment::Map::percentage_correct (const Star::list &big_i, const Star::list &b) {
    unsigned int count = 0;
    for (const Star &b_j : big_i) {
        count += (std::find(b.begin(), b.end(), b_j) != b.end()) ? 1 : 0;
    }
    
    return count / b.size();
}