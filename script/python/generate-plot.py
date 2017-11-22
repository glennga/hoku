""""
This file is used to read the temporary files data/***.tmp and plot the stars with the id numbers as annotations. 
This is visualized as a translucent 3D sphere cap (representing the Celestial sphere) with the current star vector 
endpoints projected onto it. 

data/logs/tmp/cuplt.tmp is formatted as such:
fov
norm
focus.i focus.j focus.k
star_1.i star_1.j star_1.k star_1.bsc_id
.
.
.
star_n.i star_n.j star_n.k star_n.bsc_id

data/logs/tmp/errplt.dat is formatted as such:
affected_1.i affected_1.j affected_1.k affected_1.bsc_id affected_1.plot_color
.
.
.
affected_n.i affected_n.j affected_n.k affected_n.bsc_id affected_n.plot_color
"""""



import matplotlib.pyplot as plt
import matplotlib
from mpl_toolkits.mplot3d import Axes3D
import matplotlib
import numpy as np
import os

matplotlib.rcParams.update({'font.size': 40})

# Check as true to enable lines to origin.
SHOW_QUIVER = True

# Create 3D axis and figure.
fig = plt.figure()
ax = fig.gca(projection='3d')

# Specify data used to plot.
focus, stars, errors = [], [], []
fov, norm = 0, 0

# Parse the plot file.
with open(os.environ['HOKU_PROJECT_PATH'] + '/data/logs/tmp/cuplt.tmp') as current_plot:
    # First, record the fov and norm.
    fov = float(current_plot.readline())
    norm = float(current_plot.readline())

    # Second, record the focus vector.
    for component in current_plot.readline().split():
        focus.append(float(component))

    # Next, record all of the stars.
    for line in current_plot:
        star = []
        for component in line.split():
            star.append(float(component))
        stars.append(star)

# Parse the error file.
with open(os.environ['HOKU_PROJECT_PATH'] + '/data/logs/tmp/errplt.tmp') as error_plot:
    for line in error_plot:
        error = []

        for a, component in enumerate(line.split()):
            if a < 4:
                error.append(float(component))
            else:
                error.append(component)
        errors.append([error])

# Determine the sphere cap boundaries using focus vector.
delta = np.arccos(focus[2] / norm)
alpha = np.arctan2(focus[1], focus[0])
fov_limit = np.deg2rad(fov) / 2.0

# Plot sphere cap.
u = np.linspace(alpha - fov_limit, alpha + fov_limit, 100)
v = np.linspace(delta - fov_limit, delta + fov_limit, 100)
sphere_x = np.outer(np.cos(u), np.sin(v))
sphere_y = np.outer(np.sin(u), np.sin(v))
sphere_z = np.outer(np.ones(np.size(u)), np.cos(v))
# ax.plot_surface(sphere_x, sphere_y, sphere_z, color='b', alpha='0.1')
ax.plot_wireframe(sphere_x, sphere_y, sphere_z, alpha='0.1' )

# Draw origin (if desired).
if SHOW_QUIVER:
    ax.scatter(0, 0, 0, s=100)
    # ax.text(0, 0, 0, 'Observer')

# Plot clean data set as black.
for star in stars:
    if SHOW_QUIVER:
        ax.quiver(0, 0, 0, star[0], star[1], star[2], arrow_length_ratio=0.0000001, alpha=0.2)
    ax.scatter(star[0], star[1], star[2], marker='*', color='k', s=100)
    # ax.text(star[0], star[1], star[2], '{}'.format(int(star[3])))

# Plot error models with specified colors.'
for model in errors:
    for error in model:
        if SHOW_QUIVER:
            ax.quiver(0, 0, 0, error[0], error[1], error[2], arrow_length_ratio=0.0000001, alpha=0.2)
        ax.scatter(error[0], error[1], error[2], marker='*', color=error[4])
        # ax.text(error[0], error[1], error[2], '{}'.format(int(error[3])))

# Display the plot!
ax.set_axis_off()
plt.show()