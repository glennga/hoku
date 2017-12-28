/// @file generate-n.cpp
/// @author Glenn Galvizo
///
/// Source file for the Nibble database generator. This populates all of the tables required for testing. This would
/// a really long time to run all at once, so we call the binary produced by this with an argument of which table to
/// produce.
///
/// @code{.cpp}
/// - 0 -> Produce the bright stars (< 6.0) table from the Hipparcos catalog.
/// - 1 -> Produce the hip stars (no magnitude restriction) table from the Hipparcos catalog.
/// - 2 -> Produce table for Angle method.
/// - 3 -> Produce table for SphericalTriangle method.
/// - 4 -> Produce table for PlanarTriangle method.
/// - 5 -> Produce table for Pyramid method.
/// - 6 -> Produce table for Coin method.
///
/// - x k -> Produce K-Vector table for the given method (valid above 1).
/// - x d -> Delete all tables for the given method.
/// @endcode
/// @example
/// @code{.cpp}
/// # Produce the SEP table for the Angle method. After this is done, produce the K-Vector table for SEP.
/// GenerateN 2 k
/// @endcode

#include <iostream>
#include "identification/angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
//#include "identification/pyramid.h"
//#include "identification/coin.h"

/// Defining characteristics of the Nibble tables generated.
namespace DCNT {
    static const double FOV = 20; ///< Maximum field-of-view for each generated table.
    
    static const char *BRIGHT_HIP_NAME = "HIP_BRIGHT"; ///< Name of star catalog table w/ magnitude < 6.0 restriction.
    static const char *HIP_NAME = "HIP"; ///< Name of star catalog table w/o magnitude restrictions.
    static const char *ANGLE_NAME = "ANG_20"; ///< Name of table generated for Angle method.
    static const char *SPHERE_NAME = "SPHERE_20"; ///< Name of table generated for SphericalTriangle method.
    static const char *PLANE_NAME = "PLANE_20"; ///< Name of table generated for PlanarTriangle method.
    static const char *PYRAMID_NAME = "PYRA_20"; ///< Name of table generated for Pyramid method.
    static const char *COIN_NAME = "HOKU_20"; ///< Name of table generated for Hoku method.
    
    static const char *KVEC_ANGLE_FOCUS = "theta"; ///< Focus attribute for K-Vector Angle table.
    static const char *KVEC_SPHERE_FOCUS = "a"; ///< Focus attribute for K-Vector SphericalTriangle table.
    static const char *KVEC_PLANE_FOCUS = "a"; ///< Focus attribute for K-Vector PlanarTriangle table.
    static const char *KVEC_PYRAMID_FOCUS = "theta"; ///< Focus attribute for K-Vector Pyramid table.
    static const char *KVEC_HOKU_FOCUS = "cx"; ///< Focus attribute for K-Vector Hoku table.
}

/// Given the table choice, remove the given table and the K-Vector table if they exist.
///
/// @param choice Number associated with the table to remove.
void remove_table (const int choice) {
    auto choose_table = [&choice] () -> std::string {
        switch (choice) {
            case 0: return DCNT::BRIGHT_HIP_NAME;
            case 1: return DCNT::HIP_NAME;
            case 2: return DCNT::ANGLE_NAME;
            case 3: return DCNT::SPHERE_NAME;
            case 4: return DCNT::PLANE_NAME;
            case 5: return DCNT::PYRAMID_NAME;
            case 6: return DCNT::COIN_NAME;
            default: throw "Table choice is not within space {0, 1, 2, 3, 4, 5, 6}.";
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
/// @param choice Number associated with the table to generate.
void generate_table (const int choice) {
    auto display_result = [] (const int r) -> void {
        std::cout << ((r == Nibble::TABLE_NOT_CREATED) ? "Table already exists." : "Table was created successfully")
                  << std::endl;
    };
    
    switch (choice) {
        case 0: return display_result(Chomp().generate_bright_table());
        case 1: return display_result(Chomp().generate_hip_table());
        case 2: return display_result(Angle::generate_table(DCNT::FOV, DCNT::ANGLE_NAME));
        case 3: return display_result(Sphere::generate_table(DCNT::FOV, DCNT::SPHERE_NAME));
        case 4: return display_result(Plane::generate_table(DCNT::FOV, DCNT::PLANE_NAME));
            //        case 5: return (void) Pyramid::generate_sep_table(DCNT::FOV, DCNT::PYRAMID_NAME);
            //        case 6: return (void) Coin::generate_triangle_table(DCNT::FOV, DCNT::COIN_NAME);
        default: throw "Table choice is not within space {0, 1, 2, 3, 4, 5, 6}.";
    }
}

/// Given the table choice, attempt to generate the K-Vector for the specified table and the predefined focus attribute.
///
/// @param choice Number associated with the table to generate.
void generate_kvec_table (const int choice) {
    Chomp ch;
    
    // Polish the selected table. Create the K-Vector for the given table using the given focus.
    auto create_and_polish = [&ch] (const std::string &table, const std::string &focus) -> void {
        ch.select_table(table);
        ch.polish_table(focus);
        std::cout << ((ch.create_k_vector(focus) == Nibble::TABLE_NOT_CREATED) ? "K-Vector table already exists." :
                      "K-Vector table was created successfully.") << std::endl;
    };
    
    switch (choice) {
        case 0: throw "Cannot generate KVEC table for star catalog table.";
        case 1: throw "Cannot generate KVEC table for star catalog table.";
        case 2: return create_and_polish(DCNT::ANGLE_NAME, DCNT::KVEC_ANGLE_FOCUS);
        case 3: return create_and_polish(DCNT::SPHERE_NAME, DCNT::KVEC_SPHERE_FOCUS);
        case 4: return create_and_polish(DCNT::PLANE_NAME, DCNT::KVEC_PLANE_FOCUS);
        case 5: return create_and_polish(DCNT::PYRAMID_NAME, DCNT::KVEC_PYRAMID_FOCUS);
        case 6: return create_and_polish(DCNT::COIN_NAME, DCNT::KVEC_HOKU_FOCUS);
        default: throw "Table choice is not within space {0, 1, 2, 3, 4, 5, 6}.";
    }
}

/// Select the desired table generation methods given the first argument. In the second argument, indicate whether you
/// want to build a K-Vector table for this first table as well, or delete all tables associated with the first
/// argument.
///
/// @code{.cpp}
/// - 0 -> Produce the bright stars (< 6.0) table from the Hipparcos catalog.
/// - 1 -> Produce the hip stars (no magnitude restriction) table from the Hipparcos catalog.
/// - 2 -> Produce table for Angle method.
/// - 3 -> Produce table for SphericalTriangle method.
/// - 4 -> Produce table for PlanarTriangle method.
/// - 5 -> Produce table for Pyramid method.
/// - 6 -> Produce table for Hoku method.
///
/// - x k -> Produce K-Vector table for the given method (valid above 1).
/// - x d -> Delete all tables for the given method.
/// @endcode
/// @example
/// @code{.cpp}
/// # Produce the SEP table for the Angle method. After this is done, produce the K-Vector table for SEP.
/// GenerateN 2 k
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
    if ((argc >= 2 && !is_valid_arg(argv[1], {"0", "1", "2", "3", "4", "5", "6"}))
        || (argc == 3 && !is_valid_arg(argv[2], {"k", "d"}))) {
        std::cout << "Usage: GenerateN [0 - 6] ['k', 'd']" << std::endl;
        return -1;
    }
    
    // If desired, delete all tables related to the specified table.
    if (argc == 3 && std::string(argv[2]) == "d") {
        remove_table(static_cast<int> (strtol(argv[1], nullptr, 10)));
        return 0;
    }
    
    // Attempt to generate the specified table.
    generate_table(static_cast<int> (strtol(argv[1], nullptr, 10)));
    
    // If desired, generate the K-Vector for the specified table.
    if (argc == 3 && std::string(argv[2]) == "k") {
        generate_kvec_table(static_cast<int> (strtol(argv[1], nullptr, 10)));
    }
    
    return 0;
}