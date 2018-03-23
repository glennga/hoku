""""
This file is used to supplement 'plot_experiment.py' with 
This file is used to produce plots to show the relationship between various parameters in the Query, Reduction, 
and Identification trials. We assume these trials use the following schema:

Query:          IdentificationMethod TEXT, Timestamp TEXT, Sigma1 FLOAT, Sigma2 FLOAT, Sigma3 FLOAT, 
                    ShiftDeviation FLOAT, CandidateSetSize FLOAT, SExistence INT
Reduction:      IdentificationMethod TEXT, Timestamp TEXT, Sigma1 FLOAT, Sigma2 FLOAT, Sigma3 FLOAT, Sigma4 FLOAT, 
                    ShiftDeviation FLOAT, CameraSensitivity FLOAT, ResultMatchesInput INT
Identification: IdentificationMethod TEXT, Timestamp TEXT, Sigma1 FLOAT, Sigma2 FLOAT, Sigma3 FLOAT, Sigma4 FLOAT, 
                    ShiftDeviation FLOAT, CameraSensitivity FLOAT, FalseStars INT, ComparisonCount INT, 
                    PercentageCorrect FLOAT        

There exists two possible arguments passed to this file: the experiment to plot and a secondary location to the
lumberjack database.

Usage: plot_experiment [experiment-to-visualize] [location-of-lumberjack]
"""""

import sqlite3
import json
import sys
import os

import numpy as np
from matplotlib import pyplot as plt
from plot_parameters import params


def attach_figure_legend(att, fig, p):
    """ Attach a legend to a figure, rather than the current 'plt' object.

    :param att: Attributes dictionary, containing the key to which axes to label.
    :param fig: Figure to attach the legend to.
    :param p: Plot object to attach the legend to (grabs what was just plotted).
    :return: None.
    """
    sec = att['params_section']
    leg = fig.legend(p, params[sec]['ll'], fontsize=18,
                     ncol=len(params[sec]['ll']), mode='expand')
    leg.draggable(True)


def attach_plot_info(att):
    """ Attach plot characteristics to the global 'plt' object, given the attributes in 'att'.

    :param att: Dictionary containing attributes, y transformation, and configuration file prefix to plot with.
    :return: None.
    """
    sec, pre = att['params_section'], att['params_prefix']
    ax = plt.gca()

    def attempt(f, h=lambda _: None):
        """ Wrap the given lambda(s) in a try catch. If this throws a KeyError, then we simply ignore it.

        :param f: Initial function to try. Result is saved and used with the H function.
        :param h: If defined, use the output of F with this.
        :return: None.
        """
        try:
            h(f(None))
        except KeyError:
            pass

    # Set Y limits.
    attempt(lambda _: ax.set_ylim(params[sec][pre + '-yll']))

    # If desired, set the X axis as logarithmic.
    attempt(lambda _: ax.set_xscale('log') if params[sec][pre + '-lxa'] == 1 else None)

    # If desired, set the Y axis as logarithmic.
    attempt(lambda _: ax.set_yscale('log') if params[sec][pre + '-lya'] == 1 else None)

    # Add the legend.
    attempt(lambda _: plt.legend(params[sec]['ll'], fontsize=18)
    if not pre + '-nll' in params[sec] or params[sec][pre + '-nll'] == 0 else None,
            lambda a: a.draggable(True) if a is not None else None)

    # Add the chart X axis tick labels.
    attempt(lambda _: plt.xticks(np.arange(len(params[sec][pre + '-xtl'])), params[sec][pre + '-xtl']))

    # Add the chart X and Y labels.
    attempt(lambda _: plt.xlabel(params[sec][pre + '-xal']) and plt.ylabel(params[sec][pre + '-yal']))


def d_plot(cur_j, att):
    """ Generic density plot function. Given a cursor to the nibble database and a dictionary of attributes,
    create a histogram or a bar plot in global 'plt' object. User can choose to move this into a subplot or make this
    one plot.

    :param cur_j: Cursor to nibble database.
    :param att: Dictionary containing attributes and params prefix to plot with.
    :return: None.
    """
    sec, pre = att['params_section'], att['params_prefix']

    if len(att['attributes']) == 1:
        # Collect our data into memory.
        d = cur_j.execute('SELECT {} '.format(att['attributes'][0]) +
                          'FROM {}'.format(att['table_name'])).fetchall()
        x_0 = [x[0] for x in d]

        # Plot the histogram.
        img_bits = params[sec]['image-bits']
        plt.hist(x_0, bins=np.arange(img_bits) * (np.max(x_0) - np.min(x_0)) / img_bits)

    elif len(att['attributes']) == 2:
        # Collect our data into memory.
        d = cur_j.execute('SELECT {}, {} '.format(att['attributes'][0], att['attributes'][1]) +
                          'FROM {}'.format(att['table_name'])).fetchall()
        x_0, x_1 = [x[0] for x in d], [x[1] for x in d]

        # Construct the normalized histograms for each attribute.
        img_bits = params[sec]['image-bits']
        hist_x_0, _ = np.histogram(x_0, bins=np.arange(img_bits) * (np.max(x_0) - np.min(x_0)) / img_bits)
        hist_x_1, _ = np.histogram(x_1, bins=np.arange(img_bits) * (np.max(x_1) - np.min(x_1)) / img_bits)

        # Construct the mesh grid (the image points), and the pixel intensity. Both attributes hold equal weight.
        xx_0, xx_1 = np.meshgrid(hist_x_0, hist_x_1, sparse=True)
        zz = (xx_0 / 2 + xx_1 / 2)

        # Construct the plot.
        plt.imshow(zz, interpolation='none')

        # Redefine the X and Y ticks.
        ax, x_0_t = plt.gca(), np.linspace(0, img_bits, len(params[sec][pre + '-xhl']))
        x_1_t = np.linspace(0, img_bits, len(params[sec][pre + '-yhl']))
        ax.set_xticks(x_0_t), ax.set_xticklabels(params[sec][pre + '-xhl'])
        ax.set_yticks(x_1_t), ax.set_yticklabels(params[sec][pre + '-yhl'])

    attach_plot_info(att)


def e_plot(cur_i, att):
    """ Generic plot function. Given a cursor to the results database and a dictionary of attributes, create a bar or
    line plot in global 'plt' object. User can choose to move this into a subplot or make this one plot.

    :param cur_i: Cursor to database containing result data.
    :param att: Dictionary containing attributes and params prefix to plot with.
    :return: All axes plotted.
    """
    plots = []

    for k, m in enumerate(params[att['params_section']]['ll']):
        # Grab the possible X-axis values.
        x_space = cur_i.execute('SELECT DISTINCT {} '.format(att['x_attribute']) +
                                'FROM {} '.format(att['table_name']) +
                                'WHERE IdentificationMethod = ? ' +
                                ('' if 'constrain_that' not in att.keys() else 'AND ' + att['constrain_that']),
                                (m,)).fetchall()
        x_space = sorted(list(map(lambda x: x[0], x_space)))

        # Construct the points to plot.
        y_avg, y_std = [], []
        for x_p in x_space:
            y_data = cur_i.execute('SELECT {} '.format(att['y_attribute']) +  # Grab our Y axis data.
                                   'FROM {} '.format(att['table_name']) +
                                   'WHERE IdentificationMethod = ? '
                                   'AND {} = ? '.format(att['x_attribute']) +
                                   ('' if 'constrain_that' not in att.keys() else 'AND ' + att['constrain_that']),
                                   (m, x_p)).fetchall()
            y_data = list(map(lambda y: y[0], y_data))

            # Divide data into n sections. Apply AVG to each split of y data.
            y_data = np.split(np.array(y_data), params['split-n'])
            y_data = list(map(lambda y: np.mean(y), y_data))

            # Collect averages and deviations for each partition of data.
            y_avg.append(np.mean(y_data)), y_std.append(np.std(y_data))

        # Construct the appropriate plot and attach plot info.
        if att['plot_type'] == 'BAR':
            plots.append(plt.bar(np.arange(len(x_space)) + 0.15 * k - 0.3, y_avg, 0.15, yerr=y_std))
            attach_plot_info(att)
        elif att['plot_type'] in ['LINE', 'LINE_NOERR']:
            plots.append(plt.plot(x_space, y_avg))
            attach_plot_info(att)
        else:
            plots.append(plt.errorbar(x_space, y_avg, yerr=y_std))
            attach_plot_info(att)

    return plots
