/// @file identification.h
/// @author Glenn Galvizo
///
/// Header file for Identification class, which holds all common data between all star identification processes.
/// The identification class serves as an abstract base for other identification procedures (Angle, Pyramid, etc...).
/// Contained in this class are shared members and functions between each identification procedure, as well as
/// a method for 'completing' the attitude determination process `align()`.

#ifndef HOKU_IDENTIFICATION_H
#define HOKU_IDENTIFICATION_H

#include <memory>

#include "benchmark/benchmark.h"
#include "storage/chomp.h"
#include "math/rotation.h"

/// @brief Abstract base class for all identification procedures.
class Identification {
public:
    template<class T>
    class Builder;

    using labels_list = std::vector<int>;
    struct LabelsEither {
        labels_list result;
        int error = 0;
    };
    struct StarsEither {
        Star::list result;
        int error = 0;
    };

public:
    virtual std::vector<labels_list> query () = 0;
    virtual StarsEither reduce () = 0;
    virtual StarsEither identify () = 0;

    unsigned int get_nu ();

    static const int TABLE_ALREADY_EXISTS;
    static const int NO_CONFIDENT_A_EITHER;
    static const int NO_CONFIDENT_R_EITHER;
    static const int EXCEEDED_NU_MAX_EITHER;

    Identification (const std::shared_ptr<Benchmark> &be, const std::shared_ptr<Chomp> &ch, double epsilon_1,
                    double epsilon_2, double epsilon_3, double epsilon_4, unsigned int nu_max,
                    const std::string &identifier, const std::string &table_name);

protected:
    double epsilon_1, epsilon_2, epsilon_3, epsilon_4;
    std::string identifier, table_name;
    std::shared_ptr<Benchmark> be;
    std::shared_ptr<Chomp> ch;
    unsigned int nu_max, nu;

    static Star::list find_positive_overlay (const Star::list &big_i, const Star::list &big_p, const Rotation &q,
                                             double epsilon);
};

template<class T>
class Identification::Builder {
public:
    Builder &using_chomp (const std::shared_ptr<Chomp> &cho) {
        ch = cho;
        return *this;
    }
    Builder &given_image (const std::shared_ptr<Benchmark> &ben) {
        be = ben;
        return *this;
    }
    Builder &using_epsilon_1 (double epsilon) {
        epsilon_1 = epsilon; // Epsilon used for database queries.
        return *this;
    }
    Builder &using_epsilon_2 (double epsilon) {
        epsilon_2 = epsilon; // Epsilon used for additional reduction (triangle, dot...).
        return *this;
    }
    Builder &using_epsilon_3 (double epsilon) {
        epsilon_3 = epsilon; // Epsilon used for phi in the Dot method.
        return *this;
    }
    Builder &using_epsilon_4 (double epsilon) {
        epsilon_4 = epsilon; // Resultant of inertial->body rotation must within epsilon_4 of *a* body.
        return *this;
    }
    Builder &limit_n_comparisons (unsigned int n) {
        nu_max = n; // Maximum number of query star comparisons before returning an empty list.
        return *this;
    }
    Builder &identified_by (const std::string &id) {
        identifier = id; // String identifier associated with this method.
        return *this;
    }
    Builder &with_table (const std::string &name) {
        table_name = name; // Name of table in nibble database to use to query for candidates.
        return *this;
    }
    std::shared_ptr<T> build () {
        return std::make_shared<T>(be, ch, epsilon_1, epsilon_2, epsilon_3, epsilon_4, nu_max, identifier, table_name);
    }

private:
    double epsilon_1, epsilon_2, epsilon_3, epsilon_4;
    std::string identifier, table_name;
    std::shared_ptr<Benchmark> be;
    std::shared_ptr<Chomp> ch;
    unsigned int nu_max;
};

#endif /* HOKU_IDENTIFICATION_H */
