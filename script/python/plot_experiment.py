""""
This file is used to produce plots to show the relationship between various parameters in the Query, Reduction, 
and Identification trials. We assume these trials use the following schema:

Query:          IdentificationMethod TEXT, Timestamp TEXT, Sigma1 FLOAT, Sigma2 FLOAT, Sigma3 FLOAT, 
                    ShiftDeviation FLOAT, CandidateSetSize FLOAT, SExistence INT
Reduction:      IdentificationMethod TEXT, Timestamp TEXT, Sigma1 FLOAT, Sigma2 FLOAT, Sigma3 FLOAT, Sigma4 FLOAT, 
                    ShiftDeviation FLOAT, CameraSensitivity FLOAT, ResultMatchesInput INT
Identification: IdentificationMethod TEXT, Timestamp TEXT, Sigma1 FLOAT, Sigma2 FLOAT, Sigma3 FLOAT, Sigma4 FLOAT, 
                    ShiftDeviation FLOAT, CameraSensitivity FLOAT, FalseStars INT, ComparisonCount INT, 
                    PercentageCorrect FLOAT        

The only argument passed to this file is type of experiment to plot. This must be in the space [query, reduction, 
identification].

Usage: plot_experiment [experiment-to-visualize]
"""""

import sqlite3
import json
import sys
import os

import numpy as np
from matplotlib import pyplot as plt
from plot_parameters import params


def attach_plot_info(att):
    """ Attach plot characteristics to the global 'plt' object, given the attributes in 'att'.

    :param att: Dictionary containing attributes, y transformation, and configuration file prefix to plot with.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=12)
    sec, pre = att['param_section'], att['params_prefix']

    # Set Y limits.
    ax = plt.gca()
    ax.set_ylim(params[sec][pre + '-yll'])

    # Add the bar chart X axis tick labels.
    x_labels = params[sec][pre + '-xtl']
    plt.xticks(np.arange(len(x_labels)), x_labels)

    # Add the chart X and Y labels.
    plt.xlabel(params[sec][pre + '-xal']), plt.ylabel(params[sec][pre + '-yal'])


def bar_plot(cur_i, att):
    """ Generic bar plot function. Given a cursor to the results database and a dictionary of attributes, create a bar
    plot in global 'plt' object. User can choose to move this into a subplot or make this one plot.

    :param cur_i: Cursor to database containing result data.
    :param att: Dictionary containing attributes, y transformation, and params prefix to plot with.
    :return: None.
    """
    for k, m in enumerate(['Angle', 'Dot', 'Sphere', 'Plane', 'Pyramid', 'Composite']):
        # Grab the possible X-axis values.
        x_space = cur_i.execute('SELECT DISTINCT ? '
                                'FROM {}'.format(att['table_name']), (att['x_attribute'],)).fetchall()
        x_space = list(map(lambda x: x[0], x_space))

        # Construct the points to plot.
        y_avg, y_std = [[] for _ in x_space], [[] for _ in x_space]
        for x_p in x_space:
            y_data = cur_i.execute('SELECT ? '  # Grab our Y axis data. 
                                   'FROM {} '
                                   'WHERE IdentificationMethod = ? '
                                   'AND ? = ? ' +
                                   ('' if 'constrain_that' not in att.keys()
                                   else 'AND ' + att['constrain_that']).format(att['table_name']),
                                   (att['y_attribute'], m, att['x_attribute'], x_p)).fetchall()
            y_data = list(map(lambda y: y[0], y_data))

            # Divide data into n sections. Apply lambda to each split of y data.
            y_data = np.split(np.array(y_data), params['split-n'])
            y_data = list(map(lambda y: att['y_function'](y), y_data))

            # Collect averages and deviations for each partition of data.
            y_avg.append(np.mean(y_data)[0]), y_std.append(np.std(y_data)[0])

        # Construct the bar plot and attach plot info.
        plt.bar(np.arange(len(x_space)) + 0.15 * k - 0.3, y_avg, 0.15, yerr=y_std)
        attach_plot_info(att)


if __name__ == '__main__':
    # Ensure that there exists only one argument, and that it is in the appropriate space.
    if len(sys.argv) != 2 or sys.argv[1] not in ['query', 'reduction', 'identification']:
        print('Usage: python3 plot_experiment.py [query/reduction/identification]'), exit(1)

    # Experiment data in lumberjack.db.
    conn = ''
    try:
        conn = sqlite3.connect(os.environ['HOKU_PROJECT_PATH'] + '/CONFIG.ini')
    except sqlite3.Error as e:
        print('SQL Error: ' + str(e)), exit(2)
    cur = conn.cursor(os.environ['HOKU_PROJECT_PATH'] + '/data/lumberjack.db')

    if sys.argv[1] == 'query':
        # Run all query visualizations.
        plt.figure()
        bar_plot(cur, {table_name: 'QUERY', x_attribute: 'ShiftDeviation', y_attribute: 'SExistence',
                   y_function: lambda y: np.sum(y), params_section: 'query-plot', params_prefix: 'sdse'})
        plt.show()
        plt.figure()
        bar_plot(cur, {table_name: 'QUERY', x_attribute: 'ShiftDeviation', y_attribute: 'CandidateSetSize',
                   y_function: lambda y: np.mean(y), params_section: 'query-plot', params_prefix: 'sdcss'})
        plt.show()
    elif sys.argv[1] == 'reduction':
        # Run all reduction visualizations.
        bar_plot(cur, {table_name: 'REDUCTION', x_attribute: 'ShiftDeviation', y_attribute: 'ResultMatchesInput',
                   constrain_that: 'CameraSensitivity = 6.0', y_function: lambda y: np.sum(y),
                   params_section: 'reduction-plot', params_prefix: 'sdrmi'})
        bar_plot(cur, {table_name: 'REDUCTION', x_attribute: 'CameraSensitivity', y_attribute: 'ResultMatchesInput',
                   constrain_that: 'ShiftDeviation = 1.0e-3', y_function: lambda y: np.sum(y),
                   params_section: 'reduction-plot', params_prefix: 'csrmi'})
    else:
        # Run all identification visualizations.
        bar_plot(cf, cur, 'ShiftDeviation', 'ComparisonCount', 'AVG', 'i-sdcc')
        bar_plot(cf, cur, 'ShiftDeviation', 'PercentageCorrect', 'AVG', 'i-sspc')
        bar_plot(cf, cur, 'CameraSensitivity', 'ComparisonCount', 'AVG', 'i-cscc')
        bar_plot(cf, cur, 'CameraSensitivity', 'PercentageCorrect', 'AVG', 'i-cspc')
        bar_plot(cf, cur, 'FalseStars', 'ComparisonCount', 'AVG', 'i-fscc')
        bar_plot(cf, cur, 'FalseStars', 'PercentageCorrect', 'AVG', 'i-fspc')
