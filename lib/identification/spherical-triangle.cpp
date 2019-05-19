/// @file spherical-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for SphericalTriangle class, which matches a set of body vectors (stars) to their inertial counter-parts
/// in the database.

#include <algorithm>

#include "math/trio.h"
#include "identification/spherical-triangle.h"

const int Sphere::DEFAULT_TD_H = 3;
const unsigned int Sphere::QUERY_STAR_SET_SIZE = 3;

double Sphere::spherical_area (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3) {
    Trio::Either a = Trio::spherical_area(b_1, b_2, b_3);
    return (a.error == Trio::INVALID_TRIO_A_EITHER) ? -1 : a.result;
}
double Sphere::spherical_moment (const Vector3 &b_1, const Vector3 &b_2, const Vector3 &b_3) {
    Trio::Either i = Trio::spherical_moment(b_1, b_2, b_3, DEFAULT_TD_H);
    return (i.error == Trio::INVALID_TRIO_M_EITHER) ? -1 : i.result;
}

int Sphere::generate_table (const std::shared_ptr<Chomp> &ch, double fov, const std::string &table_name) {
    // Handle the error in the following lambdas. We define -1 to be an "error" result.
    return generate_triangle_table(ch, fov, table_name, Sphere::spherical_area, Sphere::spherical_moment);
}

Sphere::TrioVectorEither Sphere::query_for_trios (const index_trio &c) {
    return base_query_for_trios(c, Sphere::spherical_area, Sphere::spherical_moment);
}

std::vector<Identification::labels_list> Sphere::query () {
    Trio::Either a = Trio::spherical_area(be->get_image()->at(0), be->get_image()->at(1), be->get_image()->at(2));
    Trio::Either i = Trio::spherical_moment(be->get_image()->at(0), be->get_image()->at(1), be->get_image()->at(2));

    // If we have an invalid trio, then do not perform our query.
    if (a.error == Trio::INVALID_TRIO_A_EITHER || i.error == Trio::INVALID_TRIO_M_EITHER) {
        return {labels_list{}};
    }
    return e_query(a.result, i.result);
}

Sphere::StarsEither Sphere::reduce () { return e_reduction(); }
Sphere::StarsEither Sphere::identify () { return e_identify(); }