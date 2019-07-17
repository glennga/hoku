/// @file process-i.cpp
/// @author Glenn Galvizo
///
/// Source file for a rudimentary end-to-end star identification program. This includes reading from the camera,
/// identifying centroids, projecting these to 3D, and identifying stars. This is only meant to show how long the
/// entire star identification process to compared to identification times without the image processing component.
/// This is also **not** meant to be used as is, rather it is meant to be the entry point for the python script
/// calling this.

#include "third-party/cxxtimer/cxxtimer.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <numeric>

#include "identification/angle.h"
#include "identification/dot-angle.h"
#include "identification/spherical-triangle.h"
#include "identification/planar-triangle.h"
#include "identification/pyramid.h"
#include "identification/composite-pyramid.h"

enum ProcessIArguments {
    REFERENCE_DB = 1,
    HIP_TABLE = 2,
    BRIGHT_TABLE = 3,
    REFERENCE_TABLE = 4,
    IDENTIFICATION_STRATEGY = 5,
    EPSILON_1 = 6,
    EPSILON_2 = 7,
    EPSILON_3 = 8,
    EPSILON_4 = 9,
    NU_LIMIT = 10,
    SAMPLES = 11,
    FOV = 12,
    BKB_SZ = 13,
    THRES = 14,
    DPP_X = 15,
    DPP_Y = 16,
    MAX_X = 17,
    MAX_Y = 18
};

template<class T>
std::shared_ptr<T> create_generic_identifier (char *argv[], const std::shared_ptr<Star::list> &b) {
    std::shared_ptr<Chomp> ch = std::make_shared<Chomp>(
            Chomp::Builder()
                    .with_bright_name(argv[ProcessIArguments::BRIGHT_TABLE])
                    .with_hip_name(argv[ProcessIArguments::HIP_TABLE])
                    .with_database_name(argv[ProcessIArguments::REFERENCE_DB])
                    .build()
    );
    std::shared_ptr<Benchmark> be = std::make_shared<Benchmark>(
            Benchmark::Builder()
                    .limited_by_fov(std::stod(argv[ProcessIArguments::FOV]))
                    .using_chomp(ch)
                    .using_stars(b)
                    .build()
    );

    return Identification::Builder<T>()
            .using_chomp(ch)
            .given_image(be)
            .limit_n_comparisons(std::stoi(argv[ProcessIArguments::NU_LIMIT]))
            .with_table(argv[ProcessIArguments::REFERENCE_TABLE])
            .using_epsilon_1(std::stod(argv[ProcessIArguments::EPSILON_1]))
            .using_epsilon_2(std::stod(argv[ProcessIArguments::EPSILON_2]))
            .using_epsilon_3(std::stod(argv[ProcessIArguments::EPSILON_3]))
            .using_epsilon_4(std::stod(argv[ProcessIArguments::EPSILON_4]))
            .identified_by(argv[ProcessIArguments::IDENTIFICATION_STRATEGY])
            .build();
}
std::shared_ptr<Identification> identifier_factory (char *argv[], const std::shared_ptr<Star::list> &b) {
    std::string identification_strategy(argv[ProcessIArguments::IDENTIFICATION_STRATEGY]);
    std::string upper_strategy = identification_strategy;
    std::transform(identification_strategy.begin(), identification_strategy.end(), upper_strategy.begin(), ::toupper);

    if (upper_strategy == "ANGLE") return create_generic_identifier<Angle>(argv, b);
    else if (upper_strategy == "DOT") return create_generic_identifier<Dot>(argv, b);
    else if (upper_strategy == "PLANE") return create_generic_identifier<Plane>(argv, b);
    else if (upper_strategy == "SPHERE") return create_generic_identifier<Sphere>(argv, b);
    else if (upper_strategy == "PYRAMID") return create_generic_identifier<Pyramid>(argv, b);
    else if (upper_strategy == "COMPOSITE") return create_generic_identifier<Composite>(argv, b);
    else throw std::runtime_error("'strategy' must be in space [ANGLE, DOT, PLANE, SPHERE, PYRAMID, COMPOSITE].");
}

void read_image (cv::Mat &image) {
    static cv::VideoCapture cap(0);

    if (!cap.isOpened()) { throw std::runtime_error("Unable to read from webcam. "); }
    else { cap >> image; }
}

void locate_stars (char *argv[], const std::shared_ptr<Star::list> &stars, const cv::Mat &image) {
    cv::Mat working;

    // Convert to a grayscale image.
    cv::cvtColor(image, working, cv::COLOR_BGR2GRAY);

    // Blur the image with a normalized box filter, gives imperfections lower weight.
    cv::blur(working, working, cv::Size(
        std::stoi(argv[ProcessIArguments::BKB_SZ]),
        std::stoi(argv[ProcessIArguments::BKB_SZ])
    ));

    // Convert to a binary image.
    cv::threshold(working, working, std::stoi(argv[ProcessIArguments::THRES]), 255, cv::THRESH_BINARY);

    // Find the contours in the image, after producing a binary image (step above).
    std::vector<std::vector<cv::Point>> c;
    std::vector<cv::Vec4i> h;
    cv::findContours(working, c, h, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // Find the moments in the image. Do not accept zeroed entries.
    std::vector<cv::Moments> mu;
    mu.resize(c.size());
    for (unsigned int i = 0; i < c.size(); i++) {
        mu[i] = cv::moments(c[i], false);
    }

    // Compute the centroids, given each moment.
    std::vector<cv::Point2f> mc;
    mc.resize(c.size());
    for (const auto &m : mu) {
        mc.emplace_back(cv::Point2d(m.m10 / m.m00, m.m01 / m.m00));
    }

    // Project each point onto a 3D sphere and save this to our star list.
    stars->reserve(mc.size());
    for (const cv::Point2f &p : mc) {
        // Translate our 2D OpenCV system to have (0, 0) at the center and normalize.
        double m_x = (p.x - std::stod(argv[ProcessIArguments::MAX_X]) / 2.) /
            ((1. / std::stod(argv[ProcessIArguments::DPP_X])) * 180. / M_PI);
        double m_y = (p.y - std::stod(argv[ProcessIArguments::MAX_Y]) / 2.) /
            ((1. / std::stod(argv[ProcessIArguments::DPP_X])) * 180. / M_PI);

        // Apply the inverse Mercator projection w/ an ellipsoid.
        double lat = (2.0 * atan(exp(m_y))) - M_PI / 2.0;
        double lon = m_x;

        stars->push_back(Star::wrap(Vector3::Normalized(Vector3(
                cos(lat) * cos(lon),
                cos(lat) * sin(lon),
                sin(lat)))
        ));
    }
}

int main (int, char *argv[]) {
    cxxtimer::Timer t(false);
    cv::Mat image;

    // We pass state to our stars to identify different images.
    std::shared_ptr<Star::list> stars = std::make_shared<Star::list>();
    std::shared_ptr<Identification> identifier = identifier_factory(argv, stars);

    for (int i = 0; i < std::stoi(argv[ProcessIArguments::SAMPLES]); i++) {
        static std::array<double, 3> captured_times;

        t.start();
        read_image(image);
        t.stop();
        captured_times[0] = t.count();
        t.reset();

        t.start();
        locate_stars(argv, stars, image);
        t.stop();
        captured_times[1] = t.count();
        t.reset();

        t.start();
        identifier->identify();
        t.stop();
        captured_times[2] = t.count();
        t.reset();

        std::cout << "Time to Image:    " << captured_times[0] << std::endl
                  << "Time to Process:  " << captured_times[1] << std::endl
                  << "Time to Identify: " << captured_times[2] << std::endl
                  << "Total Time:       " << std::accumulate(
                captured_times.begin(),
                captured_times.end(),
                0.0
        ) << std::endl;
    }
}