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

#include "identification/angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"

/// @brief Namespace containing all parameters for Nibble table generation.
///
/// Defining characteristics of the Nibble tables generated.
namespace DCNT {
    static const double FOV = 20; ///< Maximum field-of-view for each generated table.
    
    static const char *HIP_NAME = "HIP"; ///< Name of star catalog table w/o magnitude restrictions.
    static const char *ANGLE_NAME = "ANGLE_20"; ///< Name of table generated for Angle method.
    static const char *INTERIOR_NAME = "INTERIOR_20"; ///< Name of table generated for InteriorAngle method.
    static const char *SPHERE_NAME = "SPHERE_20"; ///< Name of table generated for SphericalTriangle method.
    static const char *PLANE_NAME = "PLANE_20"; ///< Name of table generated for PlanarTriangle method.
    static const char *PYRAMID_NAME = "PYRAMID_20"; ///< Name of table generated for Pyramid method.
    static const char *COMPOSITE_NAME = "COMPOSITE_20"; ///< Name of table generated for CompositePyramid method.
    
    static const char *KVEC_ANGLE_FOCUS = "theta"; ///< Focus attribute for K-Vector Angle table.
    static const char *KVEC_INTERIOR_FOCUS = "theta"; ///< Focus attribute for K-Vector InteriorAngle table.
    static const char *KVEC_SPHERE_FOCUS = "a"; ///< Focus attribute for K-Vector SphericalTriangle table.
    static const char *KVEC_PLANE_FOCUS = "a"; ///< Focus attribute for K-Vector PlanarTriangle table.
    static const char *KVEC_PYRAMID_FOCUS = "theta"; ///< Focus attribute for K-Vector Pyramid table.
    static const char *KVEC_COMPOSITE_FOCUS = "a"; ///< Focus attribute for K-Vector CompositePyramid table.
    
    static const int HIP_TABLE_HASH = 0; ///< Index of hip table choice in choice space.
    static const int ANGLE_TABLE_HASH = 1; ///< Index of angle table choice in choice space.
    static const int INTERIOR_TABLE_HASH = 2; ///< Index of interior table choice in choice space.
    static const int SPHERE_TABLE_HASH = 3; ///< Index of sphere table choice in choice space.
    static const int PLANE_TABLE_HASH = 4; ///< Index of plane table choice in choice space.
    static const int PYRAMID_TABLE_HASH = 5; ///< Index of pyramid table choice in choice space.
    static const int COMPOSITE_TABLE_HASH = 6; ///< Index of composite table choice in choice space.
}

/// Convert our choice string into an integer for the switch statements.
///
/// @param choice Name associated with table to hash for.
/// @return Unique integer associated with the choice.
int choice_hash (const std::string &choice) {
    std::array<std::string, 7> choice_space = {"hip", "angle", "interior", "sphere", "plane", "pyramid", "composite"};
    
    return static_cast<int> (std::distance(choice_space.begin(),
                                           std::find(choice_space.begin(), choice_space.end(), choice)));
}

/// Given the table choice, remove the given table and the K-Vector table if they exist.
///
/// @param choice Name associated with the table to remove.
void remove_table (const std::string &choice) {
    auto choose_table = [&choice] () -> std::string {
        switch (choice_hash(choice)) {
            case DCNT::HIP_TABLE_HASH: return DCNT::HIP_NAME;
            case DCNT::ANGLE_TABLE_HASH: return DCNT::ANGLE_NAME;
            case DCNT::INTERIOR_TABLE_HASH: return DCNT::INTERIOR_NAME;
            case DCNT::SPHERE_TABLE_HASH: return DCNT::SPHERE_NAME;
            case DCNT::PLANE_TABLE_HASH: return DCNT::PLANE_NAME;
            case DCNT::PYRAMID_TABLE_HASH: return DCNT::PYRAMID_NAME;
            case DCNT::COMPOSITE_TABLE_HASH: return DCNT::COMPOSITE_NAME;
            default: throw std::runtime_error(std::string("Table choice is not within space {0, 1, 2, 3, 4, 5, 6}."));
        }
    };
    
    Nibble nb;
    SQLite::Transaction transaction(*nb.conn);
    (*nb.conn).exec("DROP TABLE IF EXISTS " + choose_table());
    (*nb.conn).exec("DROP TABLE IF EXISTS " + choose_table() + "_KVEC");
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
    
    switch (choice_hash(choice)) {
        case DCNT::HIP_TABLE_HASH: Chomp();
            return display_result(0);
        
        case DCNT::ANGLE_TABLE_HASH: return display_result(Angle::generate_table(DCNT::FOV, DCNT::ANGLE_NAME));
        case DCNT::INTERIOR_TABLE_HASH: throw std::runtime_error(std::string("Not implemented."));
        case DCNT::SPHERE_TABLE_HASH: return display_result(Sphere::generate_table(DCNT::FOV, DCNT::SPHERE_NAME));
        case DCNT::PLANE_TABLE_HASH: return display_result(Plane::generate_table(DCNT::FOV, DCNT::PLANE_NAME));
        case DCNT::PYRAMID_TABLE_HASH: return display_result(Pyramid::generate_table(DCNT::FOV, DCNT::PYRAMID_NAME));
        case DCNT::COMPOSITE_TABLE_HASH: throw std::runtime_error(std::string("Not implemented."));
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
        
        auto response = (ch.create_k_vector(focus) == Nibble::TABLE_NOT_CREATED) ? "K-Vector table already exists."
                                                                                 : "K-Vector table was created "
                            "successfully.";
        std::cout << response << std::endl;
    };
    
    switch (choice_hash(choice)) {
        case DCNT::HIP_TABLE_HASH:
            throw std::runtime_error(std::string("Cannot generate KVEC table for star catalog "
                                                     "table."));
            
        case DCNT::ANGLE_TABLE_HASH: return create_and_polish(DCNT::ANGLE_NAME, DCNT::KVEC_ANGLE_FOCUS);
        case DCNT::INTERIOR_TABLE_HASH: throw std::runtime_error(std::string("Not implemented."));
        case DCNT::SPHERE_TABLE_HASH: return create_and_polish(DCNT::SPHERE_NAME, DCNT::KVEC_SPHERE_FOCUS);
        case DCNT::PLANE_TABLE_HASH: return create_and_polish(DCNT::PLANE_NAME, DCNT::KVEC_PLANE_FOCUS);
        case DCNT::PYRAMID_TABLE_HASH: return create_and_polish(DCNT::PYRAMID_NAME, DCNT::KVEC_PYRAMID_FOCUS);
        case DCNT::COMPOSITE_TABLE_HASH: throw std::runtime_error(std::string("Not implemented."));
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