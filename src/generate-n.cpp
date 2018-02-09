/// @file generate-n.cpp
/// @author Glenn Galvizo
///
/// Source file for the Nibble database generator. This populates all of the tables required for testing. This would
/// a really long time to run all at once, so we call the binary produced by this with an argument of which table to
/// produce.
///
/// @code{.cpp}
/// - hip -> Produce the hip stars table from the Hipparcos catalog.
/// - angle -> Produce table for Angle method.
/// - interior -> Produce table for InteriorAngle method.
/// - sphere -> Produce table for SphericalTriangle method.
/// - plane -> Produce table for PlanarTriangle method.
/// - pyramid -> Produce table for Pyramid method.
/// - composite -> Produce table for CompositePyramid method.
///
/// - x k -> Produce K-Vector table for the given method (invalid for bright and hip).
/// - x d -> Delete all tables for the given method.
/// @endcode
/// @example
/// @code{.cpp}
/// # Produce the table for the Angle method. After this is done, produce the K-Vector table.
/// GenerateN angle k
/// @endcode

#include <iostream>
#include <algorithm>
#include "third-party/inih/INIReader.h"

#include "identification/angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"

/// INIReader to hold configuration associated with table generation.
INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/data/config.ini"));

/// @brief Namespace containing all table 'hashes' for Nibble table generation.
///
/// Table IDs for each table in Nibble.
namespace NBHA {
    static const int HIP = 0; ///< Index of hip table choice in choice space.
    static const int ANGLE = 1; ///< Index of angle table choice in choice space.
    static const int INTERIOR = 2; ///< Index of interior table choice in choice space.
    static const int SPHERE = 3; ///< Index of sphere table choice in choice space.
    static const int PLANE = 4; ///< Index of plane table choice in choice space.
    static const int PYRAMID = 5; ///< Index of pyramid table choice in choice space.
    static const int COMPOSITE = 6; ///< Index of composite table choice in choice space.
    
    /// Convert our choice string into an integer for the switch statements.
    ///
    /// @param choice Name associated with table to hash for.
    /// @return Unique integer associated with the choice.
    int choice (const std::string &choice) {
        std::array<std::string, 7> choice_space = {"hip", "angle", "interior", "sphere", "plane", "pyramid",
            "composite"};
        
        return static_cast<int> (std::distance(choice_space.begin(),
                                               std::find(choice_space.begin(), choice_space.end(), choice)));
    }
}

/// Given the table choice, remove the given table and the K-Vector table if they exist.
///
/// @param choice Name associated with the table to remove.
void remove_table (const std::string &choice) {
    std::string table = cf.Get("table-names", choice, "");
    Nibble nb;
    SQLite::Transaction transaction(*nb.conn);
    
    (*nb.conn).exec("DROP TABLE IF EXISTS " + table);
    (*nb.conn).exec("DROP TABLE IF EXISTS " + table + "_KVEC");
    transaction.commit();
    
    std::cout << "Table deletion was successful (or table does not exist)." << std::endl;
}

/// Given the table choice, attempt to generate the specified table. Error handling should occur within the table
/// generation functions themselves.
///
/// @param choice Name associated with the table to generate.
void generate_table (const std::string &choice) {
    auto display_result = [] (const int r) -> void {
        std::cout << ((r == Nibble::TABLE_NOT_CREATED) ? "Table already exists." : "Table was created successfully.")
                  << std::endl;
    };
    
    double fov = cf.GetReal("hardware", "fov", 0);
    switch (NBHA::choice(choice)) {
        case NBHA::HIP: Chomp();
            return display_result(0);
        
        case NBHA::ANGLE: return display_result(Angle::generate_table(fov, cf.Get("table-names", "angle", "")));
        case NBHA::INTERIOR: throw std::runtime_error(std::string("Not implemented."));
        case NBHA::SPHERE:return display_result(Sphere::generate_table(fov, cf.Get("table-names", "sphere", "")));
        case NBHA::PLANE: return display_result(Plane::generate_table(fov, cf.Get("table-names", "plane", "")));
        case NBHA::PYRAMID:return display_result(Pyramid::generate_table(fov, cf.Get("table-names", "pyramid", "")));
        case NBHA::COMPOSITE: throw std::runtime_error(std::string("Not implemented."));
        default: throw std::runtime_error(std::string("Table choice is not within space {0, 1, 2, 3, 4, 5, 6}."));
    }
}

/// Given the table choice, attempt to generate the K-Vector for the specified table and the predefined focus attribute.
///
/// @param choice Name associated with the table to generate.
void generate_kvec_table (const std::string &choice) {
    Chomp ch;
    
    // Polish the selected table. Create the K-Vector for the given table using the given focus.
    auto create_and_polish = [&ch] (const std::string &table, const std::string &focus) -> void {
        ch.select_table(table);
        ch.polish_table(focus);
        
        std::string response = (ch.create_k_vector(focus) == Nibble::TABLE_NOT_CREATED) ? "K-Vector table already "
            "exists." : "K-Vector table was created successfully.";
        std::cout << response << std::endl;
    };
    
    switch (NBHA::choice(choice)) {
        case NBHA::HIP:throw std::runtime_error(std::string("Cannot generate KVEC table for star catalog table."));
        
        case NBHA::ANGLE: return create_and_polish(cf.Get("table-names", "angle", ""),
                                                   cf.Get("table-focus", "angle", ""));
        case NBHA::INTERIOR: throw std::runtime_error(std::string("Not implemented."));
        case NBHA::SPHERE: return create_and_polish(cf.Get("table-names", "sphere", ""),
                                                    cf.Get("table-focus", "sphere", ""));
        case NBHA::PLANE: return create_and_polish(cf.Get("table-names", "plane", ""),
                                                   cf.Get("table-focus", "plane", ""));
        case NBHA::PYRAMID: return create_and_polish(cf.Get("table-names", "pyramid", ""),
                                                     cf.Get("table-focus", "pyramid", ""));
        case NBHA::COMPOSITE: throw std::runtime_error(std::string("Not implemented."));
        default: throw std::runtime_error(std::string("Table choice is not within space {0, 1, 2, 3, 4, 5, 6}."));
    }
}

/// Select the desired table generation methods given the first argument. In the second argument, indicate whether you
/// want to build a K-Vector table for this first table as well, or delete all tables associated with the first
/// argument.
///
/// @code{.cpp}
/// - hip -> Produce the hip stars from the Hipparcos catalog.
/// - angle -> Produce table for Angle method.
/// - interior -> Produce table for InteriorAngle method.
/// - sphere -> Produce table for SphericalTriangle method.
/// - plane -> Produce table for PlanarTriangle method.
/// - pyramid -> Produce table for Pyramid method.
/// - composite -> Produce table for CompositePyramid method.
///
/// - x k -> Produce K-Vector table for the given method (invalid for bright and hip).
/// - x d -> Delete all tables for the given method.
/// @endcode
/// @example
/// @code{.cpp}
/// # Produce the table for the Angle method. After this is done, produce the K-Vector table.
/// GenerateN angle k
/// @endcode
///
/// @param argc Argument count. Domain is [2, 3].
/// @param argv Argument vector. argv[1] is our selected table. argv[2] is our additional operation specification.
/// @return -1 if the arguments are incorrect. 0 otherwise.
int main (int argc, char *argv[]) {
    auto is_valid_arg = [] (const char *arg, const std::vector<std::string> &input_space) -> bool {
        std::string a = std::string(arg);
        return std::find(input_space.begin(), input_space.end(), a) != input_space.end();
    };
    
    // Validate our input.
    if (argc < 2 || argc > 3) {
        std::cout << "Usage: GenerateN [TableSpecification] [GenerateKVector/DeleteTable]" << std::endl;
        return -1;
    }
    if ((argc >= 2 && !is_valid_arg(argv[1], {"hip", "angle", "interior", "sphere", "plane", "pyramid", "composite"}))
        || (argc == 3 && !is_valid_arg(argv[2], {"k", "d"}))) {
        std::cout << "Usage: GenerateN ['hip', 'angle', 'sphere', ...] ['k', 'd']" << std::endl;
        return -1;
    }
    
    // If desired, delete all tables related to the specified table.
    if (argc == 3 && strcmp(argv[2], "d") == 0) {
        remove_table(std::string(argv[1]));
        return 0;
    }
    
    // Attempt to generate the specified table.
    generate_table(std::string(argv[1]));
    
    // If desired, generate the K-Vector for the specified table.
    if (argc == 3 && strcmp(argv[2], "k") == 0) {
        generate_kvec_table(std::string(argv[1]));
    }
    
    return 0;
}