/// @file test-angle.cpp
/// @author Glenn Galvizo
///
/// Source file for all Angle class unit tests.

#define ENABLE_TESTING_ACCESS

#include "identification/angle.h"
#include "identification/dot-angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"
#include "identification/composite-pyramid.h"
#include "gmock/gmock.h"

// Import several matchers from Google Mock.
using testing::UnorderedElementsAre;
using testing::Contains;
using testing::Not;

std::array<std::shared_ptr<Identification>, 6> generate_identifiers () {
    std::shared_ptr<Chomp> ch = std::make_shared<Chomp>(
            Chomp::Builder()
                    .with_database_name(
                            std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/nibble.db"
                    )
                    .with_hip_name("HIP")
                    .with_bright_name("BRIGHT")
                    .using_catalog(
                            std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat"
                    )
                    .limited_by_magnitude(4.5)
                    .using_current_time("01-2018")
                    .build()
    );
    std::shared_ptr<Benchmark> be = std::make_shared<Benchmark>(
            Benchmark::Builder()
                    .using_chomp(ch)
                    .limited_by_m(4.5)
                    .limited_by_fov(20)
                    .build()
    );
    std::array<std::shared_ptr<Identification>, 6> identifiers;

    identifiers[0] = Identification::Builder<Angle>()
            .using_chomp(ch)
            .given_image(be)
            .identified_by("ANGLE")
            .with_table("ANGLE")
            .using_epsilon_1(0.00001)
            .using_epsilon_4(0.00001)
            .build();
    identifiers[1] = Identification::Builder<Dot>()
            .using_chomp(ch)
            .given_image(be)
            .identified_by("DOT")
            .with_table("DOT")
            .using_epsilon_1(0.00001)
            .using_epsilon_2(0.00001)
            .using_epsilon_3(0.00001)
            .build();
    identifiers[2] = Identification::Builder<Sphere>()
            .using_chomp(ch)
            .given_image(be)
            .identified_by("SPHERE")
            .with_table("SPHERE")
            .using_epsilon_1(0.00001)
            .using_epsilon_2(0.00001)
            .using_epsilon_4(0.00001)
            .build();
    identifiers[3] = Identification::Builder<Plane>()
            .using_chomp(ch)
            .given_image(be)
            .identified_by("PLANE")
            .with_table("PLANE")
            .using_epsilon_1(0.00001)
            .using_epsilon_2(0.00001)
            .using_epsilon_4(0.00001)
            .build();
    identifiers[4] = Identification::Builder<Pyramid>()
            .using_chomp(ch)
            .given_image(be)
            .identified_by("PYRAMID")
            .with_table("PYRAMID")
            .using_epsilon_1(0.00001)
            .build();
    identifiers[5] = Identification::Builder<Composite>()
            .using_chomp(ch)
            .given_image(be)
            .identified_by("COMPOSITE")
            .with_table("COMPOSITE")
            .using_epsilon_1(0.00001)
            .using_epsilon_2(0.00001)
            .using_epsilon_4(0.00001)
            .build();

    return identifiers;
}

TEST(Identification, Constructor) {
    std::array<std::shared_ptr<Identification>, 6> identifiers = generate_identifiers();
}

TEST (Identification, Identification) {
    std::array<std::shared_ptr<Identification>, 6> identifiers = generate_identifiers();

    for (const auto &identifier : identifiers) { identifier->identify(); }
}