/// @file planar-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for PlanarTriangle class, which matches a set of body vectors (stars) to their inertial counter-parts in
/// the database.

#include <algorithm>

#include "math/trio.h"
#include "identification/planar-triangle.h"

const unsigned int Plane::QUERY_STAR_SET_SIZE = 3;

int PlanarTriangle::generate_table (const std::shared_ptr<Chomp> &ch, double fov, const std::string &table_name) {
    return generate_triangle_table(ch, fov, table_name, Trio::planar_area, Trio::planar_moment);
}

Plane::TrioVectorEither Plane::query_for_trios (const index_trio &c) {
    return base_query_for_trios(c, Trio::planar_area, Trio::planar_moment);
}

std::vector<Identification::labels_list> Plane::query () {
    return e_query(Trio::planar_area(be->get_image()->at(0), be->get_image()->at(1), be->get_image()->at(2)),
                   Trio::planar_moment(be->get_image()->at(0), be->get_image()->at(1), be->get_image()->at(2)));
}

Plane::StarsEither Plane::reduce () { return e_reduction(); }
Plane::StarsEither Plane::identify () {
    std::cout << "[PLANE] Starting spherical identification." << std::endl;
    return e_identify();
}