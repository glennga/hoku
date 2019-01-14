/// @file identify-fits.cpp
/// @author Glenn Galvizo
///
/// Performs the 'identify' method to identify the stars in a FITS image. User can specify which identification
/// method should be used. The output is a Matplotlib image, displaying the image with Hipparcos labels attached to
/// them.
/// @example
/// @code{.cpp}
/// # Run the Angle identification method on my-image.fits
/// IdentifyFITS angle full-path-my-image.fits
/// @endcode

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "math/mercator.h"
#include "identification/angle.h"
#include "identification/dot-angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"
#include "identification/composite-pyramid.h"

/// INIReader to hold configuration associated with experiments.
INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));

/// Holds all of the table names used with 'identifier_hash'.
namespace NBHA {
    /// Array of all table names. Used with 'identifier_hash'.
    std::array<std::string, 6> id = {cf.Get("table-names", "angle", ""), cf.Get("table-names", "dot", ""),
        cf.Get("table-names", "sphere", ""), cf.Get("table-names", "plane", ""), cf.Get("table-names", "pyramid", ""),
        cf.Get("table-names", "composite", "")};
}

/// Given the name of a FITS image file, determine the image center and the rest of the stars.
///
/// @param image Reference to the filename argument passed with this program.
/// @return A pointer to the pipe containing the output from our centroid determination script.
std::shared_ptr<FILE> parse_fits (const std::string &image) {
    // Run the FITS -> CSV centroid script.
    std::string script_path = std::string(std::getenv("HOKU_PROJECT_PATH")) + "/script/python/find_centroids.py";
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    std::string cmd = std::string("python -E \"") + script_path + "\" \"" + image + "\"";
#else
    std::string cmd = std::string("python3 \"") + script_path + "\" " + image;
#endif
    std::shared_ptr<FILE> centroids_pipe(popen(cmd.c_str(), "r"), pclose);
    if (!centroids_pipe) {
        throw std::runtime_error(std::string("'python/find_centroids.py' exited with an error. Double check the file"
                                                 " you have passed."));
    }
    
    return centroids_pipe;
}

/// Given an open filestream to a CSV containing the centroids of a FITS image, determine the image center (which
/// should be returned first) and the rest of the stars.
///
/// @param centroids_pipe Reference to the a pointer of the pipe containing the centroids in comma-separated format.
/// @return Empty list if there exist less than four total stars. Otherwise, the list of stars. The first is the focus,
/// and the following are the stars in the image.
Star::list parse_centroids (const std::shared_ptr<FILE> &centroids_pipe) {
    double hc = cf.GetReal("hardware", "hmp", 0) / 2.0, vc = cf.GetReal("hardware", "vmp", 0) / 2.0;
    double dpp = cf.GetReal("hardware", "dpp", 0);
    std::array<char, 128> buffer;
    Star::list s_i;
    
    try {
        while (!feof(centroids_pipe.get())) {
            if (fgets(buffer.data(), 128, centroids_pipe.get()) != nullptr) {
                std::istringstream entry_stream(std::string(buffer.begin(), buffer.end()));
                std::array<double, 2> s_c;
                std::string s_c_0, s_c_1;
                
                // Separate our line by commas, and convert our results into floats.
                std::getline(entry_stream, s_c_0, ','), std::getline(entry_stream, s_c_1);
                s_c[0] = std::stof(s_c_0, nullptr), s_c[1] = std::stof(s_c_1, nullptr);
                std::cout << "Image (FITS Coordinates): (" << s_c[0] << ", " << s_c[1] << ")" << std::endl;

                // Translate points to fit (0, 0) center.
                double x = s_c[0] - hc, y = s_c[1] - vc;

                // Project the star to 3D, and save it.
                s_i.emplace_back(Star::wrap(Mercator::transform_point(x, y, dpp)));
            }
        }
    }
    catch (std::exception &e) {
        // Ignore entries that cannot be parsed (most likely comments).
    }
    
    return (s_i.size() < 4) ? Star::list {} : s_i;
}

/// Convert the given user argument specifying the identifier name, to its appropriate hash.
///
/// @param identifier_in Input given by the user, to identify the type of experiment table.
/// @return Index of the name space below. 6 is given if the given input is not in the name space.
unsigned int identifier_hash (const std::string &identifier_in) {
    std::array<std::string, 6> space = {"angle", "dot", "sphere", "plane", "pyramid", "composite"};
    return static_cast<unsigned int> (std::distance(space.begin(),
                                                    std::find(space.begin(), space.end(), identifier_in)));
}

/// Run the specified identification method with the given field-of-view and star list. Display the results using
/// Matplotlib.
///
/// @param id_method String containing the identification method to run.
/// @param s_i Star list containing the image center (first) and the image stars (following).
/// @return 0 when finished.
int identify_fits (const std::string &id_method, const Star::list &s_i) {
    // Construct the image into a Benchmark given the arguments.
    const double fov = cf.GetReal("hardware", "fov", 0);
    const unsigned int i = identifier_hash(id_method);
    Benchmark input(Star::list(s_i.begin() + 1, s_i.end()), s_i[0], fov);
    
    // Attach hyperparameters.
    Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
    p.table_name = cf.Get("table-names", id_method, Identification::DEFAULT_TABLE_NAME);
    Identification::collect_parameters(p, cf, id_method);
    
    // Identify using the given ID method, and display the results through Python.
    auto plot = [&] (const Star::list &result) -> int {
        for (const Star &s: result) {
            std::cout << "Star: " << s << std::endl;
        }
        Chomp ch;
        Star r_f = ch.query_hip(result[0].get_label());

        // Display the quaternion associated with this mapping.
        std::cout << "Rotation: " << p.f({result[0], result[1]}, {ch.query_hip(result[0].get_label()),
                                                                  ch.query_hip(result[1].get_label())}) << std::endl;

        // Search for all stars near our focus. This is what will be plotted.
        Benchmark output(ch.nearby_bright_stars(r_f, fov, 100), r_f, fov);
        output.display_plot();
        return 0;
    };
    
    switch (i) {
        case 0: return plot(Angle(input, p).identify());
        case 1: return plot(Dot(input, p).identify());
        case 2: return plot(Sphere(input, p).identify());
        case 3: return plot(Plane(input, p).identify());
        case 4: return plot(Pyramid(input, p).identify());
        case 5: return plot(Composite(input, p).identify());
        default: throw std::runtime_error(std::string("ID method not in appropriate space."));
    }
}

/// Select the desired identification method to use given the first argument. In the second argument, indicate the
/// field-of-view of the image. In the third argument, specify the FITS file to read.
///
/// @param argc Argument count. Must be equal to 3.
/// @param argv Argument vector. Must be in order 'identification method', 'image file'.
/// @return -1 if the arguments are incorrect. 0 otherwise.
int main (int argc, char *argv[]) {
    auto is_valid_arg = [] (const char *arg, const std::vector<std::string> &input_space) -> bool {
        std::string a = std::string(arg);
        return std::find(input_space.begin(), input_space.end(), a) != input_space.end();
    };
    
    // Validate our input.
    if (argc != 3) {
        std::cout << "Usage: IdentifyFITS [id method] [image file]" << std::endl;
        return -1;
    }
    else if (!is_valid_arg(argv[1], {"angle", "dot", "sphere", "plane", "pyramid", "composite"})) {
        std::cout << "Invalid ID method. Use: ['angle', 'dot', 'sphere', 'plane', 'pyramid', 'composite']" << std::endl;
        return -1;
    }
    
    // Parse the FITS for an image center and stars.
    Star::list image_s = parse_centroids(parse_fits(std::string(argv[2])));
    
    // Run the identification.
    return identify_fits(std::string(argv[1]), image_s);
}
