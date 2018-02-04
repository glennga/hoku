/// @file run-identify.cpp
/// @author Glenn Galvizo
///
/// Performs the 'identify' method to identify a set of stars, given a CSV describing the image. User can specify
/// which identification method should be used. The CSV should be formatted as such:
/// @code{.cpp}
/// # Image center is specified FIRST. Following stars are specified with reference to this point:
/// [x-coordinate],[y-coordinate],[z-coordinate]
///
/// # All stars in image follow the center. TODO: figure specific format of stars:
/// [x-coordinate-1],[y-coordinate-1],[z-coordinate-1],[name-1]
/// [x-coordinate-2],[y-coordinate-2],[z-coordinate-2],[name-2]
/// [x-coordinate-3],[y-coordinate-3],[z-coordinate-3],[name-3]
/// .
/// .
/// .
/// [x-coordinate-N],[y-coordinate-N],[z-coordinate-N],[name-N]
/// @endcode
///
/// The output is a Matplotlib image, displaying the image with Hipparcos labels attached to them.
/// @example
/// @code{.cpp}
/// # Run the Angle identification method on my-image.csv. Image has a field-of-view = 20 degrees.
/// RunIdentify angle 20 my-image.csv
/// @endcode

#include <iostream>
#include "identification/angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"

namespace DCNT {
    static const char *ANGLE_NAME = "ANGLE_20"; ///< Name of table generated for Angle method.
    static const char *INTERIOR_NAME = "INTERIOR_20"; ///< Name of table generated for InteriorAngle method.
    static const char *SPHERE_NAME = "SPHERE_20"; ///< Name of table generated for SphericalTriangle method.
    static const char *PLANE_NAME = "PLANE_20"; ///< Name of table generated for PlanarTriangle method.
    static const char *PYRAMID_NAME = "PYRAMID_20"; ///< Name of table generated for Pyramid method.
    static const char *COMPOSITE_NAME = "COMPOSITE_20"; ///< Name of table generated for CompositePyramid method.
    
    /// Array of all table names. Used with 'identifier_hash'.
    std::array<std::string, 6> id_space = {ANGLE_NAME, INTERIOR_NAME, SPHERE_NAME, PLANE_NAME, PYRAMID_NAME,
        COMPOSITE_NAME};
}

namespace DCIP {
    static const double SIGMA_QUERY = 0.0001; ///< Query must be within 3 * sigma_query.
    static const unsigned int SQL_LIMIT = 100; ///< While performing a SQL query, limit results by this number.
    static const double SIGMA_OVERLAY = 0.0001; ///< Resultant of inertial->body rotation must within 3 * this number.
    static const unsigned int NU_MAX = 10000; ///< Maximum number of query star comparisons before returning empty.
    
    unsigned int nu_location; ///< Location to hold count of query star comparisons.
    std::shared_ptr<unsigned int> nu = std::make_shared<unsigned int>(nu_location);
    
    Rotation::wahba_function f = Rotation::triad; ///< Function to use to solve Wahba's problem with.
}

/// Given an open filestream to the image file, determine the image center and the rest of the stars.
///
/// @param image Reference to the filestream of the image.
/// @return Empty list if there exist less than four total stars. Otherwise, the list of stars. The first is the focus,
/// and the following are the stars in the image.
Star::list parse_csv (std::ifstream &image) {
    Star::list s_i;
    
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
            
            // Using the first three elements (x, y, z), construct and save the **normalized** star.
            s_i.push_back(Star(s_c[0], s_c[1], s_c[2], 0, 0, true));
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
/// @param fov Field of view of the image.
/// @param s_i Star list containing the image center (first) and the image stars (following).
/// @return 0 when finished.
int run_identity (const std::string &id_method, const double fov, const Star::list &s_i) {
    // Construct the image into a Benchmark given the arguments.
    Benchmark input(Star::list(s_i.begin() + 1, s_i.end()), s_i[0], fov);
    const int i = identifier_hash(id_method);
    
    // Construct hyperparameters.
    Identification::Parameters p = {};
    p.nu_max = DCIP::NU_MAX, p.sigma_overlay = DCIP::SIGMA_OVERLAY, p.sigma_query = DCIP::SIGMA_QUERY;
    p.nu = DCIP::nu, p.f = DCIP::f, p.table_name = DCNT::id_space[i];
    
    // Identify using the given ID method, and display the results through Python.
    auto identify = [&input, &p, &s_i, &fov] (const Star::list &result) -> int {
        Benchmark output(result, s_i[0], fov);
        output.display_plot();
        return 0;
    };
    
    switch (i) {
        case 0: return identify(Angle(input, p).identify_all());
        case 1: throw "Not implemented.";
        case 2: return identify(Sphere(input, p).identify_all());
        case 3: return identify(Plane(input, p).identify_all());
        case 4: return identify(Pyramid(input, p).identify_all());
        case 5: throw "Not implemented.";
        default: throw "ID method not in appropriate space.";
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
    if (argc != 4) {
        std::cout << "Usage: RunIdentify [id method] [field of view (degrees)] [image file]" << std::endl;
        return -1;
    }
    else if (!is_valid_arg(argv[1], {"angle", "interior", "sphere", "plane", "pyramid", "composite"})) {
        std::cout << "Invalid ID method. Use: ['angle', 'interior', 'sphere', 'plane', 'pyramid', 'composite']"
                  << std::endl;
        return -1;
    }
    else if (std::stof(argv[2]) < 0) {
        std::cout << "Field of view must be greater than 0. " << std::endl;
        return -1;
    }
    
    // Open the image file.
    std::ifstream image(argv[3]);
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
    return run_identity(std::string(argv[1]), std::stof(argv[2]), image_s);
}