/// @file benchmark.cpp
/// @author Glenn Galvizo
///
/// Source file for Benchmark class, which generates the input data for star identification testing.


#include "benchmark/benchmark.h"

/// Constructor. Generate a random focus and rotation. Scale and restrict the image using the given fov and magnitude
/// sensitivity (m_bar).
///
/// @param ch Open connection to Nibble, using Chomp tables.
/// @param seed Reference to a random device to use for all future Benchmark methods.
/// @param fov Limit a star must be separated from the focus by.
/// @param m_bar Maximum magnitude a star must be within in the given benchmark.
Benchmark::Benchmark (Chomp &ch, std::random_device &seed, const double fov, const double m_bar) {
    this->fov = fov, this->focus = Star::chance(seed), this->inertial_to_image = Rotation::chance(seed);
    this->seed = &seed;
    
    generate_stars(ch, m_bar);
}

/// Overloaded constructor. Uses a user defined focus and rotation. Scal and restrict the image using the given fov
/// and magnitude sensitivity (m_bar).
///
/// @param ch Open connection to Nibble, using Chomp tables.
/// @param seed Pointer to a random device to use for all future Benchmark methods.
/// @param focus Focus star of the given star set.
/// @param q Quaternion to take inertial frame to body frame.
/// @param fov Limit a star must be separated from the focus by.
/// @param m_bar Maximum magnitude a star must be within in the given benchmark.
Benchmark::Benchmark (Chomp &ch, std::random_device &seed, const Star &focus, const Rotation &q, const double fov,
                      const double m_bar) {
    this->fov = fov, this->focus = focus, this->inertial_to_image = q, this->seed = &seed;
    generate_stars(ch, m_bar);
}

/// Private constructor. Directly sets the star set, fov, and focus. The rotation is unknown, as well as the errors
/// applied to it.
///
/// @param seed Reference to a random device to use for all future Benchmark methods.
/// @param s Star set to give the current benchmark.
/// @param focus Focus star of the given star set.
/// @param fov Field of view (degrees) associated with the given star set.
Benchmark::Benchmark (std::random_device &seed, const Star::list &s, const Star &focus, const double fov) : seed(),
    fov() {
    this->seed = &seed, this->fov = fov, this->stars = s, this->focus = focus;
}

/// Dummy image. Holds no image or field of view.
///
/// @return A dummy image without stars.
const Benchmark Benchmark::black () {
    std::random_device rd;
    return Benchmark(rd, {}, Star::zero(), 0);
}

/// Shuffle the current star set. Uses C++11 random library.
void Benchmark::shuffle () {
    std::mt19937_64 mersenne_twister((*seed)());
    std::shuffle(this->stars.begin(), this->stars.end(), mersenne_twister);
}

/// Obtain a set of stars around the current focus vector. This is the 'clean' star set, meaning that all stars in
/// 'stars' accurately represent what is found in the catalog. Uses C++11 random library to shuffle.
///
/// @param m_bar Maximum magnitude a star must be within in the given benchmark.
void Benchmark::generate_stars (Chomp &ch, const double m_bar) {
    // Expected number of stars = fov * 8. Not too concerned with accuracy here.
    auto expected = static_cast<unsigned int>(this->fov * 4);
    
    // Find nearby stars. Rotate these stars and the focus.
    for (const Star &s : ch.nearby_hip_stars(this->focus, this->fov / 2.0, expected)) {
        if (s.get_magnitude() <= m_bar) {
            this->stars.push_back(Rotation::rotate(s, this->inertial_to_image));
        }
    }
    this->focus = Rotation::rotate(this->focus, this->inertial_to_image);
    
    // Shuffle to maintain randomness.
#if !defined ENABLE_TESTING_ACCESS
    this->shuffle();
#endif
}

/// Return the current star set with all catalog IDs set to Star::NO_LABEL. In practice, the catalog ID of a star set is
/// never given from the image itself.
///
/// @return Copy of current star set with catalog IDs set to Star::NO_LABEL.
Star::list Benchmark::clean_stars () const {
    // Keep the current star set intact.
    Star::list clean = this->stars;
    
    for (Star &s : clean) {
        s = Star::reset_label(s);
    }
    return clean;
}

/// Modify the given image_s and image_fov to the current stars and fov fields. These are all points that define an
/// image, and are required for all identification methods.
///
/// @param image_s Reference to the star list to benchmark star set.
/// @param image_fov Reference to the double to set as the fov.
void Benchmark::present_image (Star::list &image_s, double &image_fov) const {
    image_fov = this->fov;
    image_s = clean_stars();
}

/// Write the current data in the star set to two files. This includes the fov, norm, focus, star set, and the
/// error set.
void Benchmark::record_current_plot () {
    std::ofstream current(CURRENT_TMP), error(ERROR_TMP);
    std::ostringstream current_record, error_record;
    
    // Do not record if files are unable to open.
    if (!current || !error) {
        throw "Unable to open current and/or error files.";
    }
    
    // Record the fov and norm first.
    current_record << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed << this->fov << '\n'
                   << this->focus.norm() << '\n';
    
    // Record the focus second.
    for (const double &component : {this->focus[0], this->focus[1], this->focus[2]}) {
        current_record << component << " ";
    }
    current_record << "\n";
    
    // Record the rest of the stars.
    for (const Star &s : this->stars) {
        current_record << s[0] << " " << s[1] << " " << s[2] << " " << s.get_label() << "\n";
    }
    current << current_record.str();
    
    // Record each error model, which has it's own set of stars and plot colors.
    error_record << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed;
    for (const ErrorModel &model: this->error_models) {
        for (const Star &s: model.affected) {
            error_record << s[0] << " " << s[1] << " " << s[2] << " " << s.get_label() << " " << model.plot_color
                         << "\n";
        }
    }
    error << error_record.str();
    
    current.close();
    error.close();
}

/// Write the current data in the star set to a file, and let a separate Python script generate the plot. I am most
/// familiar with Python's MatPlotLib, so this seemed like the most straight-forward approach.
void Benchmark::display_plot () {
    std::remove(CURRENT_TMP.c_str());
    std::remove(ERROR_TMP.c_str());

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    std::string cmd = std::string("python ") + PLOT_SCRIPT;
#else
    std::string cmd = "python3 " + std::string(PLOT_SCRIPT);
#endif
    
    if (std::ifstream(CURRENT_TMP) || std::ifstream(ERROR_TMP)) {
        throw "Current and/or error plot files could not deleted.";
    }
    
    // Record the current instance, and let Python work its magic!
    this->record_current_plot();
    std::system(cmd.c_str());
}

/// Compare the number of matching stars that exist between the two stars sets.
///
/// @param b Benchmark containing star list to compare with s_l.
/// @param s_l Star list to compare with B.
/// @return The number of stars found matching both lists.
int Benchmark::compare_stars (const Benchmark &b, const Star::list &s_l) {
    Star::list s_candidates = s_l;
    unsigned int c = 0;
    
    for (const Star &s_a : b.stars) {
        for (unsigned int i = 0; i < s_candidates.size(); i++) {
            
            // If we find b match, erase this from our candidates list.
            if (s_a == s_candidates[i]) {
                s_candidates.erase(s_candidates.begin() + i);
                c++;
            }
        }
    }
    
    return c;
}

/// Append n randomly placed vectors that fall within fov/2 degrees of the focus. This models stray light that may
/// randomly wander into the detector.
///
/// @param n Number of extra stars to add.
/// @param cap_error Flag to move an error star to the front of the star list.
void Benchmark::add_extra_light (const unsigned int n, bool cap_error) {
    unsigned int current_n = 0;
    ErrorModel extra_light = {"Extra Light", "r", {Star::zero()}};
    
    while (current_n < n) {
        Star generated = Star::chance(*seed, -current_n - 1);
        if (Star::within_angle(generated, this->focus, this->fov / 2.0)) {
            this->stars.emplace_back(generated);
            extra_light.affected.emplace_back(generated);
            current_n++;
        }
    }
    
    // Shuffle to maintain randomness. If desired, an error star remains at the front.
    std::iter_swap(this->stars.begin(), this->stars.end() - 1);
    (!cap_error) ? std::shuffle(this->stars.begin() + 1, this->stars.end(), std::mt19937(std::random_device()()))
                 : this->shuffle();
    
    // Remove the first element. Append to error models.
    extra_light.affected.erase(extra_light.affected.begin());
    this->error_models.emplace_back(extra_light);
}

/// Randomly generate n dark spots. All stars in the input that are within psi/2 degrees of the blobs are removed.
/// This model simulates celestial bodies that may block light from the detector.
///
/// @param n Number of blobs to generate.
/// @param psi Dark spot size (degrees). This is the cone size of the dark spot vectors.
void Benchmark::remove_light (const unsigned int n, const double psi) {
    unsigned int current_n = 0;
    std::vector<Star> blobs;
    ErrorModel removed_light = {"Removed Light", "0.5", {Star::zero()}};
    bool is_affected = false;
    
    while (!is_affected) {
        // First, generate the light blocking blobs.
        while (current_n < n) {
            Star generated = Star::chance(*seed);
            if (Star::within_angle(generated, this->focus, this->fov / 2.0)) {
                blobs.emplace_back(generated);
                current_n++;
            }
        }
        
        // Second, check if any of the stars fall within psi / 2 of a dark spot.
        for (unsigned int i = 0; i < blobs.size(); i++) {
            for (const Star &star : this->stars) {
                if (Star::within_angle(blobs[i], star, psi / 2.0)) {
                    this->stars.erase(this->stars.begin() + i);
                    removed_light.affected.emplace_back(star);
                }
            }
        }
        
        // Only when we have some stars affected, we exit out of this loop.
        is_affected = !removed_light.affected.empty();
    }
    
    // Shuffle to maintain randomness.
    this->shuffle();
    
    // Remove first element. Append this to the error models.
    removed_light.affected.erase(removed_light.affected.begin());
    this->error_models.emplace_back(removed_light);
}

/// Randomly move n vectors based off the given sigma. Change is normally distributed, with 0 change being the most
/// expected and having a standard deviation of sigma.
///
/// @param n Number of stars to move.
/// @param sigma Amount to shift stars by, in terms of degrees.
/// @param cap_error Flag to move an error star to the front of the star list.
void Benchmark::shift_light (const unsigned int n, const double sigma, bool cap_error) {
    ErrorModel shifted_light = {"Shifted Light", "g", {Star::zero()}};
    unsigned int current_n = 0;
    
    // Loop through entire list again if n not met through past run.
    while (current_n < n || this->stars.size() == current_n + 1) {
        bool n_condition = true;
        
        // Check inside if n is met early, break if met.
        for (unsigned int i = 0; i < this->stars.size() && n_condition; i++) {
            Star candidate = Rotation::shake(this->stars[i], sigma, *seed);
            
            // If shifted star is near focus, add the shifted star and remove the old.
            if (Star::within_angle(candidate, this->focus, this->fov / 2.0)) {
                this->stars.push_back(candidate);
                this->stars.erase(this->stars.begin() + i);
                shifted_light.affected.emplace_back(candidate);
                current_n++;
            }
            
            // If the n-condition is met early, we break.
            n_condition = (current_n < n || this->stars.size() == current_n + 1);
        }
    }
    
    // Shuffle to maintain randomness. If desired, an error star remains at the front.
    std::iter_swap(this->stars.begin(), this->stars.end() - 1);
    cap_error ? std::shuffle(this->stars.begin() + 1, this->stars.end(), std::mt19937(std::random_device()()))
              : this->shuffle();
    
    // Remove first element. Append this to the error models.
    shifted_light.affected.erase(shifted_light.affected.begin());
    this->error_models.push_back(shifted_light);
}