/// @file benchmark.cpp
/// @author Glenn Galvizo
///
/// Source file for Benchmark class, which generates the input data for star identification testing.

#include <algorithm>
#include <iomanip>
#include <fstream>

#include "benchmark/benchmark.h"
#include "math/random-draw.h"

/// Indicates that a given identification object does not have an designated field of view.
const double Benchmark::NO_FOV = -1;

/// The default apparent magnitude, minimum brightness from Earth.
const double Benchmark::DEFAULT_M_BAR = 6.0;

/// String of HOKU_PROJECT_PATH environment variable.
const std::string Benchmark::PROJECT_LOCATION = std::getenv("HOKU_PROJECT_PATH");

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
/// String of the current plot temp file.
const std::string Benchmark::CURRENT_TMP = std::string(std::getenv("TEMP")) + "/cuplt.tmp";

/// String of the error plot temp file.
const std::string Benchmark::ERROR_TMP = std::string(std::getenv("TEMP")) + "/errplt.tmp";
#else
/// String of the current plot temp file.
const std::string Benchmark::CURRENT_TMP = "/tmp/cuplt.tmp";

/// String of the error plot temp file.
const std::string Benchmark::ERROR_TMP = "/tmp/errplt.tmp";
#endif

/// Location of the Python benchmark plotter.
const std::string Benchmark::PLOT_SCRIPT = "\"" + PROJECT_LOCATION + "/script/python/draw_image.py\"";

/// Constructor. Generate a random focus and rotation. Scale and restrict the image using the given fov and magnitude
/// sensitivity (m_bar).
///
/// @param ch Open connection to Nibble, using Chomp tables.
/// @param fov Limit a star must be separated from the focus by.
/// @param m_bar Maximum magnitude a star must be within in the given benchmark.
Benchmark::Benchmark (Chomp &ch, const double fov, const double m_bar) {
    this->fov = fov, this->center = Star::chance(), this->q_rb = Rotation::chance();
    generate_stars(ch, m_bar);
}

/// Overloaded constructor. Uses a user defined focus and rotation. Scale and restrict the image using the given fov
/// and magnitude sensitivity (m_bar).
///
/// @param ch Open connection to Nibble, using Chomp tables.
/// @param center Focus star of the given star set.
/// @param q Quaternion to take inertial frame to body frame.
/// @param fov Limit a star must be separated from the focus by.
/// @param m_bar Maximum magnitude a star must be within in the given benchmark.
Benchmark::Benchmark (Chomp &ch, const Vector3 &center, const Rotation &q, const double fov, const double m_bar) {
    this->fov = fov, this->center = center, this->q_rb = q;
    generate_stars(ch, m_bar);
}

/// Private constructor. Directly sets the star set, fov, and focus. The rotation is unknown, as well as the errors
/// applied to it.
///
/// @param s Star set to give the current benchmark.
/// @param center Focus star of the given star set.
/// @param fov Field of view (degrees) associated with the given star set.
Benchmark::Benchmark (const Star::list &s, const Vector3 &center, const double fov) {
    this->fov = fov, this->b = s, this->center = center;
}

/// Dummy image. Holds no image or field of view.
///
/// @return A dummy image without stars.
const Benchmark Benchmark::black () {
    return Benchmark({}, Vector3::Zero(), 0);
}

/// Shuffle the current star set. Uses C++11 random library.
void Benchmark::shuffle () {
    std::shuffle(this->b.begin(), this->b.end(), RandomDraw::mersenne_twister);
}

/// Obtain a set of stars around the current focus vector. This is the 'clean' star set, meaning that all stars in
/// 'stars' accurately represent what is found in the catalog. Uses C++11 random library to shuffle.
///
/// @param m_bar Maximum magnitude a star must be within in the given benchmark.
void Benchmark::generate_stars (Chomp &ch, const double m_bar) {
    // Expected number of stars = fov * 8. Not too concerned with accuracy here.
    auto expected = static_cast<unsigned int>(this->fov * 4);
    
    // Find nearby stars. Rotate these stars and the center.
    Star::list s_l = ch.nearby_hip_stars(this->center, this->fov / 2.0, expected);
    std::for_each(s_l.begin(), s_l.end(), [this, &m_bar] (const Star &s) -> void {
        if (s.get_magnitude() <= m_bar) {
            this->b.push_back(Rotation::rotate(s, this->q_rb));
        }
    });
    this->center = Rotation::rotate(Star::wrap(this->center), this->q_rb);
    
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
    Star::list clean = this->b;
    
    std::transform(clean.begin(), clean.end(), clean.begin(), Star::reset_label);
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
        throw std::runtime_error(std::string("Unable to open current and/or error files."));
    }
    
    // Record the center.
    current_record << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed;
    current_record << this->center.data[0] << " " << this->center.data[1] << " " << this->center.data[2] << "\n";
    
    // Record the stars.
    std::for_each(this->b.begin(), this->b.end(), [&current_record] (const Star &s) -> void {
        current_record << s[0] << " " << s[1] << " " << s[2] << " " << s.get_label() << "\n";
    });
    current << current_record.str();
    
    // Record each error model, which has it's own set of stars and plot colors.
    error_record << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed;
    std::for_each(this->error_models.begin(), this->error_models.end(),
                  [&error_record] (const ErrorModel &model) -> void {
                      for (const Star &s: model.affected) {
                          error_record << s[0] << " " << s[1] << " " << s[2] << " " << s.get_label() << " "
                                       << model.plot_color << "\n";
                      }
                  });
    error << error_record.str();
    
    current.close();
    error.close();
}

/// Write the current data in the star set to a file, and let a separate Python script generate the plot. I am most
/// familiar with Python's MatPlotLib, so this seemed like the most straight-forward approach.
void Benchmark::display_plot () {
    // Field-of-view and norm are parameters to the plot script.
    std::string params =
        " q=on fov=" + std::to_string(this->fov) + " norm=" + std::to_string(Vector3::Magnitude(this->center));

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    std::string cmd = std::string("python -E ") + PLOT_SCRIPT + params;
#else
    std::string cmd = "python3 " + std::string(PLOT_SCRIPT) + params;
#endif
    
    // Record the current instance, and let Python work its magic!
    this->record_current_plot();
    if (int i = std::system(cmd.c_str())) {
        throw std::runtime_error(std::string("'python/draw_image.py' exited with an error: " + std::to_string(i)));
    }
}

/// Append n randomly placed vectors that fall within fov/2 degrees of the focus. This models stray light that may
/// randomly wander into the detector.
///
/// @param n Number of extra stars to add.
/// @param cap_error Flag to move an error star to the front of the star list.
void Benchmark::add_extra_light (const unsigned int n, bool cap_error) {
    unsigned int current_n = 0;
    ErrorModel extra_light = {"Extra Light", "r", {Star::wrap(Vector3::Zero())}};
    
    while (current_n < n) {
        Star generated = Star::chance(-current_n - 1);
        if (Star::within_angle(generated, this->center, this->fov / 2.0)) {
            this->b.emplace_back(generated);
            extra_light.affected.emplace_back(generated);
            current_n++;
        }
    }
    
    // Shuffle to maintain randomness. If desired, an error star remains at the front.
    std::iter_swap(this->b.begin(), this->b.end() - 1);
    (!cap_error) ? std::shuffle(this->b.begin() + 1, this->b.end(), std::mt19937(std::random_device()()))
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
    ErrorModel removed_light = {"Removed Light", "0.5", {Star::wrap(Vector3::Zero())}};
    bool is_affected = false;
    
    while (!is_affected) {
        // First, generate the light blocking blobs.
        while (current_n < n) {
            Star generated = Star::chance();
            if (Star::within_angle(generated, this->center, this->fov / 2.0)) {
                blobs.emplace_back(generated);
                current_n++;
            }
        }
        
        // Second, check if any of the stars fall within psi / 2 of a dark spot.
        for (unsigned int i = 0; i < blobs.size(); i++) {
            for (const Star &s : this->b) {
                if (Star::within_angle(blobs[i], s, psi / 2.0)) {
                    this->b.erase(this->b.begin() + i);
                    removed_light.affected.emplace_back(s);
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
    ErrorModel shifted_light = {"Shifted Light", "g", {Star::wrap(Vector3::Zero())}};
    unsigned int current_n = 0;
    
    // Loop through entire list again if n not met through past run.
    while (current_n < n || this->b.size() == current_n + 1) {
        bool n_condition = true;
        
        // Check inside if n is met early, break if met.
        for (unsigned int i = 0; i < this->b.size() && n_condition; i++) {
            Star candidate = Rotation::shake(this->b[i], sigma);
            
            // If shifted star is near center, add the shifted star and remove the old.
            if (Star::within_angle(candidate, this->center, this->fov / 2.0)) {
                this->b.push_back(candidate);
                this->b.erase(this->b.begin() + i);
                shifted_light.affected.emplace_back(candidate);
                current_n++;
            }
            
            // If the n-condition is met early, we break.
            n_condition = (current_n < n || this->b.size() == current_n + 1);
        }
    }
    
    // Shuffle to maintain randomness. If desired, an error star remains at the front.
    std::iter_swap(this->b.begin(), this->b.end() - 1);
    cap_error ? std::shuffle(this->b.begin() + 1, this->b.end(), std::mt19937(std::random_device()()))
              : this->shuffle();
    
    // Remove first element. Append this to the error models.
    shifted_light.affected.erase(shifted_light.affected.begin());
    this->error_models.push_back(shifted_light);
}

/// Simulate barrel distortion using the equation r_d = r_u(1 - alpha|r_u|^2). This distorts the entire image. Source
/// found here: https://stackoverflow.com/a/34743020
///
/// @param alpha Distortion parameter associated with the barrel. Smaller alpha -> stars moved away from image center.
void Benchmark::barrel_light (double alpha) {
    // TODO: Finish this method. Need to visually check output.
    ErrorModel barreled_light = {"Barreled Light", "y", {}};
    
    std::transform(this->b.begin(), this->b.end(), this->b.begin(), [this, alpha] (const Star &s) -> Star {
        // Determine the distance our current star must be from the center.
        double u = (180.0 / M_PI) * Star::Angle(s, this->center);
        double d = u * (1 - alpha * u * u);
        
        return Rotation::slerp(s, this->center, d);
    });
    
    // Append to our error models.
    this->error_models.push_back(barreled_light);
}