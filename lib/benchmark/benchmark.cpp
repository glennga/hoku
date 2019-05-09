/// @file benchmark.cpp
/// @author Glenn Galvizo
///
/// Source file for Benchmark class, which generates the input data for star identification testing.

#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <libgen.h>

#include "benchmark/benchmark.h"
#include "math/random-draw.h"

/// Indicates that a given identification object does not have an designated field of view.
const double Benchmark::NO_FOV = -1;

/// The default apparent magnitude, minimum brightness from Earth.
const double Benchmark::DEFAULT_M_BAR = 6.0;

/// Constructor. Generate a random focus and rotation. Scale and restrict the image using the given fov and magnitude
/// sensitivity (m_bar).
///
/// @param ch Open connection to Nibble, using Chomp tables.
/// @param fov Limit a star must be separated from the focus by.
/// @param m_bar Maximum magnitude a star must be within in the given benchmark.
Benchmark::Benchmark (Chomp &ch, const double fov, const double m_bar) {
    this->fov = fov, this->center = Star::chance().get_vector(), this->q_rb = Rotation::chance();
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
    this->fov = fov, this->b = std::make_shared<Star::list>(s), this->center = center;
}

/// Dummy image. Holds no image or field of view.
///
/// @return A dummy image without stars.
const Benchmark Benchmark::black () {
    return Benchmark({}, Vector3::Zero(), 0);
}

/// Access method for the b list components. Overloads the [] operator.
///
/// @param n Star of b to return.
/// @return Star located at index n inside B.
Star Benchmark::operator[] (const unsigned int n) const {
    return (*this->b)[n];
}

/// Shuffle the current star set. Uses C++11 random library.
void Benchmark::shuffle () {
    std::shuffle(this->b->begin(), this->b->end(), RandomDraw::mersenne_twister);
}

/// Obtain a set of stars around the current focus vector. This is the 'clean' star set, meaning that all stars in
/// 'stars' accurately represent what is found in the catalog. Uses C++11 random library to shuffle.
///
/// @param m_bar Maximum magnitude a star must be within in the given benchmark.
void Benchmark::generate_stars (Chomp &ch, const double m_bar) {
    // Expected number of stars = fov * 8. Not too concerned with accuracy here.
    auto expected = static_cast<unsigned int>(this->fov * 4);
    this->b = std::make_shared<Star::list>();

    // Find nearby stars. Rotate these stars and the center.
    Star::list s_l = ch.nearby_hip_stars(this->center, this->fov / 2.0, expected);
    std::for_each(s_l.begin(), s_l.end(), [this, &m_bar] (const Star &s) -> void {
        if (s.get_magnitude() <= m_bar) {
            this->b->push_back(Rotation::rotate(s, this->q_rb));
        }
    });
    this->center = Rotation::rotate(Star::wrap(this->center), this->q_rb).get_vector();

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
    Star::list clean = (*this->b);

    std::transform(clean.begin(), clean.end(), clean.begin(), Star::reset_label);
    return clean;
}

/// Modify the given image_s and image_fov to the current stars and fov fields. These are all points that define an
/// image, and are required for all identification methods.
///
/// @param image_s Reference to the pointer that will hold the star list to benchmark star set.
/// @param image_fov Reference to the double to set as the fov.
void Benchmark::present_image (std::unique_ptr<Star::list> &image_s, double &image_fov) const {
    image_fov = this->fov;
    image_s = std::make_unique<Star::list>(clean_stars());
}

/// Write the current data in the star set to two files. This includes the fov, focus, star set, and the error set.
void Benchmark::record_current_plot () {
    std::ofstream current("/tmp/cuplt.tmp");
    std::ofstream error("/tmp/errplt.tmp");
    std::ostringstream current_record, error_record;

    // Do not record if files are unable to open.
    if (!current || !error) {
        throw std::runtime_error(std::string("Unable to open current and/or error files."));
    }

    // Record the center.
    current_record << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::fixed;
    current_record << this->center.data[0] << " " << this->center.data[1] << " " << this->center.data[2] << "\n";

    // Record the stars.
    std::for_each(this->b->begin(), this->b->end(), [&current_record] (const Star &s) -> void {
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
    // Field-of-view is a parameter to the plot script.
    std::string params =
            " q=on fov=" + std::to_string(this->fov);

    std::string cmd = "python3 " + std::string(dirname(const_cast<char *>(__FILE__)))
            + "/../../script/python/draw_image.py " + params;

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
/// @param shuffle Flag to enable reshuffling of the I set.
void Benchmark::add_extra_light (const unsigned int n, bool shuffle) {
    unsigned int current_n = 0;
    ErrorModel extra_light = {"Extra Light", "r", {Star::wrap(Vector3::Zero())}};

    while (current_n < n) {
        Star generated = Star::chance(-current_n - 1);
        if (Star::within_angle(generated, this->center, this->fov / 2.0)) {
            this->b->emplace_back(generated);
            extra_light.affected.emplace_back(generated);
            current_n++;
        }
    }

    // Shuffle to maintain randomness (if desired).
    if (shuffle) {
        this->shuffle();
    }

    // Remove the first element. Append to error models.
    extra_light.affected.erase(extra_light.affected.begin());
    this->error_models.emplace_back(extra_light);
}

/// Randomly generate n dark spots. All stars in the input that are within psi/2 degrees of the blobs are removed.
/// This model simulates celestial bodies that may block light from the detector.
///
/// @param n Number of blobs to generate.
/// @param psi Dark spot size (degrees). This is the cone size of the dark spot vectors.
void Benchmark::remove_light (const unsigned int n, const double psi, const bool shuffle) {
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
        for (const Star &blob : blobs) {
            auto s = this->b->begin();

            while (s != this->b->end()) {
                if (Star::within_angle(blob, *s, psi / 2.0)) {
                    removed_light.affected.emplace_back(*s);
                    s = this->b->erase(s);
                }
                else {
                    ++s;
                }
            }
        }

        // Only when we have some stars affected, we exit out of this loop. We always expect to remove some stars.
        is_affected = !removed_light.affected.empty();
    }

    // Shuffle to maintain randomness (if desired).
    if (shuffle) {
        this->shuffle();
    }

    // Remove first element. Append this to the error models.
    removed_light.affected.erase(removed_light.affected.begin());
    this->error_models.emplace_back(removed_light);
}

/// Randomly move n vectors based off the given sigma. Change is normally distributed, with 0 change being the most
/// expected and having a standard deviation of sigma.
///
/// @param n Number of stars to move.
/// @param sigma Amount to shift stars by, in terms of degrees.
/// @param shuffle Flag to enable reshuffling of the I set.
void Benchmark::shift_light (const unsigned int n, const double sigma, bool shuffle) {
    ErrorModel shifted_light = {"Shifted Light", "g", {Star::wrap(Vector3::Zero())}};
    unsigned int current_n = 0;

    for (unsigned int i = 0; i < this->b->size() && current_n < n; i++) {
        Star candidate;

        // Ensure that the shifted star does not veer out of focus.
        do {
            candidate = Rotation::shake(this->b->at(i), sigma);
        } while (!Star::within_angle(candidate, this->center, this->fov / 2.0));

        // Modify our star set.
        this->b->at(i) = candidate, current_n++;
        shifted_light.affected.emplace_back(candidate);
    }

    // Shuffle to maintain randomness (if desired).
    if (shuffle) {
        this->shuffle();
    }

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
//    ErrorModel barreled_light = {"Barreled Light", "y", {}};
//
//    barreled_light.affected.resize(this->b->size());
//    std::transform(this->b->begin(), this->b->end(), this->b->begin(),
//            [this, alpha, &barreled_light] (const Star &s) -> Star {
////        // Determine the distance our current star must be from the center.
////        double r = Star::Angle(s.get_vector(), this->center);
////        double t = ((1.0 - alpha) * r);
//
//        Star modified = Rotation::slerp(s, this->center, alpha);
//        barreled_light.affected.emplace_back(modified);
//        return modified;
//    });
//
//    // Append to our error models.
//    this->error_models.push_back(barreled_light);
}
