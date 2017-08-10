""""
This file is used to read the temporary files data/***.dat and plot the stars with the id
numbers as annotations. This is visualized as a translucent 3D sphere cap (representing the 
Celestial sphere) with the current star vector endpoints projected onto it. 

data/current_plot.dat is formatted as such:
fov
norm
focus.i focus.j focus.k
star_1.i star_1.j star_1.k star_1.bsc_id
.
.
.
star_n.i star_n.j star_n.k star_n.bsc_id

data/error_plot.dat is formatted as such:
affected_1.i affected_1.j affected_1.k affected_1.bsc_id affected_1.plot_color
.
.
.
affected_n.i affected_n.j affected_n.k affected_n.bsc_id affected_n.plot_color
"""""



import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np

# create 3D axis and figure
fig = plt.figure()
ax = fig.gca(projection='3d')

# data used to specify plot
focus, stars, errors = [], [], []
fov, norm = 0, 0

# parse the plot file
with open('../../../data/current_plot.dat') as current_plot:
    # first, record the fov and norm
    fov = float(current_plot.readline())
    norm = float(current_plot.readline())

    # second, record the focus vector
    for component in current_plot.readline().split():
        focus.append(float(component))

    # next, record all of the stars
    for line in current_plot:
        star = []
        for component in line.split():
            star.append(float(component))
        stars.append(star)

# parse the error file
with open('../../../data/error_plot.dat') as error_plot:
    for line in error_plot:
        error = []

        for a, component in enumerate(line.split()):
            if a < 4:
                error.append(float(component))
            else:
                error.append(component)
        errors.append([error])

# determine sphere cap boundaries using focus vector
delta = np.arccos(focus[2] / norm)
alpha = np.arctan2(focus[1], focus[0])
fov_limit = np.deg2rad(fov) / 2.0

# plot sphere cap
u = np.linspace(alpha - fov_limit, alpha + fov_limit, 100)
v = np.linspace(delta - fov_limit, delta + fov_limit, 100)
sphere_x = np.outer(np.cos(u), np.sin(v))
sphere_y = np.outer(np.sin(u), np.sin(v))
sphere_z = np.outer(np.ones(np.size(u)), np.cos(v))
ax.plot_surface(sphere_x, sphere_y, sphere_z, color='b', alpha='0.1')

# plot clean data set as black
for star in stars:
    ax.scatter(star[0], star[1], star[2], marker='*', color='k')
    ax.text(star[0], star[1], star[2], '{}'.format(int(star[3])))

# plot error models with specified colors
for model in errors:
    for error in model:
        ax.scatter(error[0], error[1], error[2], marker='*', color=error[4])
        ax.text(error[0], error[1], error[2], '{}'.format(int(error[3])))

# display the plot!
plt.show()
