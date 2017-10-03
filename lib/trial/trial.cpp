/// @file trial.cpp
/// @author Glenn Galvizo
///
/// Source file for the trials. This holds all of the functions required to iterate through all dimensions of 
/// a specific identification, and log the results.

#include "trial/trial.h"
#include "trial/character.h"

/// Quad-tree roots for the PlanarTriangle method. Restrict scope to this file.
std::array<std::shared_ptr<QuadNode>, DCPLA::BQT_ITER> pla_roots;

/// Quad-tree roots for the SphericalTriangle method. Restrict scope to this file.
std::array<std::shared_ptr<QuadNode>, DCSPH::BQT_ITER> sph_roots;

/// Star kd-tree roots for the AstrometryNet method. Restrict scope to this file.
std::array<std::shared_ptr<KdNode>, DCAST::BKT_ITER> ast_sta_roots;

/// Astro kd-tree roots for the AstrometryNet method. Restrict scope to this file.
std::array<std::shared_ptr<KdNode>, DCAST::BKT_ITER> ast_ast_roots;

/// Produce the quad-tree roots for the PlanarTriangle method. This **MUST** be run before calling the record function.
void Trial::generate_plane_trees () {
    for (int i = 0; i < DCPLA::BQT_ITER; i++) {
        pla_roots[i] = std::make_shared<QuadNode>(QuadNode::load_tree(DCPLA::BQT_MIN + DCPLA::BQT_STEP * i));
    }
}

/// Produce the quad-tree roots for the SphericalTriangle method. This **MUST** be run before calling the record 
/// function.
void Trial::generate_sphere_trees () {
    for (int i = 0; i < DCSPH::BQT_ITER; i++) {
        sph_roots[i] = std::make_shared<QuadNode>(QuadNode::load_tree(DCSPH::BQT_MIN + DCSPH::BQT_STEP * i));
    }
}

/// Produce the kd-tree roots for the AstrometryNet method. This **MUST** be run before calling the record function.
void Trial::generate_astro_trees () {
    Star::list astro_stars;
    Nibble nb;
    
    // Load ASTRO_C table into RAM.
    nb.select_table(DCAST::ASTROC_NAME);
    unsigned int n = (unsigned) (nb).search_table("MAX(rowid)", 1)[0];
    Nibble::tuple asterisms = (nb).search_table("i, j, k", n * 3);
    
    // Convert ASTRO_C table into stars.
    astro_stars.reserve(n / 3);
    for (unsigned int i = 0; i < n; i += 3) {
        Nibble::tuple astro_i = (nb).table_results_at(asterisms, 3, i);
        astro_stars.emplace_back(Star(astro_i[0], astro_i[1], astro_i[2]));
    }
    
    for (int i = 0; i < DCSPH::BQT_ITER; i++) {
        int kd_tree_w = DCAST::BKT_MIN + DCAST::BKT_STEP * i;
        ast_sta_roots[i] = std::make_shared<KdNode>(KdNode::load_tree(nb.all_bsc5_stars(), kd_tree_w));
        ast_ast_roots[i] = std::make_shared<KdNode>(KdNode::load_tree(astro_stars, kd_tree_w));
    }
}

/// Wrap three dimensions of Angle testing (query sigma, match sigma, match minimum) in a small function. Passed in the
/// working benchmark.
///
/// @param nb Open Nibble connection.
/// @param set_n Working Benchmark number.
/// @param log Open stream to log file.
void Trial::record_angle (Nibble &nb, const unsigned int set_n, std::ofstream &log) {
    Angle::Parameters p;
    p.table_name = DCANG::TABLE_NAME;
    Star::list s;
    double fov;
    
    for (int qs_i = 0; qs_i < DCANG::QS_ITER; qs_i++) {
        p.query_sigma = DCANG::QS_MIN + qs_i * DCANG::QS_STEP;
        for (int ms_i = 0; ms_i < DCANG::MS_ITER; ms_i++) {
            p.match_sigma = DCANG::MS_MIN + ms_i * DCANG::MS_STEP;
            for (unsigned int mm_i = 0; mm_i < DCANG::MM_ITER; mm_i++) {
                p.match_minimum = DCANG::MM_MIN + mm_i * DCANG::MM_STEP;
                
                // Read the benchmark, copy the list here.
                Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
                input.present_image(s, fov);
                unsigned int z = 0;
                
                // Identify the image, record the number of actual matches that exist.
                Star::list results = Angle::identify(input, p, z);
                int matches_found = Benchmark::compare_stars(input, results);
                
                log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
                log << p.query_sigma << "," << p.match_sigma << "," << p.match_minimum << "," << z << '\n';
            }
        }
    }
}

/// Wrap two dimensions of Plane testing (quadtree width and match minimum) in a small function. Passed in the working
/// benchmark and calls the helper method record_plane_as_ms_ms to test the other dimensions.
///
/// @param nb Open Nibble connection.
/// @param set_n Working Benchmark number.
/// @param log Open stream to log file.
void Trial::record_plane (Nibble &nb, const unsigned int set_n, std::ofstream &log) {
    Plane::Parameters p;
    p.table_name = DCPLA::TABLE_NAME;
    
    // We parse in an already-generated quad-tree from pla_roots. 
    for (unsigned int bqt_i = 0; bqt_i < DCPLA::BQT_ITER; bqt_i++) {
        p.quadtree_w = DCPLA::BQT_MIN + DCPLA::BQT_STEP * bqt_i;
        for (unsigned int mm_i = 0; mm_i < DCPLA::MM_ITER; mm_i++) {
            p.match_minimum = DCPLA::MM_MIN + DCPLA::MM_STEP * mm_i;
            record_plane_as_ms_ms(nb, set_n, log, pla_roots[bqt_i], p);
        }
    }
}

/// Wrap three dimensions of Plane testing (area sigma, moment sigma, and match sigma) in a small function. Passed in
/// the working benchmark, the match minimum, and the quadtree w.
///
/// @param nb Open Nibble connection.
/// @param set_n Working Benchmark number.
/// @param log Open stream to log file.
/// @param q_root Working quad-tree root. Generated with every iteration of quadtree_w.
/// @param p Working Plane parameters. Currently has quadtree_w and match_minimum set.
void Trial::record_plane_as_ms_ms (Nibble &nb, const unsigned int set_n, std::ofstream &log,
                                   std::shared_ptr<QuadNode> &q_root, Plane::Parameters &p) {
    Star::list s;
    double fov;
    
    for (int sa_i = 0; sa_i < DCPLA::SA_ITER; sa_i++) {
        p.sigma_a = DCPLA::SA_MIN + sa_i * DCPLA::SA_STEP;
        for (int si_i = 0; si_i < DCPLA::SI_ITER; si_i++) {
            p.sigma_i = DCPLA::SI_MIN + si_i * DCPLA::SI_STEP;
            for (int ms_i = 0; ms_i < DCPLA::MS_ITER; ms_i++) {
                p.match_sigma = DCPLA::MS_MIN + ms_i * DCPLA::MS_STEP;
                // Read the benchmark, copy the list here.
                Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
                input.present_image(s, fov);
                unsigned int z = 0;
                
                // Identify the image, record the number of actual matches that exist.
                Star::list results = Plane::identify(input, p, z, q_root);
                int matches_found = Benchmark::compare_stars(input, results);
                
                log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
                log << p.sigma_a << "," << p.sigma_i << "," << p.match_sigma << "," << p.match_minimum << ",";
                log << p.quadtree_w << "," << z << '\n';
            }
        }
    }
}

/// Wrap two dimensions of Sphere testing (quadtree width and match minimum) in a small function. Passed in the working
/// benchmark and calls the helper method record_sphere_as_ms_ms to test the other dimensions.
///
/// @param nb Open Nibble connection.
/// @param set_n Working Benchmark number.
/// @param log Open stream to log file.
void Trial::record_sphere (Nibble &nb, const unsigned int set_n, std::ofstream &log) {
    Sphere::Parameters p;
    p.table_name = DCSPH::TABLE_NAME;
    
    // We parse in an already-generated quad-tree from pla_roots. 
    for (unsigned int bqt_i = 0; bqt_i < DCSPH::BQT_ITER; bqt_i++) {
        p.quadtree_w = DCSPH::BQT_MIN + DCSPH::BQT_STEP * bqt_i;
        for (unsigned int mm_i = 0; mm_i < DCSPH::MM_ITER; mm_i++) {
            p.match_minimum = DCSPH::MM_MIN + DCSPH::MM_STEP * mm_i;
            record_sphere_as_ms_ms(nb, set_n, log, pla_roots[bqt_i], p);
        }
    }
}

/// Wrap three dimensions of Sphere testing (area sigma, moment sigma, and match sigma) in a small function. Passed in
/// the working benchmark, the match minimum, and the quadtree w.
///
/// @param nb Open Nibble connection.
/// @param set_n Working Benchmark number.
/// @param log Open stream to log file.
/// @param q_root Working quad-tree root. Generated with every iteration of quadtree_w.
/// @param p Working Sphere parameters. Currently has quadtree_w and match_minimum set.
void Trial::record_sphere_as_ms_ms (Nibble &nb, const unsigned int set_n, std::ofstream &log,
                                    std::shared_ptr<QuadNode> &q_root, Sphere::Parameters &p) {
    Star::list s;
    double fov;
    
    for (int sa_i = 0; sa_i < DCSPH::SA_ITER; sa_i++) {
        p.sigma_a = DCSPH::SA_MIN + sa_i * DCSPH::SA_STEP;
        for (int si_i = 0; si_i < DCSPH::SI_ITER; si_i++) {
            p.sigma_i = DCSPH::SI_MIN + si_i * DCSPH::SI_STEP;
            for (int ms_i = 0; ms_i < DCSPH::MS_ITER; ms_i++) {
                p.match_sigma = DCSPH::MS_MIN + ms_i * DCSPH::MS_STEP;
                // Read the benchmark, copy the list here.
                Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
                input.present_image(s, fov);
                unsigned int z = 0;
                
                // Identify the image, record the number of actual matches that exist.
                Star::list results = Sphere::identify(input, p, z, q_root);
                int matches_found = Benchmark::compare_stars(input, results);
                
                log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
                log << p.sigma_a << "," << p.sigma_i << "," << p.match_sigma << "," << p.match_minimum << ",";
                log << p.quadtree_w << "," << z << '\n';
            }
        }
    }
}

/// Wrap five dimensions of Astro testing (kd-tree width, u_tp, u_fp, u_tn, and u_fn) in a small function. Passed in
/// the working benchmark and calls the helper method record_astro_ka_qs_ms to test the other dimensions.
///
/// @param nb Open Nibble connection.
/// @param set_n Working Benchmark number.
/// @param log Open stream to log file.
void Trial::record_astro (Nibble &nb, const unsigned int set_n, std::ofstream &log) {
    Astro::Parameters p;
    
    // We parse in an already-generated quad-tree from ast_star_roots and ast_ast_roots.
    for (int kw_i = 0; kw_i <= DCAST::BKT_ITER; kw_i++) {
        p.kd_tree_w = (unsigned) DCAST::BKT_MIN + DCAST::BKT_STEP * kw_i;
        
        for (int u_tp_i = 0; u_tp_i < DCAST::UT_ITER; u_tp_i++) {
            p.u_tp = DCAST::UT_MIN + u_tp_i * DCAST::UT_STEP;
            for (int u_tn_i = 0; u_tn_i < DCAST::UT_ITER; u_tn_i++) {
                p.u_tn = DCAST::UT_MIN + u_tn_i * DCAST::UT_STEP;
                for (int u_fp_i = 0; u_fp_i < DCAST::UT_ITER; u_fp_i++) {
                    p.u_fp = DCAST::UT_MIN + u_fp_i * DCAST::UT_STEP;
                    for (int u_fn_i = 0; u_fn_i < DCAST::UT_ITER; u_fn_i++) {
                        p.u_fn = DCAST::UT_MIN + u_fn_i * DCAST::UT_STEP;
                        record_astro_ka_qs_ms(nb, set_n, log, ast_sta_roots[kw_i], ast_ast_roots[kw_i], p);
                    }
                }
            }
        }
    }
}

/// Wrap three dimensions of testing (bayes factor, query sigma, and match sigma) in a small function. Passed in the
/// working benchmark, the kd-tree w, and the utility points.
///
/// @param nb Open Nibble connection.
/// @param set_n Working Benchmark number.
/// @param log Open stream to log file.
/// @param star_root Working kd-tree nearby-stars root. Generated with every iteration of kd_tree_w.
/// @param astro_root Working kd-tree nearby-asterisms root. Generated with every iteration of kd_tree_w.
void Trial::record_astro_ka_qs_ms (Nibble &nb, const unsigned int set_n, std::ofstream &log,
                                   std::shared_ptr<KdNode> &star_root, std::shared_ptr<KdNode> &astro_root,
                                   Astro::Parameters &p) {
    Star::list s;
    double fov;
    
    for (int ka_i = 0; ka_i < DCAST::KAA_ITER; ka_i++) {
        p.k_accept = DCAST::KAA_MIN + ka_i * DCAST::KAA_STEP;
        for (int qs_i = 0; qs_i < DCAST::QS_ITER; qs_i++) {
            p.query_sigma = DCAST::QS_MIN + qs_i * DCAST::QS_STEP;
            for (int ms_i = 0; ms_i < DCAST::MS_ITER; ms_i++) {
                p.match_sigma = DCAST::MS_MIN + ms_i * DCAST::MS_STEP;
                
                // Read the benchmark, copy the list here.
                Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
                input.present_image(s, fov);
                unsigned int z = 0;
                
                // Identify the image, record the number of actual matches that exist.
                Star::list results = Astro::identify(input, p, z, star_root, astro_root);
                int matches_found = Benchmark::compare_stars(input, results);
                
                log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
                log << p.query_sigma << "," << p.match_sigma << "," << p.kd_tree_w << "," << p.k_accept << ",";
                log << p.u_fn << "," << p.u_fp << "," << p.u_tn << "," << p.u_tp << "," << z << '\n';
            }
        }
    }
}

/// Wrap two dimensions of Pyramid testing (quadtree width and match minimum) in a small function. Passed in the
/// working benchmark.
///
/// @param nb Open Nibble connection.
/// @param set_n Working Benchmark number.
/// @param log Open stream to log file.
void Trial::record_pyramid (Nibble &nb, const unsigned int set_n, std::ofstream &log) {
    Pyramid::Parameters p;
    p.table_name = DCPYR::TABLE_NAME;
    Star::list s;
    double fov;
    
    for (int qs_i = 0; qs_i < DCPYR::QS_ITER; qs_i++) {
        p.query_sigma = DCPYR::QS_MIN + qs_i * DCPYR::QS_STEP;
        for (int ms_i = 0; ms_i < DCPYR::MS_ITER; ms_i++) {
            p.match_sigma = DCPYR::MS_MIN + ms_i * DCPYR::MS_STEP;
            
            // Read the benchmark, copy the list here.
            Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
            input.present_image(s, fov);
            unsigned int z = 0;
            
            // Identify the image, record the number of actual matches that exist.
            Star::list results = Pyramid::identify(input, p, z);
            int matches_found = Benchmark::compare_stars(input, results);
            
            log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
            log << p.query_sigma << "," << p.match_sigma << "," << z << '\n';
        }
    }
}