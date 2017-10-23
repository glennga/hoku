""""
This file is used to produce plots to show the relationship between various parameters in the Query, Alignment, 
and Crown trials. We assume the Query, Alignment, and Crown trials use the following attributes:

Query:     IdentificationMethod,QuerySigma,ShiftSigma,CandidateSetSize,SExistence
Alignment: IdentificationMethod,MatchSigma,ShiftSigma,MBar,OptimalConfigRotation,
           NonOptimalConfigRotation,OptimalComponentError,NonOptimalComponentError
Crown:     IdentificationMethod,MatchSigma,QuerySigma,ShiftSigma,MBar,FalsePercentage,
           ComparisonCount,BenchmarkSetSize,ResultSetSize,NumberCorrectInResultSet

The first argument is the location of the log file. We can infer the type of trial from the length of the header.

Usage: visualize-trial [angle-log] [sphere-log] [plane-log] [pyramid-log] 
"""""

import matplotlib.pyplot as plt
import numpy as np
import os, csv, sys

# Points per variation, defined in each trial's header file (query.h, alignment.h, and crown.h).
POINTS_PER_VARIATION = 1000

# Y axis limits for each plot, wrapped in an iterator.
qu_yll = iter([[0, 1], [0, 100], [0, 1]])
al_yll = iter([[0, 3.0e-015], [0, 1.0e-11]])

# X axis tick labels, wrapped in an iterator.
qu_xtl = iter([[r'$\epsilon \times 3^{0}$'] + [r'$\epsilon \times 3^{4}$'] + [r'$\epsilon \times 3^{9}$'] +
               [r'$\epsilon \times 3^{14}$'] + [r'$\epsilon \times 3^{19}$']] +
              [[r'$\epsilon \times 3^{15}$'] + [r'$\epsilon \times 3^{16}$'] + [r'$\epsilon \times 3^{17}$'] +
               [r'$\epsilon \times 3^{18}$'] + [r'$\epsilon \times 3^{19}$']] +
              [[r'$\epsilon \times 3^{0}$'] + [r'$\epsilon \times 3^{3}$'] + [r'$\epsilon \times 3^{5}$'] +
               [r'$\epsilon \times 3^{7}$'] + [r'$\epsilon \times 3^{9}$']])
al_xtl = iter([['5.5', '6.0', '6.5', '7.0', '7.5']] +
              [[r'$\epsilon \times 3^{0}$', r'$\epsilon \times 3^{3}$', r'$\epsilon \times 3^{5}$',
                r'$\epsilon \times 3^{7}$', r'$\epsilon \times 3^{9}$']])

# Titles for each plot, wrapped in an iterator.
qu_tl = iter([r'$Query \ \sigma$ vs. $P(Correct \ Star \ Set \ in \ Candidate \ Set), '
              r'Noise = \epsilon \times 3^{9}$',
              r'$Query \ \sigma$ vs. $|Candidate \ Set \ Outliers|, Noise = \epsilon \times 3^{9}$',
              r'$Shift \ \sigma$ (Noise) vs. $P(Correct \ Star \ Set \ in \ Candidate \ Set)$'])
al_tl = iter([r'$Camera \ Sensitivity \ (m)$ vs. $|| Catalog \ Vector - Estimated \ Vector ||$',
              r'$Shift \ \sigma$ (Noise) vs. $|| Catalog \ Vector - Estimated \ Vector ||$'])

# Legends for each plot, wrapped in an iterator.
qu_ll = iter([['Angle', 'Spherical Triangle', 'Planar Triangle', 'Pyramid'] for _ in range(3)])
al_ll = iter([['Angle', 'Spherical Triangle', 'Planar Triangle', 'Pyramid'] for _ in range(2)])

# X axis label (not ticks) for each plot, wrapped in an iterator.
qu_xal = iter([r'$Query \ \sigma$', r'$Query \ \sigma$', r'$Shift \ \sigma$ (Noise)'])
al_xal = iter([r'$Camera \ Sensitivity \ (m)$', r'$Shift \ \sigma$'])

# Y axis label (not ticks) for each plot, wrapped in an iterator.
qu_yal = iter([r'$P(Correct \ Star \ Set \ in \ Candidate \ Set)$', r'$|Candidate \ Set \ Outliers|$',
               r'$P(Correct \ Star \ Set \ in \ Candidate \ Set)$'])
al_yal = iter([r'$|| Catalog \ Vector - Estimated \ Vector ||$' for _ in range(2)])


def bar_plot(log, k, x_index, y_index, restriction=None, limit_y=lambda h: h):
    """ Generic bar plot function, given the indices of the X and Y data (with respect the the log), as well as a
    restriction function.

    :param log: List of lists representing the contents of a single log file.
    :param k: Current log file iteration (in space [0, 1, 2, 3]).
    :param x_index: Index of the CSV column that represents dimension X of the data point.
    :param y_index: Index of the CSV column that represents dimension Y of the data point.
    :param restriction: Restriction function to apply to the data-set we operate on.
    :param limit_y: A restriction function to apply to every list of Y points. Defaults to using data as-is.
    :return: None.
    """

    # Apply a restriction to our dataset if given. Determine our X axis.
    d = [tuple for tuple in log if (True if restriction is None else restriction(tuple))]
    sorted(d, key=lambda x: x[x_index])
    x_count = list(set([float(x[x_index]) for x in d]))
    y_list = [[] for _ in x_count]

    # Determine our Y axis.
    for i in range(0, len(x_count)):
        for j in range(0, POINTS_PER_VARIATION):
            y_list[i].append(float(d[i * POINTS_PER_VARIATION + j][y_index]))

    # Plot the bar chart of our averages, as well as the corresponding error bars.
    plt.bar(np.arange(len(x_count)) + 0.2 * k - 0.3, [np.average(limit_y(y)) for y in y_list], 0.2,
            yerr=[np.std(limit_y(y)) for y in y_list])


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
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=12)
    sigma_set_1 = ['2.22045e-16', '1.79856e-14', '4.3705e-12', '1.06203e-09', '2.58074e-07']
    sigma_set_2 = ['3.1861e-09', '9.55829e-09', '2.86749e-08', '8.60246e-08', '2.58074e-07']
    sigma_set_3 = ['2.22045e-16', '5.9952e-15', '5.39568e-14', '4.85612e-13', '4.3705e-12']

    # Plot #1: Query Sigma vs. P(SExistence) with a small amount of noise.
    plt.figure()
    plt.subplot(121)
    [bar_plot(log, k, 1, 4, lambda g: g[2] == '4.3705e-12' and g[1] in sigma_set_1) for k, log in enumerate(log_sets)]
    plot_add_info(qu_yll, qu_xtl, qu_tl, qu_ll, qu_xal, qu_yal)

    # Plot #2: Query Sigma vs. CandidateSetSize- with a small amount of noise.
    plt.subplot(122)
    [bar_plot(log, k, 1, 3, lambda g: g[2] == '4.3705e-12' and g[1] in sigma_set_2,
              lambda h: [a for a in h if a > 1]) for k, log in enumerate(log_sets)]
    plot_add_info(qu_yll, qu_xtl, qu_tl, qu_ll, qu_xal, qu_yal)

    # Plot #3: ShiftSigma vs. P(SExistence). We restrict each data set to it's specific optimal query sigma.
    plt.figure()
    bar_plot(log_sets[0], 0, 2, 4, lambda g: g[1] == '5.39568e-14' and g[2] in sigma_set_3)
    bar_plot(log_sets[1], 1, 2, 4, lambda g: g[1] == '1.61871e-13' and g[2] in sigma_set_3)
    bar_plot(log_sets[2], 2, 2, 4, lambda g: g[1] == '5.9952e-15' and g[2] in sigma_set_3)
    bar_plot(log_sets[3], 3, 2, 4, lambda g: g[1] == '5.39568e-14' and g[2] in sigma_set_3)
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
    sigma_set = ['2.22045e-16', '5.9952e-15', '5.39568e-14', '4.85612e-13', '4.3705e-12']
    middle_match_sigma = '4.3705e-12'

    # Plot #1: Camera Sensitivity vs. ||Original Star - Estimated Star||. We are not testing for noise in this case.
    plt.figure()
    [bar_plot(log, k, 3, 6, lambda g: g[2] == '0' and g[1] == middle_match_sigma) for k, log in enumerate(log_sets)]
    plot_add_info(al_yll, al_xtl, al_tl, al_ll, al_xal, al_yal)

    # Plot #2: Shift Sigma vs. ||Original Star - Estimated Star||. Using MBar = 6.0 & middle MatchSigma for each.
    plt.figure()
    [bar_plot(log, k, 2, 6, lambda g: g[3] == '6' and g[1] == middle_match_sigma and g[2] in sigma_set)
     for k, log in enumerate(log_sets)]
    plot_add_info(al_yll, al_xtl, al_tl, al_ll, al_xal, al_yal)

    # Plot 2 requires a log axis here.
    ax = plt.gca()
    ax.set_yscale('log')

    plt.show()


def visualize_trial(log_1, log_2, log_3, log_4):
    """ Source function, used to display a plot of the given exactly four log files.

    :param log_1 Location of the first log file to use.
    :param log_2 Location of the second log file to use.
    :param log_3 Location of the third log file to use.
    :param log_4 Location of the fourth log file to use.
    :return: None.
    """
    with open(log_1, 'r') as f_1, open(log_2, 'r') as f_2, open(log_3, 'r') as f_3, open(log_4, 'r') as f_4:
        csv_1, csv_2, csv_3, csv_4 = list(map(lambda f: csv.reader(f, delimiter=','), [f_1, f_2, f_3, f_4]))

        # Parse our header, and the rest of the logs.
        attributes = list(map(lambda c: next(c), [csv_1, csv_2, csv_3, csv_4]))
        logs = list(map(lambda c: np.array(np.array([tuple for tuple in c])), [csv_1, csv_2, csv_3, csv_4]))

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
if len(sys.argv) is not 5:
    print('Usage: visualize-trial [angle-log] [sphere-log] [plane-log] [pyramid-log] ')
else:
    visualize_trial(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
