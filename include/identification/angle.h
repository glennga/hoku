/// @file angle.h
/// @author Glenn Galvizo
///
/// Header file for Angle class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database. The angle class is an implementation of Gottlieb's Angle method with Tappe's DMT process for alignment
///// determination. This is one of the six star identification procedures being tested.

#ifndef HOKU_ANGLE_H
#define HOKU_ANGLE_H

#include "benchmark/benchmark.h"
#include "identification.h"

/// @brief Star identification class using angles.
class Angle final : public Identification {
public:
    std::vector<Identification::labels_list> query () override;
    StarsEither reduce () override;
    StarsEither identify () override;

    static int generate_table (const std::shared_ptr<Chomp> &ch, double fov, const std::string &table_name);

    static const int NO_CANDIDATE_PAIR_FOUND_EITHER;
    static const int NO_CANDIDATES_FOUND_EITHER;
    static const unsigned int QUERY_STAR_SET_SIZE;

    using Identification::Identification;
    ~Angle () final = default;

private:
    struct PairsEither {
        Star::pair result;
        int error = 0;
    };

    LabelsEither query_for_pair (double theta);
    PairsEither find_candidate_pair (const Star &b_i, const Star &b_j);
    StarsEither direct_match_test (const Star::list &big_p, const Star::list &r, const Star::list &b);
};

#endif /* HOKU_ANGLE_H */