/// @file identification.h
/// @author Glenn Galvizo
///
/// Header file for Identification class, which holds all common data between all star identification processes.

#ifndef HOKU_IDENTIFICATION_H
#define HOKU_IDENTIFICATION_H

#include <memory>
#include "third-party/inih/INIReader.h"

#include "storage/chomp.h"
#include "math/rotation.h"

/// @brief Abstract base class for all identification procedures.
///
/// The identification class serves as an abstract base for other identification procedures (Angle, Pyramid, etc...).
/// Contained in this class are shared members and functions between each identification procedure, as well as
/// a method for 'completing' the attitude determination process `align()`.
class Identification {
public:
    // All identification methods must contain these parameters.
    struct Parameters {
        /// Sigma used for database queries.
        double sigma_1 = std::numeric_limits<double>::epsilon() * 10000;

        /// Sigma used for additional reduction (triangle, dot...).
        double sigma_2 = std::numeric_limits<double>::epsilon() * 10000;

        /// Sigma used for phi in the Dot method.
        double sigma_3 = std::numeric_limits<double>::epsilon() * 10000;

        /// Resultant of inertial->body rotation must within 3 * sigma_overlay of *a* body.
        double sigma_4 = std::numeric_limits<double>::epsilon() * 10000;

        /// While performing a SQL query, limit results by this number.
        unsigned int sql_limit = 500;

        /// If true, the restrict |R| = 1 is lifted. R_1 is returned instead.
        bool no_reduction = false;

        /// If false, do not favor bright stars in resulting set.
        bool favor_bright_stars = false;

        /// Maximum number of query star comparisons before returning an empty list.
        unsigned int nu_max = 50000;

        /// Pointer to the location to hold the count of query star comparisons.
        std::shared_ptr<unsigned int> nu = nullptr;

        /// Function to use to solve Wahba's problem with.
        Rotation::wahba_function f = Rotation::triad;

        ///< Name of the Nibble database table created with 'generate_sep_table'.
        std::string table_name = "NO_TABLE_DEFINED";
    };

    /// Alias for a list of labels.
    using labels_list = std::vector<int>;

    // For errors when querying for a single label set, we define an "either" struct.
    struct labels_either {
        labels_list result; // Result associated with the computation.
        int error = 0; // Error associated with the computation.
    };

    // For errors when querying, we define an "either" struct.
    struct labels_vector_either {
        std::vector<labels_list> result; // Result associated with the computation.
        int error = 0; // Error associated with the computation.
    };

    // For errors when reducing or identifying, we define an "either" struct.
    struct stars_either {
        Star::list result; // Result associated with the computation.
        int error = 0; // Error associated with the computation.
    };

public:
    Identification ();

    static Parameters collect_parameters (INIReader &cf, const std::string &identifier);

    Star::list find_positive_overlay (const Star::list &big_p, const Rotation &q, std::vector<int> &i);

    virtual labels_vector_either query (const Star::list &s) = 0;

    virtual stars_either reduce () = 0;

    virtual stars_either identify () = 0;

    Rotation align ();

    Star::list identify_all ();

    static const int TABLE_ALREADY_EXISTS;

    static const int NO_CONFIDENT_A_EITHER;
    static const int NO_CONFIDENT_R_EITHER;
    static const int EXCEEDED_NU_MAX_EITHER;

#if !defined ENABLE_TESTING_ACCESS
    protected:
#endif

    void sort_brightness (std::vector<labels_list> &big_r_ell);

    Star::list find_positive_overlay (const Star::list &big_p, const Rotation &q);

#if !defined ENABLE_TESTING_ACCESS
    protected:
#endif
    /// Pointer to the star set we are working with. The catalog ID values should be all set to 0 here.
    std::unique_ptr<Star::list> big_i;

    /// Current working parameters.
    std::unique_ptr<Parameters> parameters;

    /// Chomp instance, gives us access to the Nibble database.
    Chomp ch;

    /// All stars in 'input' are fov degrees from the focus.
    double fov;
};

#endif /* HOKU_IDENTIFICATION_H */
