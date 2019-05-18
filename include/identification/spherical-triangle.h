/// @file spherical-triangle.h
/// @author Glenn Galvizo
///
/// Header file for SphericalTriangle class, which matches a set of body vectors (stars) to their inertial counter-parts
/// in the database.

#ifndef HOKU_SPHERICAL_TRIANGLE_H
#define HOKU_SPHERICAL_TRIANGLE_H

#include "benchmark/benchmark.h"
#include "base-triangle.h"

/// @brief Star identification class using spherical triangles.
class SphericalTriangle : public BaseTriangle {
public:
    std::vector<labels_list> query () override;
    StarsEither reduce () override;
    StarsEither identify () override;

    static int generate_table (const std::shared_ptr<Chomp> &ch, double fov, const std::string &table_name);

    static const int DEFAULT_TD_H;
    static const unsigned int QUERY_STAR_SET_SIZE;

    using BaseTriangle::BaseTriangle;

private:
    TrioVectorEither query_for_trios (const index_trio &) override;
};

/// Alias for the SphericalTriangle class. 'Sphere' distinguishes the process I am testing here enough from the 5 other
/// methods.
typedef SphericalTriangle Sphere;

#endif /* HOKU_SPHERICAL_TRIANGLE_H */