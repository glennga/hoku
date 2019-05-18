/// @file planar-triangle.h
/// @author Glenn Galvizo
///
/// Header file for PlanarTriangle class, which matches a set of body vectors (stars) to their inertial counter-parts
/// in the database.

#ifndef HOKU_PLANAR_TRIANGLE_H
#define HOKU_PLANAR_TRIANGLE_H

#include "benchmark/benchmark.h"
#include "base-triangle.h"

/// @brief Star identification class using planar triangles.
class PlanarTriangle final : public BaseTriangle {
public:
    std::vector<labels_list> query () override;
    StarsEither reduce () override;
    StarsEither identify () override;

    static int generate_table (const std::shared_ptr<Chomp> &ch, double fov, const std::string &table_name);

    static const unsigned int QUERY_STAR_SET_SIZE;

    using BaseTriangle::BaseTriangle;
    ~PlanarTriangle () final = default;

private:
    TrioVectorEither query_for_trios (const index_trio &) override;
};

/// Alias for the PlanarTriangle class. 'Plane' distinguishes the process I am testing here enough from the 5 other
/// methods.
typedef PlanarTriangle Plane;

#endif /* HOKU_PLANAR_TRIANGLE_H */