/// @file generate-n.cpp
/// @author Glenn Galvizo
///
/// Source file for the Nibble database generator. This populates all of the tables required for testing. This is
/// **not** meant to be used as is, rather is meant to be the entry point for the python script calling this.

#include <iostream>
#include <algorithm>
#include <libgen.h>

#include "identification/angle.h"
#include "identification/dot-angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"
#include "identification/composite-pyramid.h"

enum GenerateNArguments {
    DATABASE_LOCATION = 1,
    CATALOG_LOCATION = 2,
    HIP_NAME = 3,
    BRIGHT_NAME = 4,
    CURRENT_TIME = 5,
    MAGNITUDE_LIMIT = 6,
    FOV_LIMIT = 7,
    TABLE_TYPE = 8,
    TABLE_NAME = 9
};

using TableGenerator = int (*) (const std::shared_ptr<Chomp> &, double, const std::string &);
TableGenerator table_generator_factory (const std::string &choice) {
    std::map<std::string, TableGenerator> table_function_map;
    table_function_map["HIP"] = [] (const std::shared_ptr<Chomp> &, double, const std::string &) -> int {
        return 0;
    };
    table_function_map["ANGLE"] = Angle::generate_table;
    table_function_map["DOT"] = Dot::generate_table;
    table_function_map["SPHERE"] = Sphere::generate_table;
    table_function_map["PLANE"] = Plane::generate_table;
    table_function_map["PYRAMID"] = Pyramid::generate_table;
    table_function_map["COMPOSITE"] = Composite::generate_table;

    std::string upper_choice = choice;  // Convert our choice to upper case.
    std::transform(choice.begin(), choice.end(), upper_choice.begin(), ::toupper);

    if (table_function_map.find(upper_choice) == table_function_map.end())
        throw std::runtime_error("'table_type' must be in space [HIP, ANGLE, DOT, SPHERE, PLANE, PYRAMID, COMPOSITE].");

    return table_function_map[upper_choice];
}

int main (int, char *argv[]) {
    table_generator_factory(argv[GenerateNArguments::TABLE_TYPE])(
            std::make_shared<Chomp>(
                    Chomp::Builder()
                            .with_database_name(argv[GenerateNArguments::DATABASE_LOCATION])
                            .using_catalog(argv[GenerateNArguments::CATALOG_LOCATION])
                            .with_hip_name(argv[GenerateNArguments::HIP_NAME])
                            .with_bright_name(argv[GenerateNArguments::BRIGHT_NAME])
                            .using_current_time(argv[GenerateNArguments::CURRENT_TIME])
                            .limited_by_magnitude(std::stod(argv[GenerateNArguments::MAGNITUDE_LIMIT]))
                            .build()
            ),
            std::stod(argv[GenerateNArguments::FOV_LIMIT]),
            argv[GenerateNArguments::TABLE_NAME]
    );
}
