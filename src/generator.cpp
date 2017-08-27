/// @file generator.cpp
/// @author Glenn Galvizo
///
/// Source file for the base benchmark data generator. This generates all of the test cases each identification
/// method will have to solve.

#include "benchmark.h"

/// Defining characteristics of the benchmarks generated.
///
/// @code{.cpp}
/// Current number of permutations: 5 *                                   // 5
///                                (20 - 7.5) / 0.5 *                     // 25
///                                (10 - 1) *                             // 9
///                                (5 - 1) * (10 - 1) / 2 *               // 18
///                                (10 - 1) * (0.5 - 0.0000000001) / 0.2  // ~22
///                                -------------------------------------
///                                445,500 benchmarks generated.
/// @endcode
namespace DCBG {
    static const int TYPE_SIZE = 5; ///< Number of tests to store for each type.
    
    static const double FOV_MINIMUM = 7.5; ///< Minimum FOV to start from.
    static const double FOV_MAXIMUM = 20; ///< Maximum FOV to end at.
    static const double FOV_STEP = 0.5; ///< Amount of FOV to increment for each test.
    
    static const int EXTRA_MINIMUM = 1; ///< Minimum number of extra stars to add.
    static const int EXTRA_MAXIMUM = 10; ///< Maximum number of extra stars to add.
    
    static const int REMOVE_MINIMUM_N = 1; ///< Minimum number of dark spots to generate.
    static const int REMOVE_MAXIMUM_N = 5; ///< Maximum number of dark spots to generate.
    static const double REMOVE_MINIMUM_SIZE = 1; ///< Minimum dark spot radius.
    static const double REMOVE_MAXIMUM_SIZE = 10; ///< Maximum dark spot radius.
    static const double REMOVE_STEP = 2; ///< Amount of radius to increment for each test.
    
    static const int SHIFT_MINIMUM_N = 1; ///< Minimum number of stars to shift.
    static const int SHIFT_MAXIMUM_N = 10; ///< Minimum number of stars to shift.
    static const double SHIFT_MINIMUM_SIGMA = 0.0000000001; ///< Minimum sigma to shift stars.
    static const double SHIFT_MAXIMUM_SIGMA = 0.5; ///< Maximum sigma to shift stars.
    static const double SHIFT_STEP = 0.2; ///< Amount of sigma to increment for each test.
}

/// Alias for record function pointers.
typedef void (*record_function) (Nibble &, unsigned int &, double);

/// Drops the existing benchmark table in Nibble. This removes **ALL** previously generated benchmarks. 
///
/// @param nb Open Nibble connection.
void delete_existing_benchmark (Nibble &nb) {
    (*nb.db).exec("DROP TABLE IF EXISTS " + Benchmark::TABLE_NAME);
}

/// Records sets of clean benchmarks.
///
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
/// @param fov Current field-of-view.
void record_c (Nibble &nb, unsigned int &set_n, double fov) {
    Benchmark(fov, Star::chance(), Rotation::chance()).insert_into_nibble(nb, set_n++);
}

/// Records sets of benchmarks with additional stars.
/// 
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
/// @param fov Current field-of-view.
void record_e (Nibble &nb, unsigned int &set_n, double fov) {
    Benchmark b(fov, Star::chance(), Rotation::chance());
    for (int i = DCBG::EXTRA_MINIMUM; i <= DCBG::EXTRA_MAXIMUM; i++) {
        b.add_extra_light(i);
        b.insert_into_nibble(nb, set_n++);
    }
}

/// Records sets of benchmarks with removed stars.
/// 
/// @param nb Open Nibble connection.
/// @param set_n Reference to the current ID. This is logged with Nibble, and must **NOT** already exist in Nibble.
/// @param fov Current field-of-view.
void record_r (Nibble &nb, unsigned int &set_n, double fov) {
    Benchmark b(fov, Star::chance(), Rotation::chance());
    for (int i = DCBG::REMOVE_MINIMUM_N; i <= DCBG::REMOVE_MAXIMUM_N; i++) {
        for (double j = DCBG::REMOVE_MINIMUM_SIZE; j <= DCBG::REMOVE_MAXIMUM_SIZE; j += DCBG::REMOVE_STEP) {
            b.remove_light(i, j);
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
    Benchmark b(fov, Star::chance(), Rotation::chance());
    for (int i = DCBG::SHIFT_MINIMUM_N; i <= DCBG::SHIFT_MAXIMUM_N; i++) {
        for (double j = DCBG::SHIFT_MINIMUM_SIGMA; j <= DCBG::SHIFT_MAXIMUM_SIGMA; j += DCBG::SHIFT_STEP) {
            b.shift_light(i, j);
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
    Benchmark b(fov, Star::chance(), Rotation::chance());
    for (int i = DCBG::SHIFT_MINIMUM_N; i <= DCBG::SHIFT_MAXIMUM_N; i++) {
        for (double j = DCBG::SHIFT_MINIMUM_SIGMA; j <= DCBG::SHIFT_MAXIMUM_SIGMA; j += DCBG::SHIFT_STEP) {
            for (int k = DCBG::EXTRA_MINIMUM; k <= DCBG::EXTRA_MAXIMUM; k++) {
                b.add_extra_light(k);
                b.shift_light(i, j);
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
    Benchmark b(fov, Star::chance(), Rotation::chance());
    for (int i = DCBG::SHIFT_MINIMUM_N; i <= DCBG::SHIFT_MAXIMUM_N; i++) {
        for (double j = DCBG::SHIFT_MINIMUM_SIGMA; j <= DCBG::SHIFT_MAXIMUM_SIGMA; j += DCBG::SHIFT_STEP) {
            for (int m = DCBG::REMOVE_MINIMUM_N; m <= DCBG::REMOVE_MAXIMUM_N; m++) {
                for (double n = DCBG::REMOVE_MINIMUM_SIZE; n <= DCBG::REMOVE_MAXIMUM_SIZE; n += DCBG::REMOVE_STEP) {
                    b.remove_light(m, n);
                    b.shift_light(i, j);
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
    for (double fov = DCBG::FOV_MINIMUM; fov <= DCBG::FOV_MAXIMUM; fov += DCBG::FOV_STEP) {
        SQLite::Transaction transaction(*nb.db);
        for (int i = 0; i < DCBG::TYPE_SIZE; i++) {
            r(nb, set_n, fov);
        }
        
        // Record every FOV step.
        transaction.commit();
    }
}

/// Generate all of the base benchmark data. ALL OF IT. (☞ﾟヮﾟ)☞
///
/// @return 0 when finished.
int main () {
    std::vector<record_function> r_actions = {record_c, record_e, record_r, record_s, record_es, record_rs};
    unsigned int set_n = 0;
    Nibble nb;
    
    for (const record_function &r : r_actions) {
        record_benchmark(nb, set_n, r);
    }
    
    return 0;
}