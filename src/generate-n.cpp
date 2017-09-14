/// @file generate-n.cpp
/// @author Glenn Galvizo
///
/// Source file for the Nibble database generator. This populates all of the tables required for testing. This would
/// a really long time to run all at once, so we call the binary produced by this with an argument of which table to
/// produce.
///
/// @code{.cpp}
/// - 0 -> Produce table for Angle method.
/// - 1 -> Produce hash table for AstrometryNet method.
/// - 2 -> Produce centers table for AstrometryNet method .
/// - 3 -> Produce table for SphericalTriangle method.
/// - 4 -> Produce table for PlanarTriangle method.
/// - 5 -> Produce table for Pyramid method.
///
/// - x 1 -> Produce K-Vector table for the given method.
/// - x 2000 -> Delete all tables for the given method.
/// @endcode
/// @example
/// @code{.cpp}
/// # Produce the SEP table for the Angle method. After this is done, produce the K-Vector table for SEP.
/// GenerateN 0 1
/// @endcode

#include <iostream>
#include "identification/angle.h"
#include "identification/astrometry-net.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"

/// Defining characteristics of the Nibble tables generated.
namespace DCNT {
    static const double FOV = 20; ///< Maximum field-of-view for each generated table.
    static const int A_LIMIT = 1000; ///< Asterism limit for AstrometryNet tables.
    static const int TD_H = 3; ///< Recursion depth maximum (moment calculation) for SphericalTriangle tables.
    
    static const std::string ANGLE_NAME = "SEP_20"; ///< Name of table generated for Angle method.
    static const std::string ASTROH_NAME = "ASTRO_H20"; ///< Name of hash table generated for AstrometryNet method.
    static const std::string ASTROC_NAME = "ASTRO_C20"; ///< Name of centers table generated for AstrometryNet method.
    static const std::string SPHERE_NAME = "SPHERE_20"; ///< Name of table generated for SphericalTriangle method.
    static const std::string PLANE_NAME = "PLANE_20"; ///< Name of table generated for PlanarTriangle method.
    static const std::string PYRAMID_NAME = "PYRA_20"; ///< Name of table generated for Pyramid method.
    
    static const std::string KVEC_ANGLE_FOCUS = "theta"; ///< Focus attribute for K-Vector Angle table.
    static const std::string KVEC_ASTROH_FOCUS = "cx"; ///< Focus attribute for K-Vector AstrometryNet hash table.
    static const std::string KVEC_ASTROC_FOCUS = "i"; ///< Focus attribute for K-Vector AstrometryNet centers table.
    static const std::string KVEC_SPHERE_FOCUS = "a"; ///< Focus attribute for K-Vector SphericalTriangle table.
    static const std::string KVEC_PLANE_FOCUS = "a"; ///< Focus attribute for K-Vector PlanarTriangle table.
    static const std::string KVEC_PYRAMID_FOCUS = "theta"; ///< Focus attribute for K-Vector Pyramid table.
}

/// Given the table choice, remove the given table and the K-Vector table if they exist.
///
/// @param choice Number associated with the table to remove.
void remove_table (const int choice) {
    auto choose_table = [choice] (const int &c) -> std::string {
        switch (choice) {
            case 0: return DCNT::ANGLE_NAME;
            case 1: return DCNT::ASTROH_NAME;
            case 2: return DCNT::ASTROC_NAME;
            case 3: return DCNT::SPHERE_NAME;
            case 4: return DCNT::PLANE_NAME;
            case 5: return DCNT::PYRAMID_NAME;
            default: throw "Table choice is not within space {0, 1, 2, 3, 4, 5}.";
        }
    };
    
    Nibble nb;
    SQLite::Transaction transaction(*nb.db);
    (*nb.db).exec("DROP TABLE IF EXISTS " + choose_table(choice));
    (*nb.db).exec("DROP TABLE IF EXISTS " + choose_table(choice) + "_KVEC");
    transaction.commit();
}

/// Given the table choice, attempt to generate the specified table. Error handling should occur within the table
/// generation functions themselves.
///
/// @param choice Number associated with the table to generate.
void generate_table (const int choice) {
    switch (choice) {
        case 0: return (void) Angle::generate_sep_table(DCNT::FOV, DCNT::ANGLE_NAME);
        case 1: return (void) AstrometryNet::generate_hash_table(DCNT::FOV, DCNT::A_LIMIT, DCNT::ASTROH_NAME);
        case 2: return (void) AstrometryNet::generate_center_table(DCNT::ASTROH_NAME, DCNT::ASTROC_NAME);
        case 3: return (void) Sphere::generate_triangle_table(DCNT::FOV, DCNT::TD_H, DCNT::SPHERE_NAME);
        case 4: return (void) Plane::generate_triangle_table(DCNT::FOV, DCNT::PLANE_NAME);
        case 5: return (void) Pyramid::generate_sep_table(DCNT::FOV, DCNT::PYRAMID_NAME);
        default: throw "Table choice is not within space {0, 1, 2, 3, 4, 5}.";
    }
}

/// Given the table choice, attempt to generate the K-Vector for the specified table and the predefined focus attribute.
///
/// @param choice Number associated with the table to generate.
void generate_kvec_table (const int choice) {
    Chomp ch;
    
    // Create the K-Vector for the given table using the given focus. Polish the table using the focus as well.
    auto create_and_polish = [&ch] (const std::string &table, const std::string &focus) -> void {
        ch.select_table(table);
        ch.create_k_vector(focus);
        ch.polish_table(focus);
    };
    
    switch (choice) {
        case 0: return create_and_polish(DCNT::ANGLE_NAME, DCNT::KVEC_ANGLE_FOCUS);
        case 1: return create_and_polish(DCNT::ASTROH_NAME, DCNT::KVEC_ASTROH_FOCUS);
        case 2: return create_and_polish(DCNT::ASTROC_NAME, DCNT::KVEC_ASTROC_FOCUS);
        case 3: return create_and_polish(DCNT::SPHERE_NAME, DCNT::KVEC_SPHERE_FOCUS);
        case 4: return create_and_polish(DCNT::PLANE_NAME, DCNT::KVEC_PLANE_FOCUS);
        case 5: return create_and_polish(DCNT::PYRAMID_NAME, DCNT::KVEC_PYRAMID_FOCUS);
        default: throw "Table choice is not within space {0, 1, 2, 3, 4, 5}.";
    }
}

/// Select the desired table generation methods given the first argument. In the second argument, indicate whether you
/// want to build a K-Vector table for this first table as well, or delete all tables associated with the first
/// argument.
///
/// @code{.cpp}
/// - 0 -> Produce table for Angle method.
/// - 1 -> Produce hash table for AstrometryNet method.
/// - 2 -> Produce centers table for AstrometryNet method .
/// - 3 -> Produce table for SphericalTriangle method.
/// - 4 -> Produce table for PlanarTriangle method.
/// - 5 -> Produce table for Pyramid method.
///
/// - x 1 -> Produce K-Vector table for the given method.
/// - x 2000 -> Delete all tables for the given method.
/// @endcode
/// @example
/// @code{.cpp}
/// # Produce the SEP table for the Angle method. After this is done, produce the K-Vector table for SEP.
/// GenerateN 0 1
/// @endcode
///
/// @param argc Argument count. Domain is [1, 2].
/// @param argv Argument vector. argv[0] is our selected table. argv[1] is our additional operation specification.
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
    else if (!(is_valid_arg(argv[1], {"0", "1", "2", "3", "4", "5"}) && is_valid_arg(argv[2], {"1", "2000"}))) {
        std::cout << "Usage: GenerateN [0/1/2/3/4/5] [1/2000] " << std::endl;
        return -1;
    }
    
    // If desired, delete all tables related to the specified table.
    if (argc == 3 && atoi(argv[2]) == 2000) {
        remove_table(atoi(argv[0]));
        return 0;
    }
    
    // Attempt to generate the specified table.
    generate_table(atoi(argv[1]));
    
    // If desired, generate the K-Vector for the specified table.
    if (argc == 3 && std::string(argv[2]) == "1") {
        generate_kvec_table(atoi(argv[0]));
    }
    
    return 0;
}