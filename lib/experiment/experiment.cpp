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
/// @param seed Random seed to use for rotation and focus star.
/// @param body Reference to location of the body stars.
/// @param focus Reference to location of the focus star.
/// @param m_bar Minimum magnitude that all stars must be under.
void Experiment::present_benchmark (Chomp &ch, std::random_device &seed, Star::list &body, Star &focus,
                                    const double m_bar) {
    std::mt19937_64 mersenne_twister(seed());
    Rotation q = Rotation::chance(seed);
    
    // We require at-least five stars to exist here.
    do {
        focus = Star::chance(seed);
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
    std::shuffle(body.begin(), body.end(), mersenne_twister);
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
/// @param seed Seed used to generate random stars with.
/// @param focus Reference to center used to query Nibble with.
/// @return List of n stars who are within fov degrees from each other.
Star::list Experiment::Query::generate_n_stars (Chomp &ch, const unsigned int n, std::random_device &seed,
                                                Star &focus) {
    std::mt19937_64 mersenne_twister(seed());
    
    // Find all stars near some random focus.
    focus = Star::chance(seed);
    Star::list s_c = ch.nearby_bright_stars(focus, WORKING_FOV / 2.0, static_cast<unsigned int> (WORKING_FOV * 4)), s;
    std::shuffle(s_c.begin(), s_c.end(), mersenne_twister);
    
    // Insert n stars to s.
    s.reserve(n);
    for (unsigned int i = 0; i < n; i++) {
        s.emplace_back(s_c[i]);
    }
    
    // Rotate all stars by a random quaternion.
    Rotation q = Rotation::chance(seed);
    for (Star &s_i : s) {
        s_i = Rotation::rotate(s_i, q);
    }
    focus = Rotation::rotate(focus, q);
    
    return s;
}

/// Generate a set of stars near a random focus (inertial set), rotate it with some random rotation Q to get our
/// body set. The same stars share indices between the inertial and body set.
///
/// @param ch Open Nibble connection using Chomp methods.
/// @param seed Random seed to use for rotation and focus star.
/// @param body Reference to the location to store the body list.
/// @param inertial Reference to the location to store the inertial set of stars.
/// @param q Reference to the location to store the rotation.
/// @param focus Reference to the location to store the focus star.
/// @param m_bar Minimum magnitude that all stars must be under.
void Experiment::FirstAlignment::present_stars (Chomp &ch, std::random_device &seed, Star::list &body,
                                                Star::list &inertial, Rotation &q, Star &focus, const double m_bar) {
    std::mt19937_64 mersenne_twister(seed());
    q = Rotation::chance(seed);
    
    // We require at-least five stars to exist here.
    do {
        focus = Star::chance(seed);
        
        inertial.clear(), inertial.reserve(static_cast<unsigned int>(WORKING_FOV * 4));
        for (const Star &s : ch.nearby_hip_stars(focus, WORKING_FOV / 2.0,
                                                 static_cast<unsigned int>(WORKING_FOV * 4))) {
            if (s.get_magnitude() < m_bar) {
                inertial.emplace_back(s);
            }
        }
    }
    while (inertial.size() < 5);
    
    // Shuffle our inertial, then rotate our inertial set to get our body set.
    std::shuffle(inertial.begin(), inertial.end(), mersenne_twister);
    body.clear(), body.reserve(inertial.size());
    for (const Star &s : inertial) {
        body.emplace_back(Rotation::rotate(s, q));
    }
}

/// Generate gaussian noise for the first n body stars. Noise is distributed given shift_sigma.
///
/// @param seed Random seed to use for rotation and focus star.
/// @param candidates Reference to the location to store the candidate list.
/// @param shift_sigma Sigma for gaussian noise to apply. Defaults to 0.
/// @param shift_n Number of stars to shift. Shifts from the front of the list.
void Experiment::FirstAlignment::shift_body (std::random_device &seed, Star::list &candidates, const double shift_sigma,
                                             const int shift_n) {
    if (shift_sigma != 0) {
        // Starting from the front of our list, apply noise. **Assumes that list generated is bigger than shift_n.
        for (int i = 0; i < shift_n; i++) {
            candidates[i] = Rotation::shake(candidates[i], shift_sigma, seed);
        }
    }
}

/// Determine if the stars returned by a first alignment experiment have the correct labels.
///
/// @param w Reference to the location of our predicated body set.
/// @param body_j Reference to our ground truth, our actual body subset (subset of the complete body set).
/// @return True if body_j shares mutual information with w. False otherwise.
bool Experiment::FirstAlignment::is_correctly_aligned (Star::list &w, Star::list &body_j) {
    auto compare_labels = [] (const Star &s_1, const Star &s_2) -> bool {
        return s_1.get_label() < s_2.get_label();
    };
    
    // Sort each list by their labels, and determine if they are equal.
    std::sort(w.begin(), w.end(), compare_labels), std::sort(body_j.begin(), body_j.end(), compare_labels);
    return std::equal(w.begin(), w.end(), body_j.begin());
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

/// Return the percentage of correctly identified stars in our original body set and their labels. Any false stars
/// defined in the labeled body set are counted as incorrectly labeled.
///
/// @param body All body stars. Check if b_ell correctly matches stars in this set.
/// @param b_ell Subset of body stars with predicted catalog labels.
/// @return True if the predicted catalog labels matches the ground truth labels (body set).
double Experiment::Crown::is_correctly_matched (const Star::list &body, const Star::list &b_ell) {
    unsigned int c = 0;
    
    for (const Star &b : body) {
        // If a false star is not labeled in our predicted set, then we count this as a matched.
        if (b.get_label() <= 0) {
            c += (std::find_if(b_ell.begin(), b_ell.end(), [&b] (const Star &b_i) -> bool {
                return Star::within_angle(b_i, b, 0);
            }) == b_ell.end()) ? 1 : 0;
        }
        else {
            // For all other stars, check the label.
            c += (std::find_if(b_ell.begin(), b_ell.end(), [&b] (const Star &b_i) -> bool {
                return b_i == b && b.get_label() > 0;
            }) != b_ell.end()) ? 1 : 0;
        }
    }
    
    return c / body.size();
}
