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

/// Given the table choice, remove the given table and the K-Vector table if they exist.
///
/// @param choice Name associated with the table to remove.
/// @param cf Configuration file reader to use.
void remove_table (const std::string &choice, INIReader &cf) {
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
/// @param cf Configuration file reader to use.
void generate_table (const std::string &choice, INIReader &cf) {
    static auto display_result = [] (const int r) -> void {
        std::cout << ((r == Nibble::TABLE_NOT_CREATED) ?
                      "\nTable already exists." : "\nTable was created successfully.") << std::endl;
    };

    // Choice to function mapping.
    std::array<std::string, 7> choice_space = {"HIP", "ANGLE", "DOT", "SPHERE", "PLANE", "PYRAMID", "COMPOSITE"};
    std::array<int (*) (INIReader &), 7> function_space = {
            [] (INIReader &cf_t) -> int { return Angle::generate_table(cf_t); },
            Dot::generate_table,
            Sphere::generate_table,
            [] (INIReader &cf_t) -> int { return Plane::generate_table(cf_t); },
            Pyramid::generate_table,
            Composite::generate_table
    };

    // Convert our choice to upper case.
    std::string upper_choice;
    std::transform(choice.begin(), choice.end(), upper_choice.begin(), ::toupper);

    // To generate the HIP table, we just call Chomp.
    if (upper_choice == "HIP") {
        Chomp ch;
        return display_result(0);
    }

    // We now call the desired function. Messy but it works! Trust me!
    long d = std::distance(choice_space.begin(), std::find(choice_space.begin(), choice_space.end(),
                                                           upper_choice));
    return display_result(function_space[static_cast<int> (d)](cf));
}

/// Given the table choice, attempt to generate the K-Vector for the specified table and the predefined focus attribute.
///
/// @param choice Name associated with the table to generate.
/// @param cf Configuration file reader to use.
void generate_kvec_table (const std::string &choice, INIReader &cf) {
    std::array<std::string, 7> choice_space = {"HIP", "ANGLE", "DOT", "SPHERE", "PLANE", "PYRAMID", "COMPOSITE"};
    Chomp ch;

    // Polish the selected table. Create the K-Vector for the given table using the given focus.
    auto create_and_polish = [&ch, &cf] (const std::string &method) -> void {
        // Convert our choice to lowercase.
        std::string method_lowercase;
        std::transform(method.begin(), method.end(), method_lowercase.begin(), ::toupper);

        // Polish the table.
        ch.select_table(cf.Get("table-names", method, ""));
        ch.polish_table(cf.Get("table-focus", method, ""));

        std::string response = (ch.create_k_vector(cf.Get("table-focus", method, "")) == Nibble::TABLE_NOT_CREATED)
                               ? "K-Vector table already exists." : "K-Vector table was created successfully.";
        std::cout << response << std::endl;
    };

    // Convert our choice to upper case.
    std::string upper_choice;
    std::transform(choice.begin(), choice.end(), upper_choice.begin(), ::toupper);

    // We cannot generate a K vector for the HIP tables.
    if (upper_choice == "HIP") {
        std::cout << "Cannot generate KVEC table for star catalog table." << std::endl;
        exit(1);
    }

    return create_and_polish(upper_choice);
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
        std::transform(a.begin(), a.end(), a.begin(), ::tolower);

        return std::find(input_space.begin(), input_space.end(), a) != input_space.end();
    };

    /// INIReader to hold configuration associated with table generation.
    INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));

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
        remove_table(std::string(argv[1]), cf);
        return 0;
    }

    // Attempt to generate the specified table.
    generate_table(std::string(argv[1]), cf);

    // If desired, generate the K-Vector for the specified table.
    if (argc == 3 && strcmp(argv[2], "k") == 0) {
        generate_kvec_table(std::string(argv[1]), cf);
    }

    return 0;
}