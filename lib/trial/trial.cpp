/// @file trial.cpp
/// @author Glenn Galvizo
///
/// Source file for the trials. This holds all of the functions required to iterate through all dimensions of 
/// a specific identification, and log the results.

#include "trial/trial.h"
#include "trial/character.h"

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
    
    for (p.query_sigma = DCANG::QS_MIN; p.query_sigma <= DCANG::QS_MAX; p.query_sigma += DCANG::QS_STEP) {
        for (p.match_sigma = DCANG::MS_MIN; p.match_sigma <= DCANG::MS_MAX; p.match_sigma += DCANG::MS_STEP) {
            for (p.match_minimum = DCANG::MM_MIN; p.match_minimum <= DCANG::MM_MAX; p.match_minimum += DCANG::MM_STEP) {
                // Read the benchmark, copy the list here.
                Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
                input.present_image(s, fov);
                unsigned int z = 0;
                
                // Identify the image, record the number of actual matches that exist.
                Star::list results = Angle::identify(input, p, z);
                int matches_found = Benchmark::compare_stars(input, results);
                
                log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
                log << p.query_sigma << "," << p.match_sigma << "," << p.match_minimum << "," <<  z << '\n';
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
    
    // Build the quadtree for the given W- as early as possible to avoid constant rebuilding.
    for (p.quadtree_w = DCPLA::BQT_MIN; p.quadtree_w <= DCPLA::BQT_MAX; p.quadtree_w += DCPLA::BQT_STEP) {
        std::shared_ptr<QuadNode> q_root = std::make_shared<QuadNode>(QuadNode::load_tree(p.quadtree_w));
        for (p.match_minimum = DCPLA::MM_MIN; p.match_minimum <= DCPLA::MM_MAX; p.match_minimum += DCPLA::MM_STEP) {
            record_plane_as_ms_ms(nb, set_n, log, q_root, p);
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
    
    for (p.sigma_a = DCPLA::SA_MIN; p.sigma_a <= DCPLA::SA_MAX; p.sigma_a += DCPLA::SA_STEP) {
        for (p.sigma_i = DCPLA::SI_MIN; p.sigma_i <= DCPLA::SI_MAX; p.sigma_i += DCPLA::SI_STEP) {
            for (p.match_sigma = DCPLA::MS_MIN; p.match_sigma <= DCPLA::MS_MAX; p.match_sigma += DCPLA::MS_STEP) {
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
    
    // Build the quadtree for the given W- as early as possible to avoid constant rebuilding.
    for (p.quadtree_w = DCSPH::BQT_MIN; p.quadtree_w <= DCSPH::BQT_MAX; p.quadtree_w += DCSPH::BQT_STEP) {
        std::shared_ptr<QuadNode> q_root = std::make_shared<QuadNode>(QuadNode::load_tree(p.quadtree_w));
        for (p.match_minimum = DCSPH::MM_MIN; p.match_minimum <= DCSPH::MM_MAX; p.match_minimum += DCSPH::MM_STEP) {
            record_sphere_as_ms_ms(nb, set_n, log, q_root, p);
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
    
    for (p.sigma_a = DCSPH::SA_MIN; p.sigma_a <= DCSPH::SA_MAX; p.sigma_a += DCSPH::SA_STEP) {
        for (p.sigma_i = DCSPH::SI_MIN; p.sigma_i <= DCSPH::SI_MAX; p.sigma_i += DCSPH::SI_STEP) {
            for (p.match_sigma = DCSPH::MS_MIN; p.match_sigma <= DCSPH::MS_MAX; p.match_sigma += DCSPH::MS_STEP) {
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
    p.hash_name = DCAST::ASTROH_NAME;
    p.center_name = DCAST::ASTROC_NAME;
    Star::list astro_stars;
    
    // Load ASTRO_C table into RAM.
    (nb).select_table(p.center_name);
    unsigned int n = (unsigned) (nb).search_table("MAX(rowid)", 1)[0];
    Nibble::tuple asterisms = (nb).search_table("i, j, k", n * 3);
    
    // Convert ASTRO_C table into stars.
    astro_stars.reserve(n / 3);
    for (unsigned int i = 0; i < n; i += 3) {
        Nibble::tuple astro_i = (nb).table_results_at(asterisms, 3, i);
        astro_stars.push_back(Star(astro_i[0], astro_i[1], astro_i[2]));
    }
    
    // Build the kd-trees for the given W- as early as possible to avoid constant rebuilding.
    for (p.kd_tree_w = DCAST::BKT_MIN; p.kd_tree_w <= DCAST::BKT_MAX; p.kd_tree_w += DCAST::BKT_STEP) {
        std::shared_ptr<KdNode> star_root = std::make_shared<
            KdNode>(KdNode::load_tree((nb).all_bsc5_stars(), p.kd_tree_w));
        std::shared_ptr<KdNode> astro_root = std::make_shared<KdNode>(KdNode::load_tree(astro_stars, p.kd_tree_w));
        
        for (p.u_tp = DCAST::UT_MIN; p.u_tp <= DCAST::UT_MAX; p.u_tp += DCAST::UT_STEP) {
            for (p.u_tn = DCAST::UT_MIN; p.u_tn <= DCAST::UT_MAX; p.u_tn += DCAST::UT_STEP) {
                for (p.u_fp = DCAST::UT_MIN; p.u_fp <= DCAST::UT_MAX; p.u_fp += DCAST::UT_STEP) {
                    for (p.u_fn = DCAST::UT_MIN; p.u_fn <= DCAST::UT_MAX; p.u_fn += DCAST::UT_STEP) {
                        record_astro_ka_qs_ms(nb, set_n, log, star_root, astro_root, p);
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
    
    for (p.k_accept = DCAST::KAA_MIN; p.k_accept <= DCAST::KAA_MAX; p.k_accept += DCAST::KAA_STEP) {
        for (p.query_sigma = DCAST::QS_MIN; p.query_sigma <= DCAST::QS_MAX; p.query_sigma += DCAST::QS_STEP) {
            for (p.match_sigma = DCAST::MS_MIN; p.match_sigma <= DCAST::MS_MAX; p.match_sigma += DCAST::MS_STEP) {
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
    
    for (p.query_sigma = DCPYR::QS_MIN; p.query_sigma <= DCPYR::QS_MAX; p.query_sigma += DCPYR::QS_STEP) {
        for (p.match_sigma = DCPYR::MS_MIN; p.match_sigma <= DCPYR::MS_MAX; p.match_sigma += DCPYR::MS_STEP) {
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