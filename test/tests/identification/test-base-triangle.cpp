/// @file test-base-triangle.cpp
/// @author Glenn Galvizo
///
/// Source file for all BaseTriangle class unit tests. Using PlanarTriangle as a proxy class for
/// testing.

#define ENABLE_TESTING_ACCESS

#include "gmock/gmock.h"

#include "identification/planar-triangle.h"
#include "math/trio.h"

// Import several matchers from Google Mock.
using testing::UnorderedElementsAre;
using testing::Each;
using testing::Contains;

/// Check that the base constructor initializes an empty pivot queue.
TEST(BaseTriangle, ConstructorEmptyPivotQueue) {
    Plane p(Benchmark::black(), Plane::DEFAULT_PARAMETERS);
    EXPECT_EQ(0, p.pivot_c.size());
}

/// Check that the correct stars are returned when a single trio is requested.
TEST(BaseTriangle, QueryCorrectInput) {
    Chomp ch;
    Benchmark input(ch, 15);
    Identification::Parameters p = Plane::DEFAULT_PARAMETERS;
    p.sigma_1 = p.sigma_2 = 1.0e-9;
    
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532)};
    std::sort(b.begin(), b.end(), [] (const Star &s_1, const Star &s_2) -> bool {
        return s_1.get_label() < s_2.get_label();
    });
    double a = Trio::planar_area(b[0], b[1], b[2]);
    double i = Trio::planar_moment(b[0], b[1], b[2]);
    input.b = b;
    
    Plane d(input, p);
    std::vector<BaseTriangle::labels_list> e = d.query_for_trio(a, i);
    
    EXPECT_EQ(e.size(), 1);
    EXPECT_THAT(e[0], Contains(input.b[0].get_label()));
    EXPECT_THAT(e[0], Contains(input.b[1].get_label()));
    EXPECT_THAT(e[0], Contains(input.b[2].get_label()));
}

/// Check that the correct result is returned when there are no trios found.
TEST(BaseTriangle, QueryNoCandidates) {
    Chomp ch;
    Benchmark input(ch, 15);
    Identification::Parameters p = Plane::DEFAULT_PARAMETERS;
    p.sigma_1 = p.sigma_2 = std::numeric_limits<double>::epsilon();
    
    Star::list b = {Star(1, 1, 1), Star(1.101, 1, 1), Star(1.11, 1, 1)};
    std::sort(b.begin(), b.end(), [] (const Star &s_1, const Star &s_2) -> bool {
        return s_1.get_label() < s_2.get_label();
    });
    double a = Trio::planar_area(b[0], b[1], b[2]);
    double i = Trio::planar_moment(b[0], b[1], b[2]);
    input.b = b;
    
    Plane d(input, p);
    std::vector<BaseTriangle::labels_list> e = d.query_for_trio(a, i);
    
    EXPECT_EQ(e.size(), 1);
    EXPECT_THAT(e[0], Contains(BaseTriangle::NO_CANDIDATE_TRIOS_FOUND[0][0]));
    EXPECT_THAT(e[0], Contains(BaseTriangle::NO_CANDIDATE_TRIOS_FOUND[0][1]));
    EXPECT_THAT(e[0], Contains(BaseTriangle::NO_CANDIDATE_TRIOS_FOUND[0][2]));
}

/// Check that stars are sorted by brightness.
TEST(BaseTriangle, QueryFavorBrightStarsFlag) {
    Chomp ch;
    Benchmark input(ch, 15);
    Identification::Parameters p = Plane::DEFAULT_PARAMETERS, p2 = Plane::DEFAULT_PARAMETERS;
    p.sigma_1 = p.sigma_2 = 0.00000001, p2.sigma_1 = p2.sigma_2 = 0.00000001, p.favor_bright_stars = true;
    p.no_reduction = true, p2.no_reduction = true, p.sql_limit = 100000, p2.sql_limit = 100000;
    
    Star::list b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532)};
    std::sort(b.begin(), b.end(), [] (const Star &s_1, const Star &s_2) -> bool {
        return s_1.get_label() < s_2.get_label();
    });
    double a = Trio::planar_area(b[0], b[1], b[2]);
    double i = Trio::planar_moment(b[0], b[1], b[2]);
    input.b = b;
    
    Plane d(input, p), d2(input, p2);
    std::vector<BaseTriangle::labels_list> e = d.query_for_trio(a, i);
    std::vector<BaseTriangle::labels_list> f = d2.query_for_trio(a, i);
    EXPECT_LT(ch.query_hip(e[0][0]).get_magnitude() + ch.query_hip(e[0][1]).get_magnitude()
              + ch.query_hip(e[0][2]).get_magnitude(),
              ch.query_hip(f[0][0]).get_magnitude() + ch.query_hip(f[0][1]).get_magnitude()
              + ch.query_hip(f[0][2]).get_magnitude());
}

/// Check that base query for trio does not return any matches when stars are out of the fov.
TEST(BaseTriangle, QueryTriosFOV) {
    Chomp ch;
    Plane a(Benchmark::black(), Plane::DEFAULT_PARAMETERS);
    Star b(0.998078771188383, -0.0350062881876723, 0.0511207031486225);
    Star c(0.998078771188383, -0.0350062881876723, 0.0511207);
    Star d(0.928454687492219, 0.132930961972911, 0.346844709665121);
    a.fov = 10, a.big_i = {b, c, d};
    
    std::vector<Star::trio> e = a.base_query_for_trios({0, 1, 2}, Trio::planar_area, Trio::planar_moment);
    EXPECT_EQ(e.size(), 1);
    EXPECT_EQ(e[0][0], BaseTriangle::NO_CANDIDATE_STARS_FOUND[0][0]);
    EXPECT_EQ(e[0][1], BaseTriangle::NO_CANDIDATE_STARS_FOUND[0][1]);
    EXPECT_EQ(e[0][2], BaseTriangle::NO_CANDIDATE_STARS_FOUND[0][2]);
}

/// Check that the correct result is returned when no trios exist.
TEST(BaseTriangle, QueryTriosNoCandidates) {
    Chomp ch;
    Plane a(Benchmark::black(), Plane::DEFAULT_PARAMETERS);
    Star::list b = {Star(1, 1, 1), Star(1.1, 1, 1), Star(1.11, 1, 1)};
    a.fov = 10, a.big_i = b, a.parameters.sigma_1 = a.parameters.sigma_2 = std::numeric_limits<double>::epsilon();
    
    std::vector<Star::trio> e = a.base_query_for_trios({0, 1, 2}, Trio::planar_area, Trio::planar_moment);
    EXPECT_EQ(e.size(), 1);
    EXPECT_EQ(e[0][0], BaseTriangle::NO_CANDIDATE_STARS_FOUND[0][0]);
    EXPECT_EQ(e[0][1], BaseTriangle::NO_CANDIDATE_STARS_FOUND[0][1]);
    EXPECT_EQ(e[0][2], BaseTriangle::NO_CANDIDATE_STARS_FOUND[0][2]);
}

/// Check that the correct results are returned with a clean input.
TEST(BaseTriangle, QueryTriosCorrectInput) {
    Chomp ch;
    Benchmark input(ch, 15);
    Identification::Parameters p = Plane::DEFAULT_PARAMETERS;
    p.sigma_1 = p.sigma_2 = 1.0e-9;
    Plane a(input, p);
    std::vector<Star::trio> b = a.base_query_for_trios({0, 1, 2}, Trio::planar_area, Trio::planar_moment);
    
    EXPECT_EQ(b.size(), 1);
    EXPECT_THAT(b[0], Contains(ch.query_hip(input.b[0].get_label())));
    EXPECT_THAT(b[0], Contains(ch.query_hip(input.b[1].get_label())));
    EXPECT_THAT(b[0], Contains(ch.query_hip(input.b[2].get_label())));
}

/// Check that the correct pivot list is generated.
TEST(BaseTriangle, PivotGenerated) {
    Chomp ch;
    Benchmark input(ch, 15);
    input.b = {ch.query_hip(1), ch.query_hip(2), ch.query_hip(3), ch.query_hip(4)};
    Plane p(input, Plane::DEFAULT_PARAMETERS);
    
    p.initialize_pivot();
    EXPECT_EQ(p.big_r_1, nullptr);
    EXPECT_EQ(p.pivot_c.size(), 4);
    EXPECT_EQ(p.pivot_c[0], 0);
    EXPECT_EQ(p.pivot_c[1], 1);
    EXPECT_EQ(p.pivot_c[2], 2);
    EXPECT_EQ(p.pivot_c[3], 3);
    
    std::vector<Star::trio> b = {Star::trio {Star(0, 0, 0), Star(0, 0, 0), Star(0, 0, 0)}};
    p.big_r_1 = std::make_unique<std::vector<Star::trio>>(b);
    p.initialize_pivot({0, 1});
    EXPECT_EQ(p.big_r_1, nullptr);
    EXPECT_EQ(p.pivot_c.size(), 2);
    EXPECT_EQ(p.pivot_c[0], 2);
    EXPECT_EQ(p.pivot_c[1], 3);
}

/// Check that a different result is returned when NO_REDUCTION is applied.
TEST(BaseTriangle, PivotDifferentResult) {
    Chomp ch;
    Benchmark input(ch, 20);
    Plane::Parameters p = Plane::DEFAULT_PARAMETERS, p2 = Plane::DEFAULT_PARAMETERS;
    input.b = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
        ch.query_hip(101909)};
    p.sigma_1 = p.sigma_2 = 10e-9, p2.sigma_1 = p2.sigma_2 = 10e-9, p.no_reduction = true;
    Plane c(input, p), d(input, p2);
    
    c.initialize_pivot({0, 1, 2}), d.initialize_pivot({0, 1, 2});
    Star::trio a = c.pivot({0, 1, 2}), b = d.pivot({0, 1, 2});
    
    EXPECT_NE(a[0], Plane::NO_CANDIDATE_STAR_SET_FOUND[0]);
    EXPECT_NE(a[1], Plane::NO_CANDIDATE_STAR_SET_FOUND[1]);
    EXPECT_NE(a[1], Plane::NO_CANDIDATE_STAR_SET_FOUND[2]);
    EXPECT_NE(b[0], Plane::NO_CANDIDATE_STAR_SET_FOUND[0]);
    EXPECT_NE(b[1], Plane::NO_CANDIDATE_STAR_SET_FOUND[1]);
    EXPECT_NE(b[2], Plane::NO_CANDIDATE_STAR_SET_FOUND[2]);
    EXPECT_NE(a[0], b[0]);
    EXPECT_NE(a[1], b[1]);
    EXPECT_NE(a[2], b[2]);
}

/// Check that the correct result is returned when no candidate stars are found.
TEST(BaseTriangle, PivotNoCandidateStars) {
    Chomp ch;
    Benchmark input(ch, 20);
    Plane::Parameters p = Plane::DEFAULT_PARAMETERS;
    input.b = {Star(1, 1, 1), Star(1.1, 1, 1), Star(1.11, 1, 1)};
    p.sigma_1 = p.sigma_2 = std::numeric_limits<double>::epsilon();
    Plane c(input, p);
    
    c.initialize_pivot({0, 1, 2});
    Star::trio a = c.pivot({0, 1, 2});
    
    EXPECT_EQ(a[0], Plane::NO_CANDIDATE_STAR_SET_FOUND[0]);
    EXPECT_EQ(a[1], Plane::NO_CANDIDATE_STAR_SET_FOUND[1]);
    EXPECT_EQ(a[1], Plane::NO_CANDIDATE_STAR_SET_FOUND[2]);
}

/// Check that the correct result is returned after performing pivots.
TEST(BaseTriangle, PivotCorrectInput) {
    Chomp ch;
    Benchmark input(ch, 20);
    Plane::Parameters p = Plane::DEFAULT_PARAMETERS;
    p.sigma_1 = p.sigma_2 = 1.0e-10;
    Plane c(input, p);
    
    c.initialize_pivot({0, 1, 2});
    Star::trio a = c.pivot({0, 1, 2});
    Identification::labels_list b = {a[0].get_label(), a[1].get_label(), a[2].get_label()};
    
    EXPECT_THAT(b, Contains(input.b[0].get_label()));
    EXPECT_THAT(b, Contains(input.b[1].get_label()));
    EXPECT_THAT(b, Contains(input.b[2].get_label()));
}

/// Check that the direct match test returns the correct set.
TEST(BaseTriangle, DirectMatchTest) {
    Chomp ch;
    Rotation q = Rotation::chance();
    Star::list n = {ch.query_hip(102531), ch.query_hip(95498), ch.query_hip(102532), ch.query_hip(101958),
        ch.query_hip(101909)};
    Star::list n_q = {Rotation::rotate(n[0], q), Rotation::rotate(n[1], q), Rotation::rotate(n[2], q)};
    Benchmark input(n_q, n_q[0], 20);
    Identification::Parameters p = Plane::DEFAULT_PARAMETERS;
    p.sigma_4 = 0.0001;
    Plane b(input, p);
    
    Star::list a = b.direct_match_test(n, {n[0], n[1], n[2]}, {input.b[0], input.b[1], input.b[2]});
    Star::list d = b.direct_match_test(n, {n[0], n[1], n[2]}, {input.b[1], input.b[0], input.b[2]});
    ASSERT_EQ(a.size(), 3);
    ASSERT_EQ(d.size(), 3);
    
    Identification::labels_list f = {a[0].get_label(), a[1].get_label(), a[2].get_label()};
    Identification::labels_list f_2 = {d[0].get_label(), d[1].get_label(), d[2].get_label()};
    EXPECT_THAT(f, Contains(n_q[0].get_label()));
    EXPECT_THAT(f, Contains(n_q[1].get_label()));
    EXPECT_THAT(f, Contains(n_q[2].get_label()));
    EXPECT_THAT(f_2, Contains(n_q[0].get_label()));
    EXPECT_THAT(f_2, Contains(n_q[1].get_label()));
    EXPECT_THAT(f_2, Contains(n_q[2].get_label()));
}