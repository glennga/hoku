/// @file benchmark.cpp
/// @author Glenn Galvizo
///
/// Source file for Benchmark class, which generates the input data for star identification testing.

#include "benchmark.h"

/// Constructor. Sets the fov, focus, and rotation of the star set. Generate the stars after collecting this
/// information.
///
/// @param fov Limit a star must be separated from the focus by.
/// @param focus All stars will be near this vector.
/// @param inertial_to_image Rotation to move stars from inertial to image.
Benchmark::Benchmark (const double fov, const Star &focus, const Rotation &inertial_to_image) {
    this->fov = fov, this->focus = focus, this->inertial_to_image = inertial_to_image;
    generate_stars();
}

/// Shuffle the current star set. Uses C++11 random library.
void Benchmark::shuffle () {
    // Need to keep random device static to avoid starting with same seed.
    static std::random_device seed;
    static std::mt19937_64 mersenne_twister(seed());
    
    std::shuffle(this->stars.begin(), this->stars.end(), mersenne_twister);
}

/// Obtain a set of stars around the current focus vector. This is the 'clean' star set, meaning that all stars in
/// 'stars' accurately represent what is found in the catalog. Uses C++11 random library to shuffle.
void Benchmark::generate_stars () {
    // Expected number of stars = fov * 4. Not too concerned with accuracy here.
    unsigned int expected = (unsigned int) this->fov * 4;
    
    // Find nearby stars. Rotate these stars and the focus.
    for (const Star &s : Nibble().nearby_stars(this->focus, this->fov / 2.0, expected)) {
        this->stars.push_back(Rotation::rotate(s, this->inertial_to_image));
    }
    this->focus = Rotation::rotate(this->focus, this->inertial_to_image);
    
    // Shuffle to maintain randomness.
    this->shuffle();
}

/// Return the current star set with all HR numbers set to 0. In practice, the Hr number of a star set is never given
/// from the image itself.
///
/// @return Copy of current star set with HR numbers set to 0.
Star::list Benchmark::clean_stars () const {
    // Keep the current star set intact.
    Star::list clean = this->stars;
    
    for (unsigned int i = 0; i < clean.size(); i++) {
        clean[i] = Star::reset_hr(clean[i]);
    }
    return clean;
}

/// Modify the given image_s, image_focus, and image_fov to the current stars, focus, and fov fields. These are all
/// points that define an image, and are required for all identification methods.
///
/// @param image_s Reference to the star list to benchmark star set.
/// @param image_focus Reference to the star to set as the focus star.
/// @param image_fov Reference to the double to set as the fov.
void Benchmark::present_image (Star::list &image_s, Star &image_focus, double &image_fov) const {
    image_focus = this->focus, image_fov = this->fov;
    image_s = clean_stars();
}

/// Write the current data in the star set to two files. This includes the fov, norm, focus, star set, and the
/// error set.
///
/// @return -1 if any files were unable to obtain. 0 otherwise.
int Benchmark::record_current_plot () {
    std::ofstream current(this->CURRENT_PLOT), error(this->ERROR_PLOT);
    std::ostringstream current_record, error_record;
    
    // Do not record if files are unable to open.
    if (!current || !error) {
        return -1;
    }
    
    // Record the fov and norm first.
    current_record << std::setprecision(16) << std::fixed << this->fov << "\n" << this->focus.norm() << "\n";
    
    // Record the focus second.
    for (const double &component : {this->focus[0], this->focus[1], this->focus[2]}) {
        current_record << component << " ";
    }
    current_record << "\n";
    
    // Record the rest of the stars.
    for (const Star &s : this->stars) {
        current_record << s[0] << " " << s[1] << " " << s[2] << " " << s.get_hr() << "\n";
    }
    current << current_record.str();
    
    // Record each error model, which has it's own set of stars and plot colors.
    for (const ErrorModel &model: this->error_models) {
        for (const Star &s: model.affected) {
            error_record << s[0] << " " << s[1] << " " << s[2] << " " << s.get_hr() << " " << model.plot_color << "\n";
        }
    }
    error << error_record.str();
    
    current.close();
    error.close();
    return 0;
}

/// Write the current data in the star set to a file, and let a separate Python script generate the plot. I am most
/// familiar with Python's MatPlotLib, so this seemed like the most straight-forward approach.
///
/// @return -1 if the previous files could not be deleted. 0 otherwise.
int Benchmark::display_plot () {
    std::remove(this->CURRENT_PLOT.c_str());
    std::remove(this->ERROR_PLOT.c_str());
    
    std::string cmd = "python " + this->PLOT_SCRIPT;
    if (std::ifstream(this->CURRENT_PLOT.c_str()) || std::ifstream(this->ERROR_PLOT.c_str())) {
        return -1;
    }
    
    // Record the current instance, and let Python work its magic!
    this->record_current_plot();
    std::system(cmd.c_str());
    return 0;
}

/// Append n randomly placed vectors that fall within fov/2 degrees of the focus. This models stray light that may
/// randomly wander into the detector.
///
/// @param n Number of extra stars to add.
void Benchmark::add_extra_light (const int n) {
    int current_n = 0;
    ErrorModel extra_light = {"Extra Light", "r", {Star::zero()}};
    
    while (current_n < n) {
        Star generated = Star::chance(-current_n - 1);
        if (Star::within_angle(generated, this->focus, this->fov / 2.0)) {
            this->stars.emplace_back(generated);
            extra_light.affected.emplace_back(generated);
            current_n++;
        }
    }
    
    // Shuffle to maintain randomness.
    this->shuffle();
    
    // Remove the first element. Append to error models.
    extra_light.affected.erase(extra_light.affected.begin());
    this->error_models.emplace_back(extra_light);
}

/// Randomly generate n dark spots. All stars in the input that are within psi/2 degrees of the blobs are removed.
/// This model simulates celestial bodies that may block light from the detector.
///
/// @param n Number of blobs to generate.
/// @param psi Dark spot size (degrees). This is the cone size of the dark spot vectors.
void Benchmark::remove_light (const int n, const double psi) {
    int current_n = 0;
    std::vector<Star> blobs;
    ErrorModel removed_light = {"Removed Light", "0.5", {Star::zero()}};
    
    // First, generate the light blocking blobs.
    while (current_n < n) {
        Star generated = Star::chance();
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
/// @param sigma Amount to shift stars by, in terms of XYZ coordinates.
void Benchmark::shift_light (const int n, const double sigma) {
    // Need to keep random device static to avoid starting with same seed.
    static std::random_device seed;
    static std::mt19937_64 mersenne_twister(seed());
    std::normal_distribution<double> dist(0, sigma);
    
    ErrorModel shifted_light = {"Shifted Light", "g", {Star::zero()}};
    int current_n = 0;
    
    // Loop through entire list again if n not met through past run.
    while (current_n < n) {
        // Check inside if n is met early, break if met.
        for (unsigned int i = 0; i < this->stars.size() && current_n < n; i++) {
            Star candidate = Star(this->stars[i][0] + dist(mersenne_twister),
                                  this->stars[i][1] + dist(mersenne_twister),
                                  this->stars[i][2] + dist(mersenne_twister), this->stars[i].get_hr()).as_unit();
            
            // If shifted star is near focus, add the shifted star and remove the old.
            if (Star::within_angle(candidate, this->focus, this->fov / 2.0)) {
                this->stars.push_back(candidate);
                this->stars.erase(this->stars.begin() + i);
                shifted_light.affected.emplace_back(candidate);
                current_n++;
            }
        }
    }
    
    // Shuffle to maintain randomness.
    this->shuffle();
    
    // Remove first element. Append this to the error models.
    shifted_light.affected.erase(shifted_light.affected.begin());
    this->error_models.push_back(shifted_light);
}