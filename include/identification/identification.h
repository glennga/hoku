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
        unsigned int gamma; ///< The minimum number of body-inertial matches.
        unsigned int nu_max; ///< Maximum number of query star comparisons before returning an empty list.
        std::unique_ptr<unsigned int> nu; ///< Pointer to the location to hold the count of query star comparisons.
        std::string table_name; ///< Name of the Nibble database table created with 'generate_sep_table'.
    };
    
    Star::list find_matches (const Star::list &candidates, const Rotation &q);

  protected:
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
