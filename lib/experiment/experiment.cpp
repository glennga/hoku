/// @file crown.cpp
/// @author Glenn Galvizo
///
/// Source file for all trials. This holds the namespaces of functions that allow us to test various methods and log
/// the data.

#include "experiment/experiment.h"

/// Generate the benchmark to be used for identification. Restricts which stars to include based on the given camera
/// sensitivity (m_bar).
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param body Reference to location of the body stars.
/// @param focus Reference to location of the focus star.
/// @param m_bar Minimum magnitude that all stars must be under.
void Experiment::present_benchmark (Chomp &ch, Star::list &body, Star &focus, const double m_bar) {
    Rotation q = Rotation::chance();
    
    // We require at-least five stars to exist here.
    do {
        focus = Star::chance();
        body.clear(), body.reserve(static_cast<unsigned int> (WORKING_FOV * 4));
        
        for (const Star &s : ch.nearby_hip_stars(focus, WORKING_FOV / 2.0,
                                                 static_cast<unsigned int> (WORKING_FOV * 4))) {
            if (s.get_magnitude() < m_bar) {
                body.emplace_back(Rotation::rotate(s, q));
            }
        }
    }
    while (body.size() < 5);
    
    // Shuffle and rotate the focus.
    std::shuffle(body.begin(), body.end(), RandomDraw::mersenne_twister);
    focus = Rotation::rotate(focus, q);
}

/// Return true if the given body set exists somewhere in a collection of reference sets.
///
/// @param r_set Reference to the reference frame sets. An in-sort place will occur for each element.
/// @param b Reference to the body frame set. An in-sort place will occur.
/// @return True if 'b' exists somewhere in r.
bool Experiment::Query::set_existence (std::vector<Identification::labels_list> &r_set,
                                       Identification::labels_list &b) {
    for (Identification::labels_list &r_bar : r_set) {
        std::sort(r_bar.begin(), r_bar.end()), std::sort(b.begin(), b.end());
        if (std::equal(r_bar.begin(), r_bar.end(), b.begin())) {
            return true;
        }
    }
    
    return false;
}

/// Generate N random stars that fall within the specified field-of-view. Rotate this result by some random quaternion.
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param n Number of stars to generate.
/// @param focus Reference to center used to query Nibble with.
/// @return List of n stars who are within fov degrees from each other.
Star::list Experiment::Query::generate_n_stars (Chomp &ch, const unsigned int n, Star &focus) {
    // Find all stars near some random focus.
    focus = Star::chance();
    Star::list s_c = ch.nearby_bright_stars(focus, WORKING_FOV / 2.0, static_cast<unsigned int> (WORKING_FOV * 4)), s;
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
    focus = Rotation::rotate(focus, q);
    
    return s;
}

/// Determine if the stars a body subset matches the given inertial labels.
///
/// @param body All body stars. The first |r_labels| stars will be verified.
/// @param r_labels Labels from the catalog to match.
/// @return True if the inertial labels match the body labels. False otherwise.
bool Experiment::Reduction::is_correctly_identified (const Star::list &body,
                                                     const Identification::labels_list &r_labels) {
    Identification::labels_list body_labels, inertial_labels = r_labels;
    for (unsigned int i = 0; i < r_labels.size(); i++) {
        body_labels.emplace_back(body[i].get_label());
    }
    
    std::sort(body_labels.begin(), body_labels.end());
    std::sort(inertial_labels.begin(), inertial_labels.end());
    return std::equal(body_labels.begin(), body_labels.end(), inertial_labels.begin());
}

/// Determine if the stars specified in b_ell (our inertial set) correctly matches our body set.
///
/// @param body All body stars. Check if b_ell correctly matches stars in this set.
/// @param b_ell Subset of our body, but with predicted catalog labels.
/// @return True if the predicted catalog labels matches the ground truth labels (body set).
bool Experiment::Alignment::is_correctly_aligned (const Star::list &body, const Star::list &b_ell) {
    unsigned int c = 0;
    for (const Star &b : body) {
        c += (std::find(b_ell.begin(), b_ell.end(), b) != b_ell.end()) ? 1 : 0;
    }
    
    return c == b_ell.size();
}