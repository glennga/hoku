/// @file composite-pyramid.h
/// @author Glenn Galvizo
///
/// Header file for CompositePyramid class, which matches a set of body vectors (stars) to their inertial
/// counter-parts in the database.

#ifndef HOKU_COMPOSITE_PYRAMID_H
#define HOKU_COMPOSITE_PYRAMID_H

#include "benchmark/benchmark.h"
#include "identification/pyramid.h"

/// @brief Star identification class using pyramids and planar triangles.
class CompositePyramid : public Pyramid {
public:
    std::vector<Identification::labels_list> query () override;
    StarsEither reduce () override;

    static int generate_table (const std::shared_ptr<Chomp> &ch, double fov, const std::string &table_name);

    using Pyramid::Pyramid;

private:
    labels_list_list query_for_trios (double, double);
    bool verification (const Star::trio &r, const Star::trio &b) override;
    TriosEither find_catalog_stars (const Star::trio &) override;
};

/// Alias for the CompositePyramid class. 'Composite' distinguishes the process I am testing here enough from the 5
/// other methods.
typedef CompositePyramid Composite;

#endif /* HOKU_COMPOSITE_PYRAMID_H */