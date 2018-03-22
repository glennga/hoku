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
        double sigma_1; ///< Sigma used for database queries.
        double sigma_2; ///< Sigma used for additional reduction (triangle, dot...).
        double sigma_3; ///< Sigma used for phi in the Dot method.
        double sigma_4; ///< Resultant of inertial->body rotation must within 3 * sigma_overlay of *a* body.
        unsigned int sql_limit; ///< While performing a SQL query, limit results by this number.
        bool no_reduction; ///< If false, the restrict |R| = 1 is lifted. R_1 is returned instead.
        bool favor_bright_stars; ///< If false, do not favor bright stars in resulting set.
        unsigned int nu_max; ///< Maximum number of query star comparisons before returning an empty list.
        std::shared_ptr<unsigned int> nu; ///< Pointer to the location to hold the count of query star comparisons.
        Rotation::wahba_function f; ///< Function to use to solve Wahba's problem with.
        std::string table_name; ///< Name of the Nibble database table created with 'generate_sep_table'.
    };
    
    /// Alias for a list of labels.
    using labels_list = std::vector<int>;
  
  public:
    Identification ();
    static void collect_parameters (Parameters &p, INIReader &cf, const std::string &identifier);
    
    Star::list find_positive_overlay (const Star::list &big_p, const Rotation &q, std::vector<int> &i);
    virtual std::vector<labels_list> query (const Star::list &s) = 0;
    virtual Star::list reduce () = 0;
    virtual Star::list identify () = 0;
    
    Rotation align ();
    Star::list identify_all ();
    
    static const int TABLE_ALREADY_EXISTS;
    
    static const Star::list NO_CONFIDENT_A;
    static const Star::list NO_CONFIDENT_R;
    
    static const Star::list EXCEEDED_NU_MAX;
    static const Parameters DEFAULT_PARAMETERS;
    
    static const double DEFAULT_SIGMA_QUERY;
    static const unsigned int DEFAULT_SQL_LIMIT;
    static const bool DEFAULT_NO_REDUCTION;
    static const bool DEFAULT_FAVOR_BRIGHT_STARS;
    static const double DEFAULT_SIGMA_4;
    static const unsigned int DEFAULT_NU_MAX;
    static const std::shared_ptr<unsigned int> DEFAULT_NU;
    static const Rotation::wahba_function DEFAULT_F;
    static const char *DEFAULT_TABLE_NAME;

#if !defined ENABLE_TESTING_ACCESS
    protected:
#endif
    void sort_brightness (std::vector<labels_list> &big_r_ell);
    Star::list find_positive_overlay (const Star::list &big_p, const Rotation &q);

#if !defined ENABLE_TESTING_ACCESS
    protected:
#endif
    /// The star set we are working with. The catalog ID values are all set to 0 here.
    Star::list big_i;
    
    /// Current working parameters.
    Parameters parameters;
    
    /// Chomp instance, gives us access to the Nibble database.
    Chomp ch;
    
    /// All stars in 'input' are fov degrees from the focus.
    double fov;
};

#endif /* HOKU_IDENTIFICATION_H */
