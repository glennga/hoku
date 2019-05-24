/// @file base-triangle.h
/// @author Glenn Galvizo
///
/// Header file for BaseTriangle class, which holds all common methods between the Planar and Spherical triangle
/// methods (they are very similar).

#ifndef HOKU_BASE_TRIANGLE_H
#define HOKU_BASE_TRIANGLE_H

#include <deque>

#include "identification/identification.h"

/// @brief Abstract base class for both triangle star identification methods.
class BaseTriangle : public Identification {
    friend class CompositePyramid;

public:
    ~BaseTriangle () override = default;

protected:
    using index_trio = std::array<int, 3>;
    using area_function = double (*) (const Vector3 &, const Vector3 &, const Vector3 &);
    using moment_function =  double (*) (const Vector3 &, const Vector3 &, const Vector3 &);
    using Identification::Identification;


    template<typename T>
    T ptop (std::deque<T> &p) {
        T t = p.front();
        p.pop_front();
        return t;
    }

protected:
    struct TrioVectorEither {
        std::vector<Star::trio> result;
        int error = 0;
    };

    std::vector<labels_list> e_query (double a, double i);
    StarsEither e_reduction ();
    StarsEither e_identify ();

    static int generate_triangle_table (const std::shared_ptr<Chomp> &ch, double fov, const std::string &table_name,
                                        area_function compute_area, moment_function compute_moment);

    TrioVectorEither base_query_for_trios (const index_trio &c, area_function compute_area,
                                           moment_function compute_moment);

    static const index_trio STARTING_INDEX_TRIO;
    static const int NO_CANDIDATE_STARS_FOUND_EITHER;
    static const int NO_CANDIDATE_STAR_SET_FOUND_EITHER;

    virtual TrioVectorEither query_for_trios (const index_trio &) = 0;

private:
    std::deque<int> pivot_c;
    std::unique_ptr<std::vector<Star::trio>> big_r_1 = nullptr;

    struct TriosEither {
        Star::trio result;
        int error = 0;
    };

    void initialize_pivot (const index_trio & = {-1, -1, -1});
    std::vector<labels_list> query_for_trio (double a, double i);
    TriosEither pivot (const index_trio &);
    StarsEither direct_match_test (const Star::list &big_p, const Star::trio &r, const Star::trio &b);
};

#endif /* HOKU_BASE_TRIANGLE_H */