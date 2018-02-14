/// @file run-identify.cpp
/// @author Glenn Galvizo
///
/// Performs the 'identify' method to identify a set of stars, given a CSV describing the image. User can specify
/// which identification method should be used. The CSV should be formatted as such:
/// @code{.cpp}
/// # Use the FITS coordinates of each centroid [top-left = (0, 0), bottom-right = (max-width, max-height)]
/// [x-coordinate-1],[y-coordinate-1]
/// [x-coordinate-2],[y-coordinate-2]
/// .
/// .
/// .
/// [x-coordinate-N],[y-coordinate-N]
/// @endcode
///
/// The output is a Matplotlib image, displaying the image with Hipparcos labels attached to them.
/// @example
/// @code{.cpp}
/// # Run the Angle identification method on my-image.csv.
/// RunIdentify angle my-image.csv
/// @endcode

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "math/mercator.h"
#include "identification/angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"
#include "identification/composite-pyramid.h"

/// INIReader to hold configuration associated with experiments.
INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));

/// Holds all of the table names used with 'identifier_hash'.
namespace NBHA {
    /// Array of all table names. Used with 'identifier_hash'.
    std::array<std::string, 6> id = {cf.Get("table-names", "angle", ""), cf.Get("table-names", "interior", ""),
        cf.Get("table-names", "sphere", ""), cf.Get("table-names", "plane", ""), cf.Get("table-names", "pyramid", ""),
        cf.Get("table-names", "composite", "")};
}

/// Given an open filestream to the image file, determine the image center and the rest of the stars.
///
/// @param image Reference to the filestream of the image.
/// @return Empty list if there exist less than four total stars. Otherwise, the list of stars. The first is the focus,
/// and the following are the stars in the image.
Star::list parse_csv (std::ifstream &image) {
    Star::list s_i;
    
    // Image focus is always at (0, 0) after FITS -> (0, 0) centric coordinate translation.
    double hf = cf.GetReal("hardware", "cf", 0), ddp = cf.GetReal("hardware", "ddp", 0);
    s_i.push_back(Mercator::transform_point(0, 0, ddp));
    
    try {
        for (std::string entry; std::getline(image, entry);) {
            std::istringstream entry_stream(entry);
            std::vector<double> s_c;
            
            // Read the line by commas.
            while (entry_stream.good()) {
                std::string substr;
                std::getline(entry_stream, substr, ',');
                s_c.push_back(std::stof(substr, nullptr));
            }
            
            // Translate points to fit (0, 0) center.
            double x = s_c[0] - hf, y = s_c[1] - hf;
            
            // Project the star to 3D, and save it.
            s_i.push_back(Mercator::transform_point(x, y, ddp).normalize());
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
int identifier_hash (const std::string &identifier_in) {
    std::array<std::string, 6> space = {"angle", "interior", "sphere", "plane", "pyramid", "composite"};
    return static_cast<int> (std::distance(space.begin(), std::find(space.begin(), space.end(), identifier_in)));
}

/// Run the specified identification method with the given field-of-view and star list. Display the results using
/// Matplotlib.
///
/// @param id_method String containing the identification method to run.
/// @param s_i Star list containing the image center (first) and the image stars (following).
/// @return 0 when finished.
int run_identity (const std::string &id_method, const Star::list &s_i) {
    // Construct the image into a Benchmark given the arguments.
    const double fov = cf.GetReal("hardware", "fov", 0);
    const int i = identifier_hash(id_method);
    Benchmark input(Star::list(s_i.begin() + 1, s_i.end()), s_i[0], fov);
    
    // Attach hyperparameters.
    Identification::Parameters p = Identification::DEFAULT_PARAMETERS;
    Identification::collect_parameters(p, cf);
    
    // Identify using the given ID method, and display the results through Python.
    auto identify = [&input, &p, &s_i, &fov] (const Star::list &result) -> int {
        Benchmark output(result, s_i[0], fov);
        output.display_plot();
        return 0;
    };
    
    switch (i) {
        case 0: return identify(Angle(input, p).identify_all());
        case 1: throw std::runtime_error(std::string("Not implemented."));
        case 2: return identify(Sphere(input, p).identify_all());
        case 3: return identify(Plane(input, p).identify_all());
        case 4: return identify(Pyramid(input, p).identify_all());
        case 5: return identify(Composite(input, p).identify_all());
        default: throw std::runtime_error(std::string("ID method not in appropriate space."));
    }
}

/// Select the desired identification method to use given the first argument. In the second argument, indicate the
/// field-of-view of the image. In the third argument, specify the CSV file to read.
///
/// @param argc Argument count. Must be equal to 4.
/// @param argv Argument vector. Must be in order 'identification method', 'field of view', 'image file'.
/// @return -2 if the CSV is incorrectly formatted. -1 if the arguments are incorrect. 0 otherwise.
int main (int argc, char *argv[]) {
    auto is_valid_arg = [] (const char *arg, const std::vector<std::string> &input_space) -> bool {
        std::string a = std::string(arg);
        return std::find(input_space.begin(), input_space.end(), a) != input_space.end();
    };
    
    // Validate our input.
    if (argc != 3) {
        std::cout << "Usage: RunIdentify [id method] [image file]" << std::endl;
        return -1;
    }
    else if (!is_valid_arg(argv[1], {"angle", "interior", "sphere", "plane", "pyramid", "composite"})) {
        std::cout << "Invalid ID method. Use: ['angle', 'interior', 'sphere', 'plane', 'pyramid', 'composite']"
                  << std::endl;
        return -1;
    }
    
    // Open the image file.
    std::ifstream image(argv[2]);
    if (!image.is_open()) {
        std::cout << "Cannot open image file. " << std::endl;
        return -1;
    }
    
    // Parse the CSV for an image center and stars.
    Star::list image_s = parse_csv(image);
    if (image_s.empty()) {
        std::cout << "Image file not correctly formatted." << std::endl;
        return -2;
    }
    
    // Run the identification.
    return run_identity(std::string(argv[1]), image_s);
}