/// @file trial.cpp
/// @author Glenn Galvizo
///
/// Source file for the trials. This holds all of the functions required to iterate through all dimensions of 
/// a specific identification, and log the results.

#include "trial/trial.h"
#include "trial/character.h"

/// Produce the kd-tree roots for the AstrometryNet method.
/// 
/// @param star_root Reference to hold pointer to kd-tree star root.
/// @param astro_root Reference to hold pointer to kd-tree astro root.
/// @param kd_tree_w Width of the kd-trees to project to.
void Trial::generate_astro_trees(std::shared_ptr<KdNode> &star_root, std::shared_ptr<KdNode> &astro_root, 
                                 const int kd_tree_w) {
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

    // Save tree roots to star and astro root references.
    star_root = std::make_shared<KdNode>(KdNode::load_tree(nb.all_bsc5_stars(), kd_tree_w));
    astro_root = std::make_shared<KdNode>(KdNode::load_tree(nb.all_bsc5_stars(), kd_tree_w));
}

/// Identify the stars for all benchmarks in the given Nibble table using the default parameters.
/// 
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param start_bench Starting benchmark to record from.
void Trial::iterate_angle_set_n(Nibble &nb, std::ofstream &log, unsigned int start_bench) {
    Angle::Parameters p;
    p.table_name = DCANG::TABLE_NAME;
    
    // Find the end benchmark.
    nb.select_table(Benchmark::TABLE_NAME);
    const unsigned int BENCH_END = (unsigned int) nb.search_table("MAX(set_n)", 1, 1)[0];
    
    auto record_angle = [&log, &p](const Benchmark &input, const int set_n) {
        Star::list s;
        double fov;

        // Read the benchmark, copy the list here.
        input.present_image(s, fov);
        unsigned int z = 0;

        // Identify the image, record the number of actual matches that exist.
        Star::list results = Angle::identify(input, p, z);
        int matches_found = Benchmark::compare_stars(input, results);

        log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
        log << p.query_sigma << "," << p.match_sigma << "," << p.match_minimum << "," << z << '\n';
    };

    // Loop through all set_n, record results.
    for (unsigned int set_n = start_bench; set_n < BENCH_END; set_n++) {
        record_angle(Benchmark::parse_from_nibble(nb, set_n), set_n);
    }
}

/// Vary three dimensions of Angle testing (query sigma, match sigma, match minimum).
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Benchmark to operate with.
void Trial::iterate_angle_parameters(Nibble &nb, std::ofstream &log, unsigned int set_n) {
    Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
    Star::list s;
    double fov;

    input.present_image(s, fov);
    auto record_angle = [&log, &input, &s, &set_n](Angle::Parameters &p) {
        p.table_name = DCANG::TABLE_NAME;
        unsigned int z = 0;

        // Identify the image, record the number of actual matches that exist.
        Star::list results = Angle::identify(input, p, z);
        int matches_found = Benchmark::compare_stars(input, results);

        log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
        log << p.query_sigma << "," << p.match_sigma << "," << p.match_minimum << "," << z << '\n';
    };

    // Vary query sigma. Every other parameter is the default.
    for (int qs_i = 0; qs_i < DCANG::QS_ITER; qs_i++) {
        Angle::Parameters p;
        p.query_sigma = DCANG::QS_MIN + qs_i * DCANG::QS_STEP;
        record_angle(p);
    }

    // Vary match sigma. Every other parameter is the default.
    for (int ms_i = 0; ms_i < DCANG::MS_ITER; ms_i++) {
        Angle::Parameters p;
        p.match_sigma = DCANG::MS_MIN + ms_i * DCANG::MS_STEP;
        record_angle(p);
    }

    // Vary match minimum. Every other parameter is the default.
    for (unsigned int mm_i = 0; mm_i < DCANG::MM_ITER; mm_i++) {
        Angle::Parameters p;
        p.match_minimum = DCANG::MM_MIN + mm_i * DCANG::MM_STEP;
        record_angle(p);
    }
}

/// Identify the stars for all benchmarks in the given Nibble table using the default parameters.
/// 
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param start_bench Starting benchmark to record from.
void Trial::iterate_plane_set_n(Nibble &nb, std::ofstream &log, unsigned int start_bench) {
    Plane::Parameters p;
    p.table_name = DCPLA::TABLE_NAME;
    const int BQT_MAX = DCPLA::BQT_MIN + DCPLA::BQT_STEP * DCPLA::BQT_ITER;
    std::shared_ptr<QuadNode> default_root = std::make_shared<QuadNode>(QuadNode::load_tree(BQT_MAX));

    // Find the end benchmark.
    nb.select_table(Benchmark::TABLE_NAME);
    const unsigned int BENCH_END = (unsigned int) nb.search_table("MAX(set_n)", 1, 1)[0];

    auto record_plane = [&log, &p, &default_root](const Benchmark &input, const int set_n) {
        Star::list s;
        double fov;

        // Read the benchmark, copy the list here.
        input.present_image(s, fov);
        unsigned int z = 0;

        // Identify the image, record the number of actual matches that exist.
        Star::list results = Plane::identify(input, p, z, default_root);
        int matches_found = Benchmark::compare_stars(input, results);

        log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
        log << p.sigma_a << "," << p.sigma_i << "," << p.match_sigma << "," << p.match_minimum << ",";
        log << p.quadtree_w << "," << z << '\n';
    };

    // Loop through all set_n, record results.
    for (unsigned int set_n = start_bench; set_n < BENCH_END; set_n++) {
        record_plane(Benchmark::parse_from_nibble(nb, set_n), set_n);
    }
}

/// Vary two dimensions of Plane testing (quad-tree width and match minimum). Call iterate_plane_helper to finish
/// testing the other dimensions.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Benchmark to operate with.
void Trial::iterate_plane_parameters(Nibble &nb, std::ofstream &log, unsigned int set_n) {
    Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
    const int BQT_MAX = DCPLA::BQT_MIN + DCPLA::BQT_STEP * DCPLA::BQT_ITER;
    std::shared_ptr<QuadNode> default_root = std::make_shared<QuadNode>(QuadNode::load_tree(BQT_MAX));
    Star::list s;
    double fov;

    input.present_image(s, fov);
    auto record_plane = [&log, &input, &s, &set_n](Plane::Parameters &p, const std::shared_ptr<QuadNode> &q_root) {
        p.table_name = DCPLA::TABLE_NAME;
        unsigned int z = 0;

        // Identify the image, record the number of actual matches that exist.
        Star::list results = Plane::identify(input, p, z, q_root);
        int matches_found = Benchmark::compare_stars(input, results);

        log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
        log << p.sigma_a << "," << p.sigma_i << "," << p.match_sigma << "," << p.match_minimum << ",";
        log << p.quadtree_w << "," << z << '\n';
    };

    // Vary quad-tree width. Every other parameter is the default.
    for (unsigned int bqt_i = 0; bqt_i < DCPLA::BQT_ITER; bqt_i++) {
        Plane::Parameters p;
        p.quadtree_w = DCPLA::BQT_MIN + DCPLA::BQT_STEP * bqt_i;
        record_plane(p, std::make_shared<QuadNode>(QuadNode::load_tree(DCPLA::BQT_MIN + DCPLA::BQT_STEP * bqt_i)));
    }

    // Vary match minimum. Every other parameter is the default.
    for (unsigned int mm_i = 0; mm_i < DCPLA::MM_ITER; mm_i++) {
        Plane::Parameters p;
        p.match_minimum = DCPLA::MM_MIN + DCPLA::MM_STEP * mm_i;
        record_plane(p, default_root);
    }

    Trial::iterate_plane_helper(nb, log, set_n);
}

/// Wrap three dimensions of Plane testing (area sigma, moment sigma, and match sigma) in a small function. 
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Benchmark to operate with.
void Trial::iterate_plane_helper(Nibble &nb, std::ofstream &log, unsigned int set_n) {
    Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
    const int BQT_MAX = DCPLA::BQT_MIN + DCPLA::BQT_STEP * DCPLA::BQT_ITER;
    std::shared_ptr<QuadNode> default_root = std::make_shared<QuadNode>(QuadNode::load_tree(BQT_MAX));
    Star::list s;
    double fov;

    input.present_image(s, fov);
    auto record_plane = [&log, &input, &s, &default_root, &set_n](Plane::Parameters &p) {
        p.table_name = DCPLA::TABLE_NAME;
        unsigned int z = 0;

        // Identify the image, record the number of actual matches that exist.
        Star::list results = Plane::identify(input, p, z, default_root);
        int matches_found = Benchmark::compare_stars(input, results);

        log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
        log << p.sigma_a << "," << p.sigma_i << "," << p.match_sigma << "," << p.match_minimum << ",";
        log << p.quadtree_w << "," << z << '\n';
    };

    // Vary area sigma. Every other parameter is the default.
    for (int sa_i = 0; sa_i < DCPLA::SA_ITER; sa_i++) {
        Plane::Parameters p;
        p.sigma_a = DCPLA::SA_MIN + sa_i * DCPLA::SA_STEP;
        record_plane(p);
    }

    // Vary moment sigma. Every other parameter is the default.
    for (int si_i = 0; si_i < DCPLA::SI_ITER; si_i++) {
        Plane::Parameters p;
        p.sigma_i = DCPLA::SI_MIN + si_i * DCPLA::SI_STEP;
        record_plane(p);
    }
    
    // Vary match sigma. Every other parameter is the default.
    for (int ms_i = 0; ms_i < DCPLA::MS_ITER; ms_i++) {
        Plane::Parameters p;
        p.match_sigma = DCPLA::MS_MIN + ms_i * DCPLA::MS_STEP;
        record_plane(p);
    }
}

/// Identify the stars for all benchmarks in the given Nibble table using the default parameters.
/// 
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param start_bench Starting benchmark to record from.
void Trial::iterate_sphere_set_n(Nibble &nb, std::ofstream &log, unsigned int start_bench) {
    Sphere::Parameters p;
    p.table_name = DCSPH::TABLE_NAME;
    const int BQT_MAX = DCSPH::BQT_MIN + DCSPH::BQT_STEP * DCSPH::BQT_ITER;
    std::shared_ptr<QuadNode> default_root = std::make_shared<QuadNode>(QuadNode::load_tree(BQT_MAX));

    // Find the end benchmark.
    nb.select_table(Benchmark::TABLE_NAME);
    const unsigned int BENCH_END = (unsigned int) nb.search_table("MAX(set_n)", 1, 1)[0];

    auto record_sphere = [&log, &p, &default_root](const Benchmark &input, const int set_n) {
        Star::list s;
        double fov;

        // Read the benchmark, copy the list here.
        input.present_image(s, fov);
        unsigned int z = 0;

        // Identify the image, record the number of actual matches that exist.
        Star::list results = Sphere::identify(input, p, z, default_root);
        int matches_found = Benchmark::compare_stars(input, results);

        log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
        log << p.sigma_a << "," << p.sigma_i << "," << p.match_sigma << "," << p.match_minimum << ",";
        log << p.quadtree_w << "," << z << '\n';
    };

    // Loop through all set_n, record results.
    for (unsigned int set_n = start_bench; set_n < BENCH_END; set_n++) {
        record_sphere(Benchmark::parse_from_nibble(nb, set_n), set_n);
    }
}

/// Vary two dimensions of Sphere testing (quad-tree width and match minimum). Call iterate_sphere_helper to finish
/// testing the other dimensions.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Benchmark to operate with.
void Trial::iterate_sphere_parameters(Nibble &nb, std::ofstream &log, unsigned int set_n) {
    Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
    const int BQT_MAX = DCSPH::BQT_MIN + DCSPH::BQT_STEP * DCSPH::BQT_ITER;
    std::shared_ptr<QuadNode> default_root = std::make_shared<QuadNode>(QuadNode::load_tree(BQT_MAX));
    Star::list s;
    double fov;

    input.present_image(s, fov);
    auto record_sphere = [&log, &input, &s, &set_n](Sphere::Parameters &p, const std::shared_ptr<QuadNode> &q_root) {
        p.table_name = DCSPH::TABLE_NAME;
        unsigned int z = 0;

        // Identify the image, record the number of actual matches that exist.
        Star::list results = Sphere::identify(input, p, z, q_root);
        int matches_found = Benchmark::compare_stars(input, results);

        log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
        log << p.sigma_a << "," << p.sigma_i << "," << p.match_sigma << "," << p.match_minimum << ",";
        log << p.quadtree_w << "," << z << '\n';
    };

    // Vary quad-tree width. Every other parameter is the default.
    for (unsigned int bqt_i = 0; bqt_i < DCSPH::BQT_ITER; bqt_i++) {
        Sphere::Parameters p;
        p.quadtree_w = DCSPH::BQT_MIN + DCSPH::BQT_STEP * bqt_i;
        record_sphere(p, std::make_shared<QuadNode>(QuadNode::load_tree(DCSPH::BQT_MIN + DCSPH::BQT_STEP * bqt_i)));
    }

    // Vary match minimum. Every other parameter is the default.
    for (unsigned int mm_i = 0; mm_i < DCSPH::MM_ITER; mm_i++) {
        Sphere::Parameters p;
        p.match_minimum = DCSPH::MM_MIN + DCSPH::MM_STEP * mm_i;
        record_sphere(p, default_root);
    }

    Trial::iterate_sphere_helper(nb, log, set_n);
}

/// Wrap three dimensions of Sphere testing (area sigma, moment sigma, and match sigma) in a small function. 
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Benchmark to operate with.
void Trial::iterate_sphere_helper(Nibble &nb, std::ofstream &log, unsigned int set_n) {
    Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
    const int BQT_MAX = DCSPH::BQT_MIN + DCSPH::BQT_STEP * DCSPH::BQT_ITER;
    std::shared_ptr<QuadNode> default_root = std::make_shared<QuadNode>(QuadNode::load_tree(BQT_MAX));
    Star::list s;
    double fov;

    input.present_image(s, fov);
    auto record_sphere = [&log, &input, &s, &default_root, &set_n](Sphere::Parameters &p) {
        p.table_name = DCSPH::TABLE_NAME;
        unsigned int z = 0;

        // Identify the image, record the number of actual matches that exist.
        Star::list results = Sphere::identify(input, p, z, default_root);
        int matches_found = Benchmark::compare_stars(input, results);

        log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
        log << p.sigma_a << "," << p.sigma_i << "," << p.match_sigma << "," << p.match_minimum << ",";
        log << p.quadtree_w << "," << z << '\n';
    };

    // Vary area sigma. Every other parameter is the default.
    for (int sa_i = 0; sa_i < DCSPH::SA_ITER; sa_i++) {
        Sphere::Parameters p;
        p.sigma_a = DCSPH::SA_MIN + sa_i * DCSPH::SA_STEP;
        record_sphere(p);
    }

    // Vary moment sigma. Every other parameter is the default.
    for (int si_i = 0; si_i < DCSPH::SI_ITER; si_i++) {
        Sphere::Parameters p;
        p.sigma_i = DCSPH::SI_MIN + si_i * DCSPH::SI_STEP;
        record_sphere(p);
    }

    // Vary match sigma. Every other parameter is the default.
    for (int ms_i = 0; ms_i < DCSPH::MS_ITER; ms_i++) {
        Sphere::Parameters p;
        p.match_sigma = DCSPH::MS_MIN + ms_i * DCSPH::MS_STEP;
        record_sphere(p);
    }
}

/// Identify the stars for all benchmarks in the given Nibble table using the default parameters.
/// 
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param start_bench Starting benchmark to record from.
void Trial::iterate_astro_set_n(Nibble &nb, std::ofstream &log, unsigned int start_bench) {
    Astro::Parameters p;
    p.center_name = DCAST::ASTROC_NAME, p.hash_name = DCAST::ASTROH_NAME;
    const int BKT_MAX = DCAST::BKT_MIN + DCAST::BKT_STEP * DCAST::BKT_ITER;
    std::shared_ptr<KdNode> default_star_root, default_astro_root;

    // Find the end benchmark.
    nb.select_table(Benchmark::TABLE_NAME);
    const unsigned int BENCH_END = (unsigned int) nb.search_table("MAX(set_n)", 1, 1)[0];

    generate_astro_trees(default_star_root, default_astro_root, BKT_MAX);
    auto record_astro = [&log, &p, &default_star_root, &default_astro_root](const Benchmark &input, const int set_n) {
        Star::list s;
        double fov;

        // Read the benchmark, copy the list here.
        input.present_image(s, fov);
        unsigned int z = 0;

        // Identify the image, record the number of actual matches that exist.
        Star::list results = Astro::identify(input, p, z, default_star_root, default_astro_root);
        int matches_found = Benchmark::compare_stars(input, results);

        log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
        log << p.query_sigma << "," << p.match_sigma << "," << p.kd_tree_w << "," << p.k_accept << ",";
        log << p.u_fn << "," << p.u_fp << "," << p.u_tn << "," << p.u_tp << "," << z << '\n';
    };

    // Loop through all set_n, record results.
    for (unsigned int set_n = start_bench; set_n < BENCH_END; set_n++) {
        record_astro(Benchmark::parse_from_nibble(nb, set_n), set_n);
    }
}

/// Wrap five dimensions of Astro testing (kd-tree width, u_tp, u_fp, u_tn, and u_fn). Call iterate_astro_helper to 
/// finish testing the other dimensions.
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Benchmark to operate with.
void Trial::iterate_astro_parameters(Nibble &nb, std::ofstream &log, unsigned int set_n) {
    Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
    const int BKT_MAX = DCAST::BKT_MIN + DCAST::BKT_STEP * DCAST::BKT_ITER;
    std::shared_ptr<KdNode> default_star_root, default_astro_root;
    Star::list s;
    double fov;
    
    input.present_image(s, fov);
    generate_astro_trees(default_star_root, default_astro_root, BKT_MAX);
    auto record_astro = [&log, &input, &s, &set_n](Astro::Parameters &p, const std::shared_ptr<KdNode> &star_root,
                                           const std::shared_ptr<KdNode> &astro_root) {
        p.center_name = DCAST::ASTROC_NAME, p.hash_name = DCAST::ASTROH_NAME;
        unsigned int z = 0;

        // Identify the image, record the number of actual matches that exist.
        Star::list results = Astro::identify(input, p, z, star_root, astro_root);
        int matches_found = Benchmark::compare_stars(input, results);

        log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
        log << p.query_sigma << "," << p.match_sigma << "," << p.kd_tree_w << "," << p.k_accept << ",";
        log << p.u_fn << "," << p.u_fp << "," << p.u_tn << "," << p.u_tp << "," << z << '\n';
    };

    // Vary kd-tree width. Every other parameter is the default.
    for (int kw_i = 0; kw_i < DCAST::BKT_ITER; kw_i++) {
        Astro::Parameters p;
        p.kd_tree_w = (unsigned) DCAST::BKT_MIN + DCAST::BKT_STEP * kw_i;
        std::shared_ptr<KdNode> star_root, astro_root;
        
        generate_astro_trees(default_star_root, default_astro_root, (int) p.kd_tree_w);
        record_astro(p, star_root, astro_root);
    }
    
    // Vary utility of true positive. Every other parameter is the default.
    for (int u_tp_i = 0; u_tp_i < DCAST::UT_ITER; u_tp_i++) {
        Astro::Parameters p;
        p.u_tp = DCAST::UT_MIN + u_tp_i * DCAST::UT_STEP;
        record_astro(p, default_star_root, default_astro_root);
    }

    // Vary utility of true negative. Every other parameter is the default.
    for (int u_tn_i = 0; u_tn_i < DCAST::UT_ITER; u_tn_i++) {
        Astro::Parameters p;
        p.u_tn = DCAST::UT_MIN + u_tn_i * DCAST::UT_STEP;
        record_astro(p, default_star_root, default_astro_root);
    }

    // Vary utility of false positive. Every other parameter is the default.
    for (int u_fp_i = 0; u_fp_i < DCAST::UT_ITER; u_fp_i++) {
        Astro::Parameters p;
        p.u_fp = DCAST::UT_MIN + u_fp_i * DCAST::UT_STEP;
        record_astro(p, default_star_root, default_astro_root);
    }

    // Vary utility of false negative. Every other parameter is the default.
    for (int u_fn_i = 0; u_fn_i < DCAST::UT_ITER; u_fn_i++) {
        Astro::Parameters p;
        p.u_fn = DCAST::UT_MIN + u_fn_i * DCAST::UT_STEP;
        record_astro(p, default_star_root, default_astro_root);
    }

    iterate_astro_helper(nb, log, set_n);
}

/// Wrap three dimensions of Astro testing (bayes factor, query sigma, and match sigma). 
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Benchmark to operate with.
void Trial::iterate_astro_helper(Nibble &nb, std::ofstream &log, unsigned int set_n) {
    Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
    const int BKT_MAX = DCAST::BKT_MIN + DCAST::BKT_STEP * DCAST::BKT_ITER;
    std::shared_ptr<KdNode> default_star_root, default_astro_root;
    Star::list s;
    double fov;

    input.present_image(s, fov);
    generate_astro_trees(default_star_root, default_astro_root, BKT_MAX);
    auto record_astro = [&log, &input, &s, &default_star_root, &default_astro_root, &set_n](Astro::Parameters &p) {
        p.center_name = DCAST::ASTROC_NAME, p.hash_name = DCAST::ASTROH_NAME;
        unsigned int z = 0;

        // Identify the image, record the number of actual matches that exist.
        Star::list results = Astro::identify(input, p, z, default_star_root, default_astro_root);
        int matches_found = Benchmark::compare_stars(input, results);

        log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
        log << p.query_sigma << "," << p.match_sigma << "," << p.kd_tree_w << "," << p.k_accept << ",";
        log << p.u_fn << "," << p.u_fp << "," << p.u_tn << "," << p.u_tp << "," << z << '\n';
    };
    
    // Vary the bayes factor. Every other parameter is constant.
    for (int ka_i = 0; ka_i < DCAST::KAA_ITER; ka_i++) {
        Astro::Parameters p;
        p.k_accept = DCAST::KAA_MIN + ka_i * DCAST::KAA_STEP;
        record_astro(p);
    }
    
    // Vary the query sigma. Every other parameter is constant.
    for (int qs_i = 0; qs_i < DCAST::QS_ITER; qs_i++) {
        Astro::Parameters p;
        p.query_sigma = DCAST::QS_MIN + qs_i * DCAST::QS_STEP;
        record_astro(p);
    }

    // Vary the match sigma. Every other parameter is constant.
    for (int ms_i = 0; ms_i < DCAST::MS_ITER; ms_i++) {
        Astro::Parameters p;
        p.match_sigma = DCAST::MS_MIN + ms_i * DCAST::MS_STEP;
        record_astro(p);
    }
}

/// Identify the stars for all benchmarks in the given Nibble table using the default parameters.
/// 
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param start_bench Starting benchmark to record from.
void Trial::iterate_pyramid_set_n(Nibble &nb, std::ofstream &log, unsigned int start_bench) {
    Pyramid::Parameters p;
    p.table_name = DCPYR::TABLE_NAME;

    // Find the end benchmark.
    nb.select_table(Benchmark::TABLE_NAME);
    const unsigned int BENCH_END = (unsigned int) nb.search_table("MAX(set_n)", 1, 1)[0];

    auto record_pyramid = [&log, &p](const Benchmark &input, const int set_n) {
        Star::list s;
        double fov;

        // Read the benchmark, copy the list here.
        input.present_image(s, fov);
        unsigned int z = 0;

        // Identify the image, record the number of actual matches that exist.
        Star::list results = Pyramid::identify(input, p, z);
        int matches_found = Benchmark::compare_stars(input, results);

        log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
        log << p.query_sigma << "," << p.match_sigma << "," << z << '\n';
    };

    // Loop through all set_n, record results.
    for (unsigned int set_n = start_bench; set_n < BENCH_END; set_n++) {
        record_pyramid(Benchmark::parse_from_nibble(nb, set_n), set_n);
    }
}

/// Wrap two dimensions of Pyramid testing (quadtree width and match minimum).
///
/// @param nb Open Nibble connection.
/// @param log Open stream to log file.
/// @param set_n Benchmark to operate with.
void Trial::iterate_pyramid_parameters(Nibble &nb, std::ofstream &log, unsigned int set_n) {
    Benchmark input = Benchmark::parse_from_nibble(nb, set_n);
    Star::list s;
    double fov;

    input.present_image(s, fov);
    auto record_pyramid = [&log, &input, &s, &set_n](Pyramid::Parameters &p) {
        p.table_name = DCPYR::TABLE_NAME;

        // Read the benchmark, copy the list here.
        unsigned int z = 0;

        // Identify the image, record the number of actual matches that exist.
        Star::list results = Pyramid::identify(input, p, z);
        int matches_found = Benchmark::compare_stars(input, results);

        log << set_n << "," << s.size() << "," << results.size() << "," << matches_found << ",";
        log << p.query_sigma << "," << p.match_sigma << "," << z << '\n';
    };

    // Vary query sigma. Every other parameter is the default.
    for (int qs_i = 0; qs_i < DCPYR::QS_ITER; qs_i++) {
        Pyramid::Parameters p;
        p.query_sigma = DCPYR::QS_MIN + qs_i * DCPYR::QS_STEP;
        record_pyramid(p);
    }

    // Vary match sigma. Every other parameter is the default.
    for (int ms_i = 0; ms_i < DCPYR::MS_ITER; ms_i++) {
        Pyramid::Parameters p;
        p.match_sigma = DCPYR::MS_MIN + ms_i * DCPYR::MS_STEP;
        record_pyramid(p);
    }
}