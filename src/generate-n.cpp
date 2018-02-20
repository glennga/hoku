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
/// - dot -> Produce table for DotAngle method.
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

#include "identification/angle.h"
#include "identification/dot-angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"
#include "identification/composite-pyramid.h"

/// INIReader to hold configuration associated with table generation.
INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));

/// @brief Namespace containing all table 'hashes' for Nibble table generation.
///
/// Table IDs for each table in Nibble.
namespace NBHA {
    static const int HIP = 0; ///< Index of hip table choice in choice space.
    static const int ANGLE = 1; ///< Index of angle table choice in choice space.
    static const int DOT = 2; ///< Index of dot table choice in choice space.
    static const int SPHERE = 3; ///< Index of sphere table choice in choice space.
    static const int PLANE = 4; ///< Index of plane table choice in choice space.
    static const int PYRAMID = 5; ///< Index of pyramid table choice in choice space.
    static const int COMPOSITE = 6; ///< Index of composite table choice in choice space.
    
    /// Convert our choice string into an integer for the switch statements.
    ///
    /// @param choice Name associated with table to hash for.
    /// @return Unique integer associated with the choice.
    int choice (const std::string &choice) {
        std::array<std::string, 7> choice_space = {"hip", "angle", "dot", "sphere", "plane", "pyramid", "composite"};
        
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
    
    switch (NBHA::choice(choice)) {
        case NBHA::HIP: Chomp();
            return display_result(0);
        
        case NBHA::ANGLE: return display_result(Angle::generate_table(cf));
        case NBHA::DOT: return display_result(Dot::generate_table(cf));
        case NBHA::SPHERE:return display_result(Sphere::generate_table(cf));
        case NBHA::PLANE: return display_result(Plane::generate_table(cf));
        case NBHA::PYRAMID:return display_result(Pyramid::generate_table(cf));
        case NBHA::COMPOSITE: return display_result(Composite::generate_table(cf));
        default: throw std::runtime_error(std::string("Table choice is not within space {0, 1, 2, 3, 4, 5, 6}."));
    }
}

/// Given the table choice, attempt to generate the K-Vector for the specified table and the predefined focus attribute.
///
/// @param choice Name associated with the table to generate.
void generate_kvec_table (const std::string &choice) {
    Chomp ch;
    
    // Polish the selected table. Create the K-Vector for the given table using the given focus.
    auto create_and_polish = [&ch] (const std::string &method) -> void {
        ch.select_table(cf.Get("table-names", method, ""));
        ch.polish_table(cf.Get("table-focus", method, ""));
        
        std::string response = (ch.create_k_vector(cf.Get("table-focus", method, "")) == Nibble::TABLE_NOT_CREATED)
                               ? "K-Vector table already exists." : "K-Vector table was created successfully.";
        std::cout << response << std::endl;
    };
    
    switch (NBHA::choice(choice)) {
        case NBHA::HIP:throw std::runtime_error(std::string("Cannot generate KVEC table for star catalog table."));
        case NBHA::ANGLE: return create_and_polish("angle");
        case NBHA::DOT: return create_and_polish("dot");
        case NBHA::SPHERE: return create_and_polish("sphere");
        case NBHA::PLANE: return create_and_polish("plane");
        case NBHA::PYRAMID: return create_and_polish("pyramid");
        case NBHA::COMPOSITE: return create_and_polish("composite");
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
/// - dot -> Produce table for DotAngle method.
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
    if ((argc >= 2 && !is_valid_arg(argv[1], {"hip", "angle", "dot", "sphere", "plane", "pyramid", "composite"}))
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