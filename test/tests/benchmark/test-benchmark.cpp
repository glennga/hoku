/// @file test-benchmark.cpp
/// @author Glenn Galvizo
///
/// Source file for all Benchmark class unit tests.

#define ENABLE_TESTING_ACCESS

#include <fstream>
#include "gtest/gtest.h"

#include "benchmark/benchmark.h"

TEST(Benchmark, ConstructorRandomGenerator) {
    std::remove("/tmp/nibble.db");
    std::shared_ptr<Chomp> ch = std::make_shared<Chomp>(
            Chomp::Builder()
                    .with_database_name("/tmp/nibble.db")
                    .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
                    .limited_by_magnitude(6.0)
                    .using_current_time("01-2018")
                    .with_bright_name("HIP_BRIGHT")
                    .with_hip_name("HIP")
                    .build()
    );

    Benchmark be_1 = Benchmark::Builder()
            .using_chomp(ch)
            .limited_by_fov(50)
            .limited_by_n_stars(10)
            .build();
    Benchmark be_2 = Benchmark::Builder()
            .using_chomp(ch)
            .limited_by_fov(50)
            .limited_by_m(2.0)
            .build();

    EXPECT_NE(be_1.get_center(), be_2.get_center());
    EXPECT_NE(be_1.get_image(), be_2.get_image());
    EXPECT_NE(be_1.get_answers(), be_2.get_answers());
    EXPECT_NE(be_1.get_inertial(), be_2.get_inertial());
    EXPECT_DOUBLE_EQ(be_1.get_fov(), be_2.get_fov());

    EXPECT_EQ(be_1.get_image()->size(), 10);
    for (const Star &s : *be_2.get_image()) {
        EXPECT_LT(s.get_magnitude(), 2.0);
    }
}

TEST(Benchmark, ImageLabelClear) {
    std::remove("/tmp/nibble.db");
    std::shared_ptr<Chomp> ch = std::make_shared<Chomp>(
            Chomp::Builder()
                    .with_database_name("/tmp/nibble.db")
                    .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
                    .limited_by_magnitude(6.0)
                    .using_current_time("01-2018")
                    .with_bright_name("HIP_BRIGHT")
                    .with_hip_name("HIP")
                    .build()
    );

    Benchmark be = Benchmark::Builder()
            .using_chomp(ch)
            .limited_by_fov(50)
            .limited_by_n_stars(10)
            .build();

    for (const Star &s : *be.get_image()) {
        EXPECT_EQ(s.get_label(), Star::NO_LABEL);
    }
    for (const Star &s : *be.get_inertial()) {
        EXPECT_NE(s.get_label(), Star::NO_LABEL);
    }
}

TEST(Benchmark, ErrorNearFocus) {
    std::remove("/tmp/nibble.db");
    std::shared_ptr<Chomp> ch = std::make_shared<Chomp>(
            Chomp::Builder()
                    .with_database_name("/tmp/nibble.db")
                    .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
                    .limited_by_magnitude(6.0)
                    .using_current_time("01-2018")
                    .with_bright_name("HIP_BRIGHT")
                    .with_hip_name("HIP")
                    .build()
    );

    Benchmark be = Benchmark::Builder()
            .using_chomp(ch)
            .limited_by_fov(50)
            .limited_by_n_stars(10)
            .build();

    be.add_extra_light(3);
    be.remove_light(3, 40);
    be.shift_light(3, 1);
    for (const Star &s : *be.get_image()) {
        EXPECT_TRUE(Star::within_angle(s, be.get_center(), be.get_fov() / 2.0));
    }
}

TEST(Benchmark, ErrorExtraLightAdded) {
    std::remove("/tmp/nibble.db");
    std::shared_ptr<Chomp> ch = std::make_shared<Chomp>(
            Chomp::Builder()
                    .with_database_name("/tmp/nibble.db")
                    .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
                    .limited_by_magnitude(6.0)
                    .using_current_time("01-2018")
                    .with_bright_name("HIP_BRIGHT")
                    .with_hip_name("HIP")
                    .build()
    );

    Benchmark be = Benchmark::Builder()
            .using_chomp(ch)
            .limited_by_fov(50)
            .limited_by_n_stars(5)
            .build();

    be.add_extra_light(3);
    EXPECT_EQ(be.get_image()->size(), 8);
}

TEST(Benchmark, ErrorRemovedLightRemoved) {
    std::remove("/tmp/nibble.db");
    std::shared_ptr<Chomp> ch = std::make_shared<Chomp>(
            Chomp::Builder()
                    .with_database_name("/tmp/nibble.db")
                    .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
                    .limited_by_magnitude(6.0)
                    .using_current_time("01-2018")
                    .with_bright_name("HIP_BRIGHT")
                    .with_hip_name("HIP")
                    .build()
    );

    Benchmark be = Benchmark::Builder()
            .using_chomp(ch)
            .limited_by_fov(50)
            .limited_by_n_stars(5)
            .build();

    be.remove_light(10, 30);
    EXPECT_LT(be.get_image()->size(), 5);
}

TEST(Benchmark, ErrorShiftedLightMoved) {
    std::remove("/tmp/nibble.db");
    std::shared_ptr<Chomp> ch = std::make_shared<Chomp>(
            Chomp::Builder()
                    .with_database_name("/tmp/nibble.db")
                    .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
                    .limited_by_magnitude(6.0)
                    .using_current_time("01-2018")
                    .with_bright_name("HIP_BRIGHT")
                    .with_hip_name("HIP")
                    .build()
    );

    Benchmark be = Benchmark::Builder()
            .using_chomp(ch)
            .limited_by_fov(50)
            .limited_by_n_stars(10)
            .build();

    Star::list original = *be.get_image();
    be.shift_light(3, 0.1);

    int b = 0;  // |original|*|modified| = (number of different pairs) + |original| - 3
    for (Star org : original) {
        for (Star mod : *be.get_image()) {
            if (!(org.get_vector() == mod.get_vector())) b++;
        }
    }
    EXPECT_EQ(original.size() * be.get_image()->size(), b + original.size() - 3);
}

TEST(Benchmark, StarsShuffledInOrder) {
    std::remove("/tmp/nibble.db");
    std::shared_ptr<Chomp> ch = std::make_shared<Chomp>(
            Chomp::Builder()
                    .with_database_name("/tmp/nibble.db")
                    .using_catalog(std::string(dirname(const_cast<char *>(__FILE__))) + "/../../../data/hip2.dat")
                    .limited_by_magnitude(6.0)
                    .using_current_time("01-2018")
                    .with_bright_name("HIP_BRIGHT")
                    .with_hip_name("HIP")
                    .build()
    );

    Benchmark be = Benchmark::Builder()
            .using_chomp(ch)
            .limited_by_fov(50)
            .limited_by_n_stars(10)
            .build();

    for (unsigned int i = 0; i < be.get_inertial()->size(); i++) {
        EXPECT_EQ(be.get_inertial()->at(i).get_label(), be.get_answers()->at(i).get_label());
    }
}