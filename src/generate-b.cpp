/// @file generate-b.cpp
/// @author Glenn Galvizo
///
/// Source file for the base benchmark data generator. This generates all of the test cases each identification
/// method will have to solve.

#include <iostream>
#include "benchmark/benchmark.h"

/// Defining characteristics of the benchmarks generated.
///
/// @code{.cpp}
/// Current number of dimensions:   Clean Data (2), Error data (3)
///
/// Current number of permutations: 3                                     // 3
///                                (20 - 7.5) / 2.5                       // 6
///                                (10 - 1)                               // 10
///                                (5 - 1) * (10 - 1) / 3                 // 20
///                                (5 - 1) * (0.4 - 0.000001) / 0.1       // 20
///                                -------------------------------------
///                                3 * 6 * (1 + 10 + 20 + 20) benchmarks generated (918).
/// @endcode
namespace DCBG {
    static const int DUP = 3; ///< Number of tests to store for each type.
    
    static const double FOV_MIN = 7.5; ///< Minimum FOV to start from.
    static const double FOV_MAX = 20; ///< Maximum FOV to end at.
    static const double FOV_STEP = 2.5; ///< Amount of FOV to increment for each test.
    
    static const int E_MIN = 1; ///< Minimum number of extra stars to add.
    static const int E_MAX = 10; ///< Maximum number of extra stars to add.
    
    static const int RN_MIN = 1; ///< Minimum number of dark spots to generate.
    static const int RN_MAX = 5; ///< Maximum number of dark spots to generate.
    static const double RS_MIN = 1; ///< Minimum dark spot radius.
    static const double RS_MAX = 10; ///< Maximum dark spot radius.
    static const double RS_STEP = 3; ///< Amount of radius to increment for each test.
    
    static const int SN_MIN = 1; ///< Minimum number of stars to shift.
    static const int SN_MAX = 5; ///< Maximum number of stars to shift.
    static const double SS_MIN = 0.000001; ///< Minimum sigma to shift stars.
    static const double SS_MAX = 0.4; ///< Maximum sigma to shift stars.
    static const double SS_STEP = 0.1; ///< Amount of sigma to increment for each test.
}

/// Alias for record function pointers.
typedef void (*record_function) (Nibble &, unsigned int &, double);

/// Drops the existing benchmark table in Nibble. This removes **ALL** previously generated benchmarks. 
///
/// @param nb Open Nibble connection.
void delete_existing_benchmark (Nibble &nb) {
    (*nb.db).exec("DROP TABLE IF EXISTS " + Benchmark::TABLE_NAME);
}

/// Return a non-empty benchmark. If we are unable to find a non-empty benchmark in N_BOUND iterations, halt.
///
/// @param fov Field of view of the benchmark to generate.
/// @param focus Focus star of the benchmark to generate.
/// @param q Rotation of the benchmark to generate.
/// @return A non-empty benchmark.
Benchmark non_empty_benchmark(double fov, const Star &focus, const Rotation &q) {
    // We set a practical limit here to avoid hang-ups.
    const int N_BOUND = 10000;
    Star::list s;
    double fov_b;

    for (int i = 0; i < N_BOUND; i++) {
        Benchmark b(fov, Star::chance(), Rotation::chance());
        b.present_image(s, fov_b);

        if (!s.empty()) {
            return b;
        }
    }

    // We shouldn't reach here.
    throw "Unable to find non-empty benchmark.";
}

/// Records sets of clean benchmarks.
///
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
/// @param fov Current field-of-view.
void record_c(Nibble &nb, unsigned int &set_n, double fov) {
    non_empty_benchmark(fov, Star::chance(), Rotation::chance()).insert_into_nibble(nb, set_n++);
}

/// Records sets of benchmarks with additional stars.
/// 
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
/// @param fov Current field-of-view.
void record_e (Nibble &nb, unsigned int &set_n, double fov) {
    for (int n_added = DCBG::E_MIN; n_added <= DCBG::E_MAX; n_added++) {
        Benchmark b = non_empty_benchmark(fov, Star::chance(), Rotation::chance());
        b.add_extra_light(n_added);
        b.insert_into_nibble(nb, set_n++);
    }
}

/// Records sets of benchmarks with removed stars.
/// 
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
/// @param fov Current field-of-view.
void record_r (Nibble &nb, unsigned int &set_n, double fov) {
    for (int n_spots = DCBG::RN_MIN; n_spots <= DCBG::RN_MAX; n_spots++) {
        for (double spot_radius = DCBG::RS_MIN; spot_radius <= DCBG::RS_MAX; spot_radius += DCBG::RS_STEP) {
            Benchmark b = non_empty_benchmark(fov, Star::chance(), Rotation::chance());
            b.remove_light(n_spots, spot_radius);
            b.insert_into_nibble(nb, set_n++);
        }
    }
}

/// Records sets of benchmarks with shifted stars.
/// 
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
/// @param fov Current field-of-view.
void record_s (Nibble &nb, unsigned int &set_n, double fov) {
    for (int n_shifted = DCBG::SN_MIN; n_shifted <= DCBG::SN_MAX; n_shifted++) {
        for (double shift_sigma = DCBG::SS_MIN; shift_sigma <= DCBG::SS_MAX; shift_sigma += DCBG::SS_STEP) {
            Benchmark b = non_empty_benchmark(fov, Star::chance(), Rotation::chance());
            b.shift_light(n_shifted, shift_sigma);
            b.insert_into_nibble(nb, set_n++);
        }
    }
}

/// Records sets of benchmarks with extra stars and shifted stars.
///
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
/// @param fov Current field-of-view.
void record_es (Nibble &nb, unsigned int &set_n, double fov) {
    for (int n_shifted = DCBG::SN_MIN; n_shifted <= DCBG::SN_MAX; n_shifted++) {
        for (double shift_sigma = DCBG::SS_MIN; shift_sigma <= DCBG::SS_MAX; shift_sigma += DCBG::SS_STEP) {
            for (int n_added = DCBG::E_MIN; n_added <= DCBG::E_MAX; n_added++) {
                Benchmark b = non_empty_benchmark(fov, Star::chance(), Rotation::chance());
                b.add_extra_light(n_added);
                b.shift_light(n_shifted, shift_sigma);
                b.insert_into_nibble(nb, set_n++);
            }
        }
    }
}

/// Records sets of benchmarks with removed stars and shifted stars.
///
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
/// @param fov Current field-of-view.
void record_rs (Nibble &nb, unsigned int &set_n, double fov) {
    for (int n_shifted = DCBG::SN_MIN; n_shifted <= DCBG::SN_MAX; n_shifted++) {
        for (double shift_sigma = DCBG::SS_MIN; shift_sigma <= DCBG::SS_MAX; shift_sigma += DCBG::SS_STEP) {
            for (int n_spots = DCBG::RN_MIN; n_spots <= DCBG::RN_MAX; n_spots++) {
                for (double spot_radius = DCBG::RS_MIN; spot_radius <= DCBG::RS_MAX; spot_radius += DCBG::RS_STEP) {
                    Benchmark b = non_empty_benchmark(fov, Star::chance(), Rotation::chance());
                    b.remove_light(n_spots, spot_radius);
                    b.shift_light(n_shifted, shift_sigma);
                    b.insert_into_nibble(nb, set_n++);
                }
            }
        }
    }
}

/// Wrap the function that actually records the benchmark in the field-of-view and type loops.
///
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
/// @param r Function pointer to the type of record operation being performed.
void record_benchmark (Nibble &nb, unsigned int &set_n, record_function r) {
    for (double fov = DCBG::FOV_MIN; fov <= DCBG::FOV_MAX; fov += DCBG::FOV_STEP) {
        SQLite::Transaction transaction(*nb.db);
        for (int i = 0; i < DCBG::DUP; i++) {
            r(nb, set_n, fov);
        }
        
        // Record every FOV step.
        std::cout << "Set Number: " << set_n << " ||| FOV: " << fov << std::endl;
        transaction.commit();
    }
}

/// Generate all of the base benchmark data. ALL OF IT. (☞ﾟヮﾟ)☞ **(Except for multi-error sets)**
///
/// @return 0 when finished.
int main () {
    //    std::vector<record_function> r_actions = {record_c, record_e, record_r, record_s, record_es, record_rs};
    std::vector<record_function> r_actions = {record_c, record_e, record_r, record_s};

    unsigned int set_n = 0;
    Nibble nb;
    
    for (const record_function &r : r_actions) {
        record_benchmark(nb, set_n, r);
    }
    
    return 0;
}