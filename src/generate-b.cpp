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
/// Dimension Domains:              fov exists in [7.5, 10.0, 12.5, 15.0, 17.5, 20.0]
///                                 e exists in [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
///                                 rn exists in [1, 2, 3, 4, 5]
///                                 rs exists in [1, 2, 4, 6, 8]
///                                 sn exists in [1, 2, 3, 4, 5]
///                                 ss exists in [0.000001, 0.100001, 0.200001, 0.300001, 0.400001]
///
/// Current number of permutations: 49250 benchmarks generated.
/// @endcode
namespace DCBG {
    static const int DUP = 20; ///< Number of tests to store for each type.

    static const double FOV_MIN = 7.5; ///< Minimum FOV to start from.
    static const double FOV_STEP = 2.5; ///< Amount of FOV to increment for each test.
    static const int FOV_ITER = 6; ///< Number of FOV iterations.

    static const int E_MIN = 1; ///< Minimum number of extra stars to add.
    static const int E_MAX = 10; ///< Maximum number of extra stars to add.

    static const int RN_MIN = 1; ///< Minimum number of dark spots to generate.
    static const int RN_MAX = 5; ///< Maximum number of dark spots to generate.
    static const double RS_MIN = 1; ///< Minimum dark spot radius.
    static const double RS_STEP = 3; ///< Amount of radius to increment for each test.
    static const int RS_ITER = 4; ///< Number of dark spot radius iterations.

    static const int SN_MIN = 1; ///< Minimum number of stars to shift.
    static const int SN_MAX = 5; ///< Maximum number of stars to shift.
    static const double SS_MIN = 0.000001; ///< Minimum sigma to shift stars.
    static const double SS_STEP = 0.1; ///< Amount of sigma to increment for each test.
    static const int SS_ITER = 5; ///< Number of shift sigma iterations.
}

/// Alias for record function pointers.
typedef void (*record_function)(Nibble &, unsigned int &);

/// Drops the existing benchmark table in Nibble. This removes **ALL** previously generated benchmarks. 
///
/// @param nb Open Nibble connection.
void delete_existing_benchmark(Nibble &nb) {
    (*nb.db).exec("DROP TABLE IF EXISTS " + std::string(Benchmark::TABLE_NAME));
}

/// Return a non-empty benchmark. If we are unable to find a non-empty benchmark in N_BOUND iterations, halt.
///
/// @param fov Field of view of the benchmark to generate.
/// @return A non-empty benchmark.
Benchmark non_empty_benchmark(double fov) {
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

/// Records sets of clean benchmarks. Vary the field-of-view.
///
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
/// @param fov Current field-of-view.
void record_c(Nibble &nb, unsigned int &set_n) {
    for (int fov_i = 0; fov_i < DCBG::FOV_ITER; fov_i++) {
        for (int i = 0; i < DCBG::DUP; i++) {
            non_empty_benchmark(DCBG::FOV_MIN + fov_i * DCBG::FOV_STEP).insert_into_nibble(nb, set_n++);
        }
    }
}

/// Records sets of benchmarks with additional stars. Use the maximum field-of-view (20).
/// 
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
void record_e(Nibble &nb, unsigned int &set_n) {
    for (int n_added = DCBG::E_MIN; n_added <= DCBG::E_MAX; n_added++) {
        for (int i = 0; i < DCBG::DUP; i++) {
            Benchmark b = non_empty_benchmark(DCBG::FOV_MIN + (DCBG::FOV_ITER - 1) * DCBG::FOV_STEP);
            b.add_extra_light(n_added);
            b.insert_into_nibble(nb, set_n++);
        }
    }
}

/// Records sets of benchmarks with removed stars. Use the maximum field-of-view (20).
/// 
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
void record_r(Nibble &nb, unsigned int &set_n) {
    for (int n_spots = DCBG::RN_MIN; n_spots <= DCBG::RN_MAX; n_spots++) {
        for (int sp_i = 0; sp_i < DCBG::RS_ITER; sp_i++) {
            for (int i = 0; i < DCBG::DUP; i++) {
                Benchmark b = non_empty_benchmark(DCBG::FOV_MIN + (DCBG::FOV_ITER - 1) * DCBG::FOV_STEP);
                b.remove_light(n_spots, DCBG::RS_MIN + sp_i * DCBG::RS_STEP);
                b.insert_into_nibble(nb, set_n++);
            }
        }
    }
}

/// Records sets of benchmarks with shifted stars. Use the maximum field-of-view (20).
/// 
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
void record_s(Nibble &nb, unsigned int &set_n) {
    for (int n_shifted = DCBG::SN_MIN; n_shifted <= DCBG::SN_MAX; n_shifted++) {
        for (int ss_i = 0; ss_i < DCBG::SS_ITER; ss_i++) {
            for (int i = 0; i < DCBG::DUP; i++) {
                Benchmark b = non_empty_benchmark(DCBG::FOV_MIN + (DCBG::FOV_ITER - 1) * DCBG::FOV_STEP);
                b.shift_light(n_shifted, DCBG::SS_MIN + DCBG::SS_STEP * ss_i);
                b.insert_into_nibble(nb, set_n++);
            }
        }
    }
}

/// Wrap the function that actually records the benchmark in the field-of-view and type loops.
///
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
/// @param r Function pointer to the type of record operation being performed.
void record_benchmark(Nibble &nb, unsigned int &set_n, record_function r) {
    SQLite::Transaction transaction(*nb.db);
    r(nb, set_n);

    transaction.commit();
}

/// Generate all of the base benchmark data. ALL OF IT. (☞ﾟヮﾟ)☞
///
/// @return 0 when finished.
int main() {
    std::vector<record_function> r_actions = {record_c, record_e, record_r, record_s};

    unsigned int set_n = 0;
    Nibble nb;

    for (const record_function &r : r_actions) {
        record_benchmark(nb, set_n, r);
    }

    return 0;
}