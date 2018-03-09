""""
This file is used to read a FITS file, determine the centroids of the brightest stars, and write these points to the 
console. The only parameter required by this script is the path of the FITS image to parse.

Usage: python3 find_centroids.py [full path to FITS file]
"""""

from astropy.io import fits
import configparser
import numpy as np
import cv2
import csv
import os
import sys


def locate_stars(img_cv, cf):
    """ TODO: Finish the documentation here.

    :param img_cv:
    :param cf:
    :return:
    """
    # Blur the image with a normalized box filter, gives imperfections lower weight.
    img_cv = cv2.blur(img_cv, (int(cf['centroid-find']['bkb-sz']), int(cf['centroid-find']['bkb-sz'])))

    # Find the edges in the image. Uses Canny Edge Detection.
    img_edges = cv2.Canny(img_cv, int(cf['centroid-find']['min-ced']), int(cf['centroid-find']['max-ced']))

    # Find the contours in the image, after producing a binary image (step above).
    img_contours, contours, hierarchy = cv2.findContours(img_edges, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    # Find the moments in the image. Do not accept zeroed entries.
    img_moments = list(map(lambda m: cv2.moments(m), contours))
    img_moments = [m for m in img_moments if m['m00'] + m['m01'] + m['m10'] != 0]

    # Compute the centroids, given each moment. Do not accept NaN or out-of-bounds entries.
    img_centroids = list(map(lambda m: [m['m10'] / m['m00'], m['m01'] / m['m00']], img_moments))
    img_centroids = list(filter(lambda c: not np.isnan(c[0]) and not np.isnan(c[1]), img_centroids))
    img_centroids = list(filter(lambda c: 0 < c[1] < img_cv.shape[0] and 0 < c[0] < img_cv.shape[1], img_centroids))

    # Sort by brightness. Return iff the pixel intensity is below the given cutoff.
    cutoff = int(cf['centroid-find']['min-cut'])
    img_centroids.sort(key=lambda c: img_cv[int(c[1]), int(c[0])])
    return list(filter(lambda c: img_cv[int(c[1]), int(c[0])] > cutoff, img_centroids))


if __name__ == '__main__':
    # Ensure that there exists only one argument to the program.
    if len(sys.argv) != 2:
        print('Usage: python3 find_centroids.py [full path to FITS file]'), exit(1)

    # Attempt to open the configuration file.
    cf = configparser.ConfigParser(inline_comment_prefixes=';')
    cf.read(os.environ['HOKU_PROJECT_PATH'] + '/CONFIG.ini')
    if not cf:
        print('Could not open CONFIG.ini.'), exit(3)

    # Determine if the path given to us is relative or absolute.
    try:
        is_absolute = not os.path.isfile(os.environ['HOKU_PROJECT_PATH'] + '/' + sys.argv[1])
        hdul = fits.open(sys.argv[1] if is_absolute else os.environ['HOKU_PROJECT_PATH'] + '/' + sys.argv[1])
    except FileNotFoundError:
        print('Could not open the given file.'), exit(2)

    # Load the image into memory, cast to Mat (for OpenCV).
    img_data = hdul[0].data
    img_cv = cv2.resize(img_data, (img_data.shape[1], img_data.shape[0]))
    hdul.close()

    # Locate the stars in the image. Prepend the image center to the list.
    img_stars = locate_stars(img_cv, cf)
    img_stars = [[img_data.shape[1] / 2, img_data.shape[0] / 2]] + img_stars

    # Output our results to console.
    list(map(lambda a: print('{},{}'.format(a[0], a[1])), img_stars))