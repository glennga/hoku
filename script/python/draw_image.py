""""
This file is used to read the temporary files /tmp/***.tmp and plot the stars with the id numbers as annotations. 
This is visualized as a translucent 3D sphere cap (representing the Celestial sphere) with the current star vector 
endpoints projected onto it. 

The following parameters are specified. ** Indicates that this parameter is required.
**fov**
fontsize
quiver
err

TEMP/cuplt.tmp is formatted as such:
focus.i focus.j focus.k
star_1.i star_1.j star_1.k star_1.label
.
.
.
star_n.i star_n.j star_n.k star_n.label

TEMP/tmp/errplt.dat is formatted as such:
affected_1.i affected_1.j affected_1.k affected_1.label affected_1.plot_color
.
.
.
affected_n.i affected_n.j affected_n.k affected_n.label affected_n.plot_color

Usage: python3 draw_image.py [arg1] [arg2] [arg3] ...
"""""

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import matplotlib
import tempfile
import numpy as np
import os
import sys

# Default font size. Modifiable through arguments 'fontsize' or 'f'.
font_size = 10

# Don't display the quiver by default. Toggleable through arguments 'quiver' or 'q'.
quiver_flag = False

# Display the error stars by default. Toggleable through arguments 'err' or 'e'.
err_flag = True

# Field of view of the image. Must be defined through arguments.
fov = 0


def parse_args(arg):
    """ Set the appropriate global variables given the argument. If an argument

    :param arg: Argument as a key-value pair separated by an '=' sign.
    :return: True if the argument passed was valid. False otherwise.
    """
    toggle_k = [['on', '1', 'enable'], ['off', '0', 'disable']]
    global font_size, quiver_flag, err_flag, fov

    try:
        k, v = arg.split('=')[0].strip(), arg.split('=')[1].strip()
    except IndexError:
        # Occurs when argument given w/o '=' sign.
        return False

    try:
        if k == 'fontsize' or k == 'f':
            font_size = int(v)
        elif k == 'fov':
            fov = float(v)

        elif k == 'quiver' or k == 'q':
            quiver_flag = True if v in toggle_k[0] else (False if v in toggle_k[1] else None)
            if quiver_flag is None:
                return False
        elif k == 'err' or k == 'e':
            err_flag = True if v in toggle_k[0] else (False if v in toggle_k[1] else None)
            if err_flag is None:
                return False

        else:
            return False

    except ValueError:
        # Occurs when passed parameter is not an integer or float.
        return False

    # Reach here if any flags were set or if the user requested help.
    return True


def parse_files():
    """ Parse the cuplt.tmp and errplt.tmp files. Return this as a dictionary with keys 'fov', 'focus', 'stars',
    'errors'.

    :return: Dictionary with keys: 'fov', 'focus', 'stars', 'errors'
    """
    focus, stars, errors = [], [], []

    # Parse the plot file.
    with open(tempfile.gettempdir() + '/cuplt.tmp') as current_plot:
        # Record the focus vector.
        for component in current_plot.readline().split():
            focus.append(float(component))

        # Record all of the stars.
        for line in current_plot:
            star = []
            for component in line.split():
                star.append(float(component))
            stars.append(star)

    # If desired, parse the error file.
    if err_flag:
        with open(tempfile.gettempdir() + '/errplt.tmp') as error_plot:
            for line in error_plot:
                error = []

                for a, component in enumerate(line.split()):
                    if a < 4:
                        error.append(float(component))
                    else:
                        error.append(component)
                errors.append([error])

    # Return all items as a dictionary.
    return {'fov': fov, 'focus': focus, 'stars': stars, 'errors': errors}


def draw_points(stars, errors, ax):
    """ Draw the given stars and error stars.

    :param stars: List of stars to draw with.
    :param errors: List of error stars to draw with.
    :param ax: Axis to plot to. Should be 3D axis.
    :return: None.
    """

    # Open the file of common star names to HIP numbers. Store this in names dictionary.
    names = {}
    with open(os.environ['HOKU_PROJECT_PATH'] + '/data/star-names.dat') as names_f:
        names_f.readline()

        for line in names_f:
            n, h = line.split(',')[0].strip(), line.split(',')[1].strip()
            names.update({h: n})

    # Plot clean data set as black.
    for star in stars:
        if quiver_flag:
            ax.quiver(0, 0, 0, star[0], star[1], star[2], arrow_length_ratio=0.0000001, alpha=0.2)
        ax.scatter(star[0], star[1], star[2], marker='*', color='k', s=100)

        if str(int(star[3])) in names:
            ax.text(star[0], star[1], star[2], names[str(int(star[3]))])
        else:
            ax.text(star[0], star[1], star[2], 'HIP{}'.format(int(star[3])))

    # Plot error models with specified colors.
    for model in errors:
        for error in model:
            if quiver_flag:
                ax.quiver(0, 0, 0, error[0], error[1], error[2], arrow_length_ratio=0.0000001, alpha=0.2)
            ax.scatter(error[0], error[1], error[2], marker='*', color=error[4])
            ax.text(error[0], error[1], error[2], 'ERR{}'.format(int(error[3])))


def plot():
    """ Plot the image. Each star is annotated with it's common star name (if available), or it's Hipparcos number.
    The image itself is visualized as a translucent 3D sphere cap (representing a unit, or Celestial sphere) with the
    vector endpoints projected on it from the observer (i.e. Earth).

    :return: Nothing.
    """
    fig = plt.figure()
    ax = fig.gca(projection='3d')

    # Parse the data to plot from the temp files.
    p = parse_files()

    # Determine the sphere cap boundaries using focus vector.
    delta = np.arccos(p['focus'][2])
    alpha = np.arctan2(p['focus'][1], p['focus'][0])
    fov_limit = 0.5

    # Plot sphere cap.
    u = np.linspace(alpha - fov_limit, alpha + fov_limit, 100)
    v = np.linspace(delta - fov_limit, delta + fov_limit, 100)
    sphere_x = np.outer(np.cos(u), np.sin(v))
    sphere_y = np.outer(np.sin(u), np.sin(v))
    sphere_z = np.outer(np.ones(np.size(u)), np.cos(v))
    # ax.plot_surface(sphere_x, sphere_y, sphere_z, color='b', alpha='0.1')
    ax.plot_wireframe(sphere_x, sphere_y, sphere_z, alpha='0.1')

    # Draw origin (if desired).
    if quiver_flag:
        ax.scatter(0, 0, 0, s=100)

    # Draw the points.
    draw_points(p['stars'], p['errors'], ax)
    ax.set_axis_off()


if __name__ == '__main__':
    # Ensure that there exists no more than five arguments.
    if len(sys.argv) > 5:
        print('Usage: python3 draw_image.py [arg1] [arg2] [arg3] ...')
        exit(2)

    # If the user needs help, display the appropriate message.
    if sys.argv[1].strip() == 'help':
        print('Options: fontsize [1, ..., N], quiver [on, 1, enable], err [on, 1, enable]')
        exit(1)

    # Parse each parameter.
    arg_returns = list(map(lambda a: parse_args(sys.argv[a]), range(1, len(sys.argv))))
    if not all(arg_returns):
        print('Arguments not parsed. Enter \'python3 draw_image.py help\' for available options.')
        exit(2)

    # Set the font size.
    matplotlib.rcParams.update({'font.size': font_size})

    # Create the plot figure, and display the plot.
    plot()
    plt.show()
