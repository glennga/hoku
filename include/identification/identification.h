/// @file identification.h
/// @author Glenn Galvizo
///
/// Header file for Identification class, which holds all common data between all star identification processes.

#ifndef HOKU_IDENTIFICATION_H
#define HOKU_IDENTIFICATION_H

#include "benchmark/benchmark.h"

class Identification {
  public:
    // All identification methods must contain these parameters.
    struct Parameters {
        double sigma_query; ///< Query must be within 3 * sigma_query.
        unsigned int sql_limit; ///< While performing a SQL query, limit results by this number.
        double sigma_overlay; ///< Resultant of inertial->body rotation must within 3 * sigma_overlay of *a* body.
        double gamma; ///< The minimum percentage of body-inertial matches.
        unsigned int nu_max; ///< Maximum number of query star comparisons before returning an empty list.
        std::shared_ptr<unsigned int> nu; ///< Pointer to the location to hold the count of query star comparisons.
        std::string table_name; ///< Name of the Nibble database table created with 'generate_sep_table'.
    };
    
    /// Default sigma query for all identification methods.
    static constexpr double DEFAULT_SIGMA_QUERY = std::numeric_limits<double>::epsilon();
    
    /// Default SQL limit for all identification methods.
    static constexpr unsigned int DEFAULT_SQL_LIMIT = 500;
    
    /// Default sigma overlay (for matching) for all identification methods.
    static constexpr double DEFAULT_SIGMA_OVERLAY = std::numeric_limits<double>::epsilon();
    
    /// Default gamma (match minimum percentage) for all identification methods.
    static constexpr double DEFAULT_GAMMA = 0.66;
    
    /// Default nu max (comparison counts) for all identification methods.
    static constexpr unsigned int DEFAULT_NU_MAX = 50000;
    
    /// Default pointer to nu (comparison count) for all identification methods.
    static constexpr auto DEFAULT_NU = nullptr;
    
    /// Default table name for all identification methods.
    static constexpr const char *DEFAULT_TABLE_NAME = "NO_TABLE";
    
    /// Alias for a list of labels.
    using labels_list = std::vector<int>;
    
    /// Indicates that a table already exists upon a table creation attempt.
    static constexpr int TABLE_ALREADY_EXISTS = -1;
  
  public:
    Identification ();
    
    virtual std::vector<labels_list> experiment_query (const Star::list &s) = 0;
    virtual labels_list experiment_reduction () = 0;
    virtual Star::list experiment_alignment () = 0;
    
    static const Star::list NO_CONFIDENT_ALIGNMENT;
    static const labels_list NO_CANDIDATES_FOUND;
    
    static const Star::list EXCEEDED_NU_MAX;
    static const Star::list NO_CONFIDENT_MATCH_SET;
    static const Parameters DEFAULT_PARAMETERS;

#if !defined ENABLE_TESTING_ACCESS
  protected:
#endif
    Star::list find_matches (const Star::list &candidates, const Rotation &q);

#if !defined ENABLE_TESTING_ACCESS
  protected:
#endif
    /// The star set we are working with. The catalog ID values are all set to 0 here.
    Star::list input;
    
    /// Current working parameters.
    Parameters parameters;
    
    /// Chomp instance, gives us access to the Nibble database.
    Chomp ch;
    
    /// All stars in 'input' are fov degrees from the focus.
    double fov;
};

#endif /* HOKU_IDENTIFICATION_H */
