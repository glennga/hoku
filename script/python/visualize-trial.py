""""
This file is used to produce plots to show the relationship between various parameters in the Query, Alignment, 
and Crown trials. We assume the Query, Alignment, and Crown trials use the following attributes:

Query:     IdentificationMethod,QuerySigma,ShiftSigma,CandidateSetSize,SExistence
Alignment: IdentificationMethod,MatchSigma,ShiftSigma,MBar,OptimalConfigRotation,
           NonOptimalConfigRotation,OptimalComponentError,NonOptimalComponentError
Crown:     IdentificationMethod,MatchSigma,QuerySigma,ShiftSigma,CameraSensitivity,FalseStars,
           ComparisonCount,BenchmarkSetSize,ResultSetSize,PercentageCorrectInCleanResultSet

The first argument is the location of the log file. We can infer the type of trial from the length of the header.

Usage: visualize-trial [angle-log] [sphere-log] [plane-log] [pyramid-log] [coin-log]
"""""

import matplotlib.pyplot as plt
import numpy as np
import os, csv, sys

# Points per variation, defined in each trial's header file (query.h, alignment.h, and crown.h).
POINTS_PER_VARIATION = 100

# Y axis limits for each plot, wrapped in an iterator.
qu_yll = iter([[0, 1], [0, 100], [0, 1]])
al_yll = iter([[0, 5.0e-015], [0, 1.0e-11]])
cr_yll = iter([[0, 1], [0, 100], [0, 1], [0, 100]])

# X axis tick labels, wrapped in an iterator.
qu_xtl = iter([[r'$\epsilon \times 3^{0}$'] + [r'$\epsilon \times 3^{4}$'] + [r'$\epsilon \times 3^{9}$'] +
               [r'$\epsilon \times 3^{14}$'] + [r'$\epsilon \times 3^{19}$']] +
              [[r'$\epsilon \times 3^{15}$'] + [r'$\epsilon \times 3^{16}$'] + [r'$\epsilon \times 3^{17}$'] +
               [r'$\epsilon \times 3^{18}$'] + [r'$\epsilon \times 3^{19}$']] +
              [[r'$0^{\circ}$'] + [r'$0.001^{\circ}$'] + [r'$0.002^{\circ}$'] + [r'$0.003^{\circ}$'] +
               [r'$0.004^{\circ}$']])
al_xtl = iter([['5.5', '6.0', '6.5', '7.0', '7.5']] +
              [[r'$\epsilon \times 3^{0}$', r'$\epsilon \times 3^{3}$', r'$\epsilon \times 3^{5}$',
                r'$\epsilon \times 3^{7}$', r'$\epsilon \times 3^{9}$']])
cr_xtl = iter([['0', '0.1', '0.2', '0.3', '0.4']] + [['0', '0.1', '0.2', '0.3', '0.4']] +
              [[r'$0^{\circ}$'] + [r'$0.001^{\circ}$'] + [r'$0.002^{\circ}$'] + [r'$0.003^{\circ}$'] +
               [r'$0.004^{\circ}$']] +
              [[r'$0^{\circ}$'] + [r'$0.001^{\circ}$'] + [r'$0.002^{\circ}$'] + [r'$0.003^{\circ}$'] +
               [r'$0.004^{\circ}$']])

# Titles for each plot, wrapped in an iterator.
qu_tl = iter([r'$Query \ \sigma$ vs. $P(Correct \ Star \ Set \ in \ Candidate \ Set), '
              r'Noise = \epsilon \times 3^{9}$',
              r'$Query \ \sigma$ vs. $|Candidate \ Set \ Outliers|, Noise = \epsilon \times 3^{9}$',
              'Centroiding Error (i.e. Noise) vs. P(Correct Star Set in Candidates)'])
al_tl = iter([r'$Camera \ Sensitivity \ (m)$ vs. $|| Catalog \ Vector - Estimated \ Vector ||$',
              r'$Shift \ \sigma$ (Noise) vs. $|| Catalog \ Vector - Estimated \ Vector ||$'])
cr_tl = iter([r'$Percentage \ of \ False \ Stars$ vs. $|Correct \ Stars| / |Total \ Number \ of \ True \ Stars|$',
              r'$Percentage \ of \ False \ Stars$ vs. $Number \ of \ Star \ Sets \ Exhausted$',
              'Centroiding Error vs. Percentage of Correct Identification',
              'Centroiding Error vs. Number of Times we Pick Query Stars'])

# Legends for each plot, wrapped in an iterator.
qu_ll = iter([['Angle', 'Spherical Triangle', 'Planar Triangle', 'Pyramid', 'CoIn'] for _ in range(3)])
al_ll = iter([['Angle', 'Spherical Triangle', 'Planar Triangle', 'Pyramid', 'CoIn'] for _ in range(2)])
cr_ll = iter([['Angle', 'Spherical Triangle', 'Planar Triangle', 'Pyramid', 'CoIn'] for _ in range(4)])

# X axis label (not ticks) for each plot, wrapped in an iterator.
qu_xal = iter([r'$Query \ \sigma$', r'$Query \ \sigma$', 'Centroiding Error (Degrees)'])
al_xal = iter([r'$Camera \ Sensitivity \ (m)$', r'$Shift \ \sigma$'])
cr_xal = iter([r'$Percentage \ of \ False \ Stars$', r'$Percentage \ of \ False \ Stars$',
               'Centroiding Error (Degrees)', 'Centroiding Error (Degrees)'])

# Y axis label (not ticks) for each plot, wrapped in an iterator.
qu_yal = iter([r'$P(Correct \ Star \ Set \ in \ Candidates)$', r'$|Candidate \ Set \ Outliers|$',
               'P(Correct Star Set in Candidates)'])
al_yal = iter([r'$|| Catalog \ Vector - Estimated \ Vector ||$' for _ in range(2)])
cr_yal = iter([r'$|Correct \ Stars| / |Total \ Number \ of \ True \ Stars|$',
               r'$Number \ of \ Star \ Sets \ Exhausted$',
               'Percentage of Correct Identification', 'Times we Pick Query Stars'])


def bar_plot(log, k, x_index, y_index, restrict_d=None, restrict_y=lambda h: h, y_divisor=None):
    """ Generic bar plot function, given the indices of the X and Y data (with respect the the log), as well as a
    restriction function.

    :param log: List of lists representing the contents of a single log file.
    :param k: Current log file iteration (in space [0, 1, 2, 3]).
    :param x_index: Index of the CSV column that represents dimension X of the data point.
    :param y_index: Index of the CSV column that represents dimension Y of the data point.
    :param restrict_d: A restriction function to apply to the data-set we operate on.
    :param restrict_y: A restriction function to apply to every list of Y points. Defaults to using data as-is.
    :param y_divisor: If specified, we divide every Y point with the point at this index.
    :return: None.
    """

    # Apply a restriction to our data-set if given. Determine our X axis.
    d = [tuple for tuple in log if (True if restrict_d is None else restrict_d(tuple))]
    sorted(d, key=lambda x: x[x_index])
    x_count = list(set([float(x[x_index]) for x in d]))
    y_list = [[] for _ in x_count]

    # Determine our Y axis. Dependent on if y_divisor is specified or not.
    if y_divisor is None:
        for i in range(0, len(x_count)):
            for j in range(0, POINTS_PER_VARIATION):
                y_list[i].append(float(d[i * POINTS_PER_VARIATION + j][y_index]))
    else:
        for i in range(0, len(x_count)):
            for j in range(0, POINTS_PER_VARIATION):
                y_list.append(float(d[i * POINTS_PER_VARIATION + j][y_index] /
                                    d[i * POINTS_PER_VARIATION + j][y_divisor]))

    # Plot the bar chart of our averages, as well as the corresponding error bars.
    plt.bar(np.arange(len(x_count)) + 0.15 * k - 0.3, [np.average(restrict_y(y)) for y in y_list], 0.15,
            yerr=[np.std(restrict_y(y)) for y in y_list])
    # plt.bar(np.arange(len(x_count)) + 0.15 * k - 0.3, [np.average(restrict_y(y)) for y in y_list], 0.15)


def plot_add_info(yll, xtl, tl, ll, xal, yal):
    """ Adds to the global plot, the given figure characteristics.

    :param yll: Y axis limit iterator.
    :param xtl: X tick labels iterator.
    :param tl: Title iterator.
    :param ll: Legend iterator.
    :param xal: X axis label iterator.
    :param yal: Y axis label iterator.
    :return: None.
    """

    # Set Y limits.
    ax = plt.gca()
    ax.set_ylim(next(yll))

    # Add the bar chart X axis tick labels.
    x_labels = next(xtl)
    plt.xticks(np.arange(len(x_labels)), x_labels)

    # Add the plot title, and the legend.
    plt.title(next(tl)), plt.legend(next(ll))

    # Add the chart X and Y labels.
    plt.xlabel(next(xal)), plt.ylabel(next(yal))


def query_trial_plot(log_sets):
    """ For every log in log_sets (which hold sets of tuples), display a plot comparing each.

    The following plots are displayed: QuerySigma vs. P(SExistence)
                                       QuerySigma vs. CandidateSetSize
                                       ShiftSigma vs. P(SExistence) w/ Optimal QuerySigma (Before Saturation)

    :param log_sets: List of list of lists representing the contents of the several log files
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=30)
    sigma_set_1 = ['2.22045e-16', '1.79856e-14', '4.3705e-12', '1.06203e-09', '2.58074e-07']
    sigma_set_2 = ['3.1861e-09', '9.55829e-09', '2.86749e-08', '8.60246e-08', '2.58074e-07']

    # Plot #1: Query Sigma vs. P(SExistence) with noise.
    plt.figure()
    plt.subplot(121)
    [bar_plot(log, k, 1, 4, lambda g: g[2] == '0.002' and g[1] in sigma_set_1) for k, log in enumerate(log_sets)]
    plot_add_info(qu_yll, qu_xtl, qu_tl, qu_ll, qu_xal, qu_yal)

    # Plot #2: Query Sigma vs. CandidateSetSize- with noise.
    plt.subplot(122)
    [bar_plot(log, k, 1, 3, lambda g: g[2] == '0.002' and g[1] in sigma_set_2,
              lambda h: [a for a in h if a > 1]) for k, log in enumerate(log_sets)]
    plot_add_info(qu_yll, qu_xtl, qu_tl, qu_ll, qu_xal, qu_yal)

    # Plot #3: ShiftSigma vs. P(SExistence). We restrict each data set to the biggest query sigma.
    plt.figure()
    bar_plot(log_sets[0], 0, 2, 4, lambda g: g[1] == '2.58074e-07')
    bar_plot(log_sets[1], 1, 2, 4, lambda g: g[1] == '2.58074e-07')
    bar_plot(log_sets[2], 2, 2, 4, lambda g: g[1] == '2.58074e-07')
    bar_plot(log_sets[3], 3, 2, 4, lambda g: g[1] == '2.58074e-07')
    bar_plot(log_sets[4], 4, 2, 4, lambda g: g[1] == '2.58074e-07')
    plot_add_info(qu_yll, qu_xtl, qu_tl, qu_ll, qu_xal, qu_yal)

    plt.show()


def alignment_trial_plot(log_sets):
    """ For every log in log_sets (which hold sets of tuples), display a plot comparing each.

    The following plots are displayed: Camera Sensitivity vs. ||Original Star - Estimated Star||
                                       ShiftSigma vs. ||Original Star - Estimate Star|| w/ Middle MatchSigma

    :param log_sets: List of list of lists representing the contents of the several log files
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=12)
    sigma_set = ['0', '0.001', '0.002', '0.003', '0.004']
    middle_sigma = sigma_set[2]

    # Plot #1: Camera Sensitivity vs. ||Original Star - Estimated Star||. We are not testing for noise in this case.
    plt.figure()
    [bar_plot(log, k, 3, 6, lambda g: g[2] == '0' and g[1] == middle_sigma) for k, log in enumerate(log_sets)]
    plot_add_info(al_yll, al_xtl, al_tl, al_ll, al_xal, al_yal)

    # Plot #2: Shift Sigma vs. ||Original Star - Estimated Star||. Using MBar = 6.0 & middle MatchSigma for each.
    plt.figure()
    [bar_plot(log, k, 2, 6, lambda g: g[3] == '6' and g[1] == middle_sigma and g[2] in sigma_set)
     for k, log in enumerate(log_sets)]
    plot_add_info(al_yll, al_xtl, al_tl, al_ll, al_xal, al_yal)

    # Plot 2 requires a log axis here.
    ax = plt.gca()
    ax.set_yscale('log')

    plt.show()


def crown_trial_plot(log_sets):
    """ For every log in log_sets (which hold sets of tuples), display a plot comparing each.

    The following plots are displayed: False Percentage vs. |Correct Stars| / |Total Number of True Stars|
                                       False Percentage vs. Number of Star Sets Exhausted
                                       Shift Sigma vs. |Correct Stars| / |Total Number of True Stars|
                                       Shift Sigma vs. Number of Star Sets Exhausted

    :param log_sets: List of list of lists representing the contents of the several log files
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=30)
    sigma_set = ['0', '0.001', '0.002', '0.003', '0.004']

    # Plot #1: False Percentage vs. |Correct Stars| / |Total Number of True Stars|
    plt.figure(), plt.subplot(121)
    [bar_plot(log, k, 5, 9, lambda g: g[3] == sigma_set[3] and g[4] == '6.5') for k, log in enumerate(log_sets)]
    plot_add_info(cr_yll, cr_xtl, cr_tl, cr_ll, cr_xal, cr_yal)

    # Plot #2: False Percentage vs. Number of Star Sets Exhausted
    plt.subplot(122)
    [bar_plot(log, k, 5, 6, lambda g: g[3] == sigma_set[3] and g[4] == '6.5') for k, log in enumerate(log_sets)]
    plot_add_info(cr_yll, cr_xtl, cr_tl, cr_ll, cr_xal, cr_yal)

    # Plot 3: Shift Sigma vs. |Correct Stars| / |Total Number of True Stars|
    plt.figure()
    [bar_plot(log, k, 3, 9, lambda g: g[5] == '0' and g[4] == '6' and g[3] in sigma_set)
     for k, log in enumerate(log_sets)]
    plot_add_info(cr_yll, cr_xtl, cr_tl, cr_ll, cr_xal, cr_yal)

    # Plot 4: Shift Sigma vs. Number of Star Sets Exhausted
    plt.figure()
    [bar_plot(log, k, 3, 6, lambda g: g[5] == '0' and g[4] == '6' and g[3] in sigma_set)
     for k, log in enumerate(log_sets)]
    plot_add_info(cr_yll, cr_xtl, cr_tl, cr_ll, cr_xal, cr_yal)

    plt.show()


def visualize_trial(angle_log, sphere_log, plane_log, pyramid_log, coin_log):
    """ Source function, used to display a plot of the given exactly four log files.

    :param angle_log Location of the angle log file to use.
    :param sphere_log Location of the sphere log file to use.
    :param plane_log Location of the plane log file to use.
    :param pyramid_log Location of the pyramid log file to use.
    :param coin_log Location of the coin log file to use.
    :return: None.
    """
    with open(angle_log, 'r') as f_1, open(sphere_log, 'r') as f_2, open(plane_log, 'r') as f_3, \
            open(pyramid_log, 'r') as f_4, open(coin_log, 'r') as f_5:
        csv_1, csv_2, csv_3, csv_4, csv_5 = list(map(lambda f: csv.reader(f, delimiter=','), [f_1, f_2, f_3, f_4, f_5]))

        # Parse our header, and the rest of the logs.
        attributes = list(map(lambda c: next(c), [csv_1, csv_2, csv_3, csv_4, csv_5]))
        logs = list(map(lambda c: np.array(np.array([tuple for tuple in c])), [csv_1, csv_2, csv_3, csv_4, csv_5]))
        # logs = list(map(lambda c: np.array(np.array([tuple for tuple in c])), [csv_1, csv_2, csv_3]))

        # Based on the header, determine what plots to produce.
        if all(map(lambda a: len(a) == 5, attributes)):
            query_trial_plot(logs)
        elif all(map(lambda a: len(a) == 8, attributes)):
            alignment_trial_plot(logs)
        elif all(map(lambda a: len(a) == 10, attributes)):
            crown_trial_plot(logs)
        else:
            raise ValueError('Not a log file or corrupt header.')


# Perform the trials!
if len(sys.argv) is not 6:
    print('Usage: visualize-trial [angle-log] [sphere-log] [plane-log] [pyramid-log] [coin-log]')
else:
    visualize_trial(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
