""""
This file is used to generate the base plots for each trial: Query, Alignment, Reduction, Semi-Crown and Crown.
"""""

import matplotlib.pyplot as plt
import numpy as np
import os, csv, sys


def bar_plot(ppv, log, k, x_index, y_index, restrict_d=None, restrict_y=lambda h: True, y_divisor=None,
             display_err=True):
    """ Generic bar plot function, given the indices of the X and Y data (with respect the the log), as well as a
    restriction function.

    :param ppv: Points per variation.
    :param log: List of lists representing the contents of a single log file.
    :param k: Current log file iteration (in space [0, 1, 2, 3]).
    :param x_index: Index of the CSV column that represents dimension X of the data point.
    :param y_index: Index of the CSV column that represents dimension Y of the data point.
    :param restrict_d: A restriction function to apply to the data-set we operate on.
    :param restrict_y: A restriction function to apply to every list of Y points. Defaults to using data as-is.
    :param y_divisor: If specified, we divide every Y point with the point at this index.
    :param display_err: If specified, we display error-bars.
    :return: None.
    """

    # Apply a restriction to our data-set if given. Determine our X axis.
    d = [tuple for tuple in log if (True if restrict_d is None else restrict_d(tuple))]
    sorted(d, key=lambda x: x[x_index])
    x_count = list(set([float(x[x_index]) for x in d]))
    y_list, avg_y, std_y = [[] for _ in x_count], [[] for _ in x_count], [0 for _ in x_count]

    # Determine our Y axis. Dependent on if y_divisor is specified or not.
    if y_divisor is None:
        for i in range(0, len(x_count)):
            for j in range(0, ppv):
                y_list[i].append(float(d[i * ppv + j][y_index]))
    else:
        for i in range(0, len(x_count)):
            for j in range(0, ppv):
                y_list.append(float(d[i * ppv + j][y_index] / d[i * ppv + j][y_divisor]))

    # Find 10 averages over our data.
    for i in range(len(x_count)):
        for j in range(10):
            b, e = int((ppv / 10) * j), int((ppv / 10) * (j + 1))
            raw = np.array(y_list[i])[b:]
            avg_y[i].append(np.mean([y if restrict_y(y) else 0 for y in raw]))

    # Determine the deviations of this data.
    std_y = list(map(lambda s: np.std(s), avg_y))
    print('{} : {}'.format(log[0][0], std_y))

    # Plot the bar chart of our averages, as well as the corresponding error bars.
    if display_err:
        plt.bar(np.arange(len(x_count)) + 0.15 * k - 0.3, [np.mean(y) for y in avg_y], 0.15, yerr=std_y)
    else:
        plt.bar(np.arange(len(x_count)) + 0.15 * k - 0.3, [np.mean(y) for y in avg_y], 0.15)


def plot_add_info(params):
    """ Adds to the global plot, the given figure characteristics.

    :param params: Figure parameters to attach.
    :return: None.
    """

    # Set Y limits.
    ax = plt.gca()
    ax.set_ylim(next(params["yll"]))

    # Add the bar chart X axis tick labels.
    x_labels = next(params["xtl"])
    plt.xticks(np.arange(len(x_labels)), x_labels)

    # Add the legend.
    leg = plt.legend(next(params["ll"]), fontsize=18)
    leg.draggable(True)

    # Add the chart X and Y labels.
    plt.xlabel(next(params["xal"])), plt.ylabel(next(params["yal"]))


def query_trial_plot(ppv, log_sets, plot_params):
    """ For every log in log_sets (which hold sets of tuples), display a plot comparing each.

    The following plots are displayed: ShiftSigma vs. P(SExistence)
                                       ShiftSigma vs. CandidateSetSize

    :param ppv: The number of points per variation of each experiment.
    :param log_sets: List of list of lists representing the contents of the several log files.
    :param plot_params: The variables that determine how to decorate the plot.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=12)

    plt.figure()
    [bar_plot(ppv, log, k, 2, 4) for k, log in enumerate(log_sets)]
    plot_add_info(plot_params)

    plt.figure()
    # [bar_plot(ppv, log, k, 2, 3, restrict_y=lambda h: [a for a in h if a > 1]) for k, log in enumerate(log_sets)]
    [bar_plot(ppv, log, k, 2, 3) for k, log in enumerate(log_sets)]
    plot_add_info(plot_params)

    plt.show()


def alignment_trial_plot(ppv, log_sets, plot_params):
    """ For every log in log_sets (which hold sets of tuples), display a plot comparing each.

    The following plots are displayed: Camera Sensitivity vs. ||Original Star - Estimated Star||
                                       ShiftSigma vs. ||Original Star - Estimate Star||

    :param ppv: The number of points per variation of each experiment.
    :param log_sets: List of list of lists representing the contents of the several log files.
    :param plot_params: The variables that determine how to decorate the plot.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=12)

    plt.figure()
    [bar_plot(ppv, log, k, 3, 6, lambda g: g[2] == '1e-05') for k, log in enumerate(log_sets)]
    plot_add_info(plot_params)
    plt.yscale('log')

    plt.figure()
    [bar_plot(ppv, log, k, 2, 6, lambda g: g[3] == '6') for k, log in enumerate(log_sets)]
    plot_add_info(plot_params)
    plt.yscale('log')

    plt.show()


def reduction_trial_plot(ppv, log_sets, plot_params):
    """ For every log in log_sets (which hold sets of tuples), display a plot comparing each.

    The following plots are displayed: Shift Sigma vs. P(Correctly Identified Star Set)
                                       Camera Sensitivity vs. P(Correctly Identified Star Set)

    :param ppv: The number of points per variation of each experiment.
    :param log_sets: List of list of lists representing the contents of the several log files.
    :param plot_params: The variables that determine how to decorate the plot.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=25)

    # Plot #1: Shift Sigma vs. P(Correctly Identified Star Set)
    plt.figure()
    [bar_plot(ppv, log, k, 3, 5, lambda g: g[4] == '6') for k, log in enumerate(log_sets)]
    plot_add_info(plot_params)

    # Plot #2: Camera Sensitivity vs. P(Correctly Identified Star Set)
    plt.figure()
    [bar_plot(ppv, log, k, 4, 5, lambda g: g[3] == '1e-05') for k, log in enumerate(log_sets)]
    plot_add_info(plot_params)

    plt.show()


def semi_crown_trial_plot(ppv, log_sets, plot_params):
    """ For every log in log_sets (which hold sets of tuples), display a plot comparing each.

    The following plots are displayed: False Stars vs. Rotational Error
                                       Shift Sigma vs. Rotational Error
                                       False Stars vs. Number of Star Sets Exhausted
                                       Shift Sigma vs. Number of Star Sets Exhausted

    :param ppv: The number of points per variation of each experiment.
    :param log_sets: List of list of lists representing the contents of the several log files.
    :param plot_params: The variables that determine how to decorate the plot.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=25)

    plt.figure()
    [bar_plot(ppv, log, k, 5, 7, lambda g: g[3] == '0' and g[4] == '6') for k, log in enumerate(log_sets)]
    plot_add_info(plot_params)

    # Need to keep false stars zero here.
    plt.figure()
    [bar_plot(ppv, log, k, 3, 7, lambda g: g[4] == '6' and g[5] == '0')
     for k, log in enumerate(log_sets)]
    plot_add_info(plot_params)

    plt.figure()
    [bar_plot(ppv, log, k, 5, 6, lambda g: g[3] == '0' and g[4] == '6') for k, log in enumerate(log_sets)]
    plot_add_info(plot_params)

    # Need to keep false stars zero here.
    plt.figure()
    [bar_plot(ppv, log, k, 3, 6, lambda g: g[4] == '6' and g[5] == '0') for k, log in enumerate(log_sets)]
    plot_add_info(plot_params)

    plt.show()


def crown_trial_plot(ppv, log_sets, plot_params):
    """ For every log in log_sets (which hold sets of tuples), display a plot comparing each.

    The following plots are displayed: False Percentage vs. |Correct Stars| / |Total Number of True Stars|
                                       False Percentage vs. Number of Star Sets Exhausted
                                       Shift Sigma vs. |Correct Stars| / |Total Number of True Stars|
                                       Shift Sigma vs. Number of Star Sets Exhausted

    :param ppv: The number of points per variation of each experiment.
    :param log_sets: List of list of lists representing the contents of the several log files.
    :param plot_params: The variables that determine how to decorate the plot.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=12)

    plt.figure()
    [bar_plot(ppv, log, k, 5, 9, lambda g: g[3] == '0' and g[4] == '6') for k, log in enumerate(log_sets[:-1])]
    # plot_add_info(plot_params)

    plt.figure()
    [bar_plot(ppv, log, k, 5, 6, lambda g: g[3] == '0' and g[4] == '6') for k, log in enumerate(log_sets[:-1])]
    # plot_add_info(plot_params)

    plt.figure()
    [bar_plot(ppv, log, k, 3, 9, lambda g: g[5] == '0' and g[4] == '6') for k, log in enumerate(log_sets[:-1])]
    # plot_add_info(plot_params)

    plt.figure()
    [bar_plot(ppv, log, k, 3, 6, lambda g: g[5] == '0' and g[4] == '6') for k, log in enumerate(log_sets[:-1])]
    # plot_add_info(plot_params)

    plt.show()
