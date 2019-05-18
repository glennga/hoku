/// @file pyramid.h
/// @author Glenn Galvizo
///
/// Header file for Pyramid class, which matches a set of body vectors (stars) to their inertial counter-parts in the
/// database.

#ifndef HOKU_PYRAMID_H
#define HOKU_PYRAMID_H

#include "benchmark/benchmark.h"
#include "identification/angle.h"

/// @brief Star identification class using pyramids.
class Pyramid : public Identification {
public:
    std::vector<Identification::labels_list> query () override;
    StarsEither reduce () override;
    StarsEither identify () override;

    static int generate_table (const std::shared_ptr<Chomp> &ch, double fov, const std::string &table_name);

    static const unsigned int QUERY_STAR_SET_SIZE;

    using Identification::Identification;

protected:
    /// Alias for a list of catalog ID lists (STL vector of vectors of integers).
    using labels_list_list = std::vector<labels_list>;

    static const int NO_CONFIDENT_R_FOUND_EITHER;

    struct TriosEither {
        Star::trio result;
        int error = 0;
    };

    Star::list common (const labels_list_list &big_r_ab_ell, const labels_list_list &big_r_ac_ell,
                       const Star::list &removed);

    Star::list common (const labels_list_list &big_r_ae_ell, const labels_list_list &big_r_be_ell,
                       const labels_list_list &big_r_ce_ell, const Star::list &removed);

    virtual bool verification (const Star::trio &r, const Star::trio &b);
    virtual TriosEither find_catalog_stars (const Star::trio &);

    StarsEither identify_as_list (const Star::list &b);

private:
    /// Alias for a pair of catalog IDs (2-element STL array of integers).
    using label_pair = std::array<int, 2>;

    labels_list_list query_for_pairs (double);
};

#endif /* HOKU_PYRAMID_H */