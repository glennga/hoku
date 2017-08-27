/// @file benchmark.cpp
/// @author Glenn Galvizo
///
/// Source file for Benchmark class, which generates the input data for star identification testing.

#include "benchmark.h"

/// Name of the Nibble table all results are saved to.
const std::string Benchmark::TABLE_NAME = "BENCH";

/// String of the HOKU_PROJECT_PATH environment variable
const std::string Benchmark::PROJECT_LOCATION = std::string(std::getenv("HOKU_PROJECT_PATH"));

/// Path of the 'clean' star set on disk. One of the temporary files used for plotting.
const std::string Benchmark::CURRENT_PLOT = Benchmark::PROJECT_LOCATION + "/data/logs/tmp/cuplt.tmp";

/// Path of the 'error' star set on disk. One of the temporary files used for plotting.
const std::string Benchmark::ERROR_PLOT = Benchmark::PROJECT_LOCATION + "/data/logs/tmp/errplt.tmp";

/// Path of the Python script used to plot the current Benchmark instance.
const std::string Benchmark::PLOT_SCRIPT = "\"" + Benchmark::PROJECT_LOCATION + "/lib/benchmark/generate-plot.py\"";

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

/// Private constructor. Directly sets the star set, fov, and focus. The rotation is unknown, as well as the errors
/// applied to it.
///
/// @param stars Star set to give the current benchmark.
/// @param focus Focus star of the given star set.
/// @param fov Field of view (degrees) associated with the given star set.
Benchmark::Benchmark (const Star::list &stars, const Star &focus, const double fov) {
    this->stars = stars, this->focus = focus, this->fov = fov;
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

/// Modify the given image_s and image_fov to the current stars and fov fields. These are all points that define an
/// image, and are required for all identification methods.
///
/// @param image_s Reference to the star list to benchmark star set.
/// @param image_fov Reference to the double to set as the fov.
void Benchmark::present_image (Star::list &image_s, double &image_fov) const {
    image_fov = this->fov;
    image_s = clean_stars();
}

/// Insert into Nibble our current benchmark. In doing so, we lose information about the error model specifics
/// (which stars are errors, the sigma applied to shifts...), the original rotation applied, and the HR values of
/// each star.
///
/// This function is meant to be used iteratively, with set_n incrementing with every call to this. This is recorded
/// as such:
///
/// @code{.cpp}
/// (Some Random Example):
/// set_n | item_n | e | r | s | i       | j       | k        | fov
/// 0     | 0      | 1 | 0 | 0 | 0.43132 | 0.61253 | -0.98244 | 20.0
/// 0     | 1      | 1 | 0 | 0 | 0.43123 | 0.11124 | -0.05135 | 20.0
/// .
/// .
/// .
/// 1     | 0      | 2 | 0 | 0 | 0.62134 | 0.98653 | -0.76253 | 19.5
/// @endcode
///
/// @param nb Open Nibble connection.
/// @param set_n The distinguishing ID of the current benchmark instance.
/// @return -1 if there exists a benchmark with the given set_n. 0 otherwise.
int Benchmark::insert_into_nibble (Nibble &nb, const unsigned int set_n) const {
    std::string schema = "set_n INT, item_n INT, e INT, r INT, s INT, i FLOAT, j FLOAT, k FLOAT, fov FLOAT";
    std::string fields = "set_n, item_n, e, r, s, i, j, k, fov";
    double e = 0, r = 0, s = 0;
    
    // Create the table to hold this data if it does not already exist. This is our working table.
    nb.create_table(TABLE_NAME, schema);
    nb.select_table(TABLE_NAME);
    
    // If there exists records with set_n, stop here.
    if (nb.search_table("set_n = " + std::to_string(set_n), "rowid", 1, 1).size() != 0) {
        return -1;
    }
    
    // We record the number of stars included in each error model for this benchmark.
    for (const ErrorModel &m : this->error_models) {
        if (m.model_name == "Extra Light") {
            e += m.affected.size();
        }
        else if (m.model_name == "Removed Light") {
            r += m.affected.size();
        }
        else {
            // This model must be Shifted Light.
            s += m.affected.size();
        }
    }
    
    // The focus star is logged with a negative item_n.
    nb.insert_into_table(fields, {(double) set_n, -1, e, r, s, focus[0], focus[1], focus[2], fov});
    
    // Log every star into this table.
    for (double i = 0; i < (double) this->stars.size(); i++) {
        nb.insert_into_table(fields, {(double) set_n, i, e, r, s, stars[i][0], stars[i][1], stars[i][2], fov});
    }
    
    return 0;
}

/// Parse a benchmark from Nibble, given the ID.
///
/// @param nb Open Nibble connection.
/// @param set_n ID of the benchmark to return.
/// @return "Empty" benchmark if there exists no Nibble tuple with that set_n. The appropriate benchmark otherwise.
Benchmark Benchmark::parse_from_nibble (Nibble &nb, const unsigned int set_n) {
    std::string set_n_equal = "set_n = " + std::to_string(set_n);
    Nibble::tuple focus;
    Star::list stars;
    double fov;
    
    nb.select_table(TABLE_NAME);
    
    // If there exists no record with that set_n, return an empty benchmark.
    if (nb.search_table("set_n = " + std::to_string(set_n), "rowid", 1, 1).size() == 0) {
        return Benchmark(Star::list {Star::zero()}, Star::zero(), 0);
    }
    
    // Grab the FOV and the focus star for this test set.
    fov = nb.search_table(set_n_equal, "fov", 1, 1)[0];
    focus = nb.search_table(set_n_equal + " AND item_n = -1", "i, j, k", 3, 1);
    
    // Parse all stars for this benchmark.
    for (int i = 0; i <= nb.search_table(set_n_equal, "MAX(item_n)", 1, 1)[0]; i++) {
        Nibble::tuple r = nb.search_table(set_n_equal + " AND item_n = " + std::to_string(i), "i, j, k", 3, 1);
        stars.push_back(Star(r[0], r[1], r[2]));
    }
    
    return Benchmark(stars, Star(focus[0], focus[1], focus[2]), fov);
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
    while (current_n < n || this->stars.size() == (unsigned) current_n + 1) {
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