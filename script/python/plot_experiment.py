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

There exists two possible arguments passed to this file: the experiment to plot and a secondary location to the
lumberjack database.

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
    sec, pre = att['params_section'], att['params_prefix']

    # Set Y limits.
    ax = plt.gca()
    ax.set_ylim(params[sec][pre + '-yll'])

    # If desired, set the X axis as logarithmic.
    try:
        if params[sec][pre + '-lxa'] == 1:
            ax.set_xscale('log')
    except KeyError:
        pass

    # Add the legend.
    leg = plt.legend(params[sec]['ll'], fontsize=18)
    leg.draggable(True)

    # Add the chart X axis tick labels (if available).
    try:
        x_labels = params[sec][pre + '-xtl']
        plt.xticks(np.arange(len(x_labels)), x_labels)
    except KeyError:
        pass

    # Add the chart X and Y labels.
    plt.xlabel(params[sec][pre + '-xal']), plt.ylabel(params[sec][pre + '-yal'])


def e_plot(cur_i, att):
    """ Generic plot function. Given a cursor to the results database and a dictionary of attributes, create a bar or
    line plot in global 'plt' object. User can choose to move this into a subplot or make this one plot.

    :param cur_i: Cursor to database containing result data.
    :param att: Dictionary containing attributes and params prefix to plot with.
    :return: None.
    """
    for k, m in enumerate(params[att['params_section']]['ll']):
        # Grab the possible X-axis values.
        x_space = cur_i.execute('SELECT DISTINCT {} '.format(att['x_attribute']) +
                                'FROM {} '.format(att['table_name']) +
                                'WHERE IdentificationMethod = ? ' +
                                ('' if 'constrain_that' not in att.keys() else 'AND ' + att['constrain_that']),
                                (m,)).fetchall()
        x_space = list(map(lambda x: x[0], x_space))

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
            plt.bar(np.arange(len(x_space)) + 0.15 * k - 0.3, y_avg, 0.15, yerr=y_std)
            attach_plot_info(att)
        else:
            plt.plot(x_space, y_avg)
            attach_plot_info(att)
            # plt.errorbar(x_space, y_avg, yerr=y_std)


def query_plots(cur_i):
    """ Display the plots associated with the query experiment.

    :param cur_i: Cursor to database containing result data.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=12)

    plt.figure()
    plt.subplot(121)  # Sigma1 vs. SExistence visualization.
    e_plot(cur_i, {'table_name': 'QUERY', 'x_attribute': 'Sigma1', 'y_attribute': 'SExistence',
                   'constrain_that': '(Sigma2 = Sigma1 OR Sigma2 = 0) '
                                     'AND (Sigma3 = Sigma1 OR Sigma3 = 0) '
                                     'AND ShiftDeviation=0 ',
                   'params_section': 'query-plot', 'params_prefix': 's1se', 'plot_type': 'LINE'})

    plt.subplot(122)  # Sigma1 vs. CandidateSetSize visualization.
    e_plot(cur_i, {'table_name': 'QUERY', 'x_attribute': 'Sigma1', 'y_attribute': 'CandidateSetSize',
                   'constrain_that': '(Sigma2 = Sigma1 OR Sigma2 = 0) '
                                     'AND (Sigma3 = Sigma1 OR Sigma3 = 0) '
                                     'AND ShiftDeviation=0 ',
                   'params_section': 'query-plot', 'params_prefix': 's1css', 'plot_type': 'LINE'})

    # plt.figure()
    # plt.subplot(121)  # ShiftDeviation vs. SExistence visualization.
    # e_plot(cur_i, {'table_name': 'QUERY', 'x_attribute': 'ShiftDeviation', 'y_attribute': 'SExistence',
    #                  'params_section': 'query-plot', 'params_prefix': 'sdse', 'plot_type': 'BAR'})
    #
    # plt.subplot(122)  # ShiftDeviation vs. CandidateSetSize visualization.
    # bar_plot(cur_i, {'table_name': 'QUERY', 'x_attribute': 'ShiftDeviation', 'y_attribute': 'CandidateSetSize',
    #                  'params_section': 'query-plot', 'params_prefix': 'sdcss', 'plot_type': 'BAR'})
    plt.show()


def reduction_plots(cur_i):
    """ Display the plots associated with the reduction experiment.

    :param cur_i: Cursor to database containing result data.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=12)

    plt.figure()
    plt.subplot(121)
    e_plot(cur_i, {'table_name': 'REDUCTION', 'x_attribute': 'ShiftDeviation', 'y_attribute': 'ResultMatchesInput',
                   'constrain_that': 'CameraSensitivity = 6.0', 'params_section': 'reduction-plot',
                   'params_prefix': 'sdrmi'})

    plt.subplot(122)
    e_plot(cur_i, {'table_name': 'REDUCTION', 'x_attribute': 'CameraSensitivity', 'y_attribute': 'ResultMatchesInput',
                   'constrain_that': 'ShiftDeviation = 1.0e-3', 'params_section': 'reduction-plot',
                   'params_prefix': 'csrmi'})
    plt.show()


def identification_plots(cur_i):
    """ Display the plots associated with the identification experiment.

    :param cur_i: Cursor to database containing result data.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=12)


if __name__ == '__main__':
    # Ensure that there exists only one or two arguments, and that it is in the appropriate space.
    if len(sys.argv) < 1 or len(sys.argv) > 3 or sys.argv[1] not in ['query', 'reduction', 'identification']:
        print('Usage: python3 plot_experiment.py [query/reduction/identification] [lumberjack-location]'), exit(1)

    # Experiment data in lumberjack.db, or is specified by the user (second argument).
    conn = ''
    try:
        if len(sys.argv) == 3 and not os.path.isabs(sys.argv[2]):
            conn = sqlite3.connect(os.getcwd() + '/' + sys.argv[2])
        elif len(sys.argv) == 3:
            conn = sqlite3.connect(sys.argv[2])
        else:
            conn = sqlite3.connect(os.environ['HOKU_PROJECT_PATH'] + '/data/lumberjack.db')
    except sqlite3.Error as e:
        print('SQL Error: ' + str(e)), exit(2)
    cur = conn.cursor()

    if sys.argv[1] == 'query':
        query_plots(cur)
    elif sys.argv[1] == 'reduction':
        reduction_plots(cur)
    else:
        identification_plots(cur)
