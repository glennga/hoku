/// @file dot-angle.h
/// @author Glenn Galvizo
///
/// Header file for DotAngle class, which matches a set of body vectors (stars) to their inertial counter-parts in
/// the database.

#ifndef HOKU_DOT_ANGLE_H
#define HOKU_DOT_ANGLE_H

#include "benchmark/benchmark.h"
#include "identification.h"

/// @brief Star identification class using dot product angles.
class DotAngle final : public Identification {
public:
    std::vector<Identification::labels_list> query () override;
    StarsEither reduce () override;
    StarsEither identify () override;

    static int generate_table (const std::shared_ptr<Chomp> &ch, double fov, const std::string &table_name);

    static const int NO_CANDIDATE_TRIO_FOUND_EITHER;
    static const int NO_CANDIDATES_FOUND_EITHER;
    static const unsigned int QUERY_STAR_SET_SIZE;

    using Identification::Identification;

private:
    struct TriosEither {
        Star::trio result;
        int error = 0;
    };

    Star::trio find_closest (const Star &b_i);
    LabelsEither query_for_trio (double theta_1, double theta_2, double phi);
    TriosEither find_candidate_trio (const Star &b_i, const Star &b_j, const Star &b_c);
};

/// Alias for the DotAngle class. 'Dot' distinguishes the process I am testing here enough from the 5 other methods.
typedef DotAngle Dot;

#endif /* HOKU_DOT_ANGLE_H */