""""
This file is used to produce plots to show the relationship between various parameters in the Query, Reduction, 
and Identification trials, or to visualize density in the Nibble database. We assume these trials use the following 
schema:

Query:          IdentificationMethod TEXT, Timestamp TEXT, Sigma1 FLOAT, Sigma2 FLOAT, Sigma3 FLOAT, 
                    ShiftDeviation FLOAT, CandidateSetSize FLOAT, SExistence INT
Reduction:      IdentificationMethod TEXT, Timestamp TEXT, Sigma1 FLOAT, Sigma2 FLOAT, Sigma3 FLOAT, Sigma4 FLOAT, 
                    ShiftDeviation FLOAT, CameraSensitivity FLOAT, TimeToResult FLOAT, ResultMatchesInput INT
Identification: IdentificationMethod TEXT, Timestamp TEXT, Sigma1 FLOAT, Sigma2 FLOAT, Sigma3 FLOAT, Sigma4 FLOAT, 
                    ShiftDeviation FLOAT, CameraSensitivity FLOAT, FalseStars INT, ComparisonCount INT, 
                    TimeToResultFLOAT, PercentageCorrect FLOAT        

There exists two possible arguments passed to this file: the experiment to plot and a secondary location to the
database involved with the operation.

Usage: plot_experiment [experiment-to-visualize] [location-of-database]
"""""

from matplotlib import pyplot as plt
from plot_base import attach_plot_info, attach_figure_legend, e_plot, d_plot


def overlay_plots(cur_i):
    """ Display the plots associated with the overlay experiment.

    :param cur_i: Cursor to database containing result data.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=20)

    plt.figure()
    plt.subplot(121)  # Sigma4 vs. F1 w/ ShiftDeviation visualization.
    for i in ['0.0', '1.0e-6', '1.0e-5', '1.0e-4', '1.0e-3', '1.0e-2']:
        e_plot(cur_i, {'table_name': 'OVERLAY', 'x_attribute': 'Sigma4',
                       'y_attribute': '(TruePositive / (TruePositive + FalseNegative))',
                       'constrain_that': 'ABS(ShiftDeviation - {}) < 1.0e-17 '.format(i) +
                                         'AND FalseStars = 0 ',
                       'params_section': 'overlay-plot', 'params_prefix': 's4as', 'plot_type': 'LINE'})
    a = plt.legend(['0', r'$10^{-6}$', r'$10^{-5}$', r'$10^{-4}$', r'$10^{-3}$', r'$10^{-2}$'])
    a.draggable(True)

    plt.subplot(122)  # Sigma4 vs. F1 w/ FalseStars visualization.
    for i in ['0', '3', '6', '9', '12']:
        e_plot(cur_i, {'table_name': 'OVERLAY', 'x_attribute': 'Sigma4',
                       'y_attribute': '(TruePositive / (TruePositive + FalseNegative))',
                       'constrain_that': 'ABS(ShiftDeviation - 0.0) < 1.0e-17 '
                                         'AND FalseStars = {} '.format(i),
                       'params_section': 'overlay-plot', 'params_prefix': 's4af', 'plot_type': 'LINE'})
    a = plt.legend(['0', '3', '6', '9', '12'])
    a.draggable(True)
    plt.subplots_adjust(wspace=0.3, left=0.06, right=0.98, bottom=0.15, top=0.85), plt.show()


def nibble_plots(cur_j):
    """ Display the plots associated with the nibble database.

    :param cur_j: Cursor to database containing the nibble database.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=20)

    # Angle histogram.
    plt.figure()
    d_plot(cur_j, {'table_name': 'ANGLE_20', 'attributes': ['theta'], 'params_section': 'nibble-plot',
                   'params_prefix': 'nat'})

    # Dot Angle heat maps.
    plt.figure()
    plt.subplot(131)
    d_plot(cur_j, {'table_name': 'DOT_20', 'attributes': ['theta_1', 'phi'], 'params_section': 'nibble-plot',
                   'params_prefix': 'ndat1p'})
    plt.subplot(132)
    d_plot(cur_j, {'table_name': 'DOT_20', 'attributes': ['theta_2', 'phi'], 'params_section': 'nibble-plot',
                   'params_prefix': 'ndat2p'})
    plt.subplot(133)
    d_plot(cur_j, {'table_name': 'DOT_20', 'attributes': ['theta_1', 'theta_2'], 'params_section': 'nibble-plot',
                   'params_prefix': 'ndat1t2'})

    # Planar Triangle heat map.
    plt.figure()
    d_plot(cur_j, {'table_name': 'PLANE_20', 'attributes': ['a', 'i'], 'params_section': 'nibble-plot',
                   'params_prefix': 'nptai'})

    # Spherical Triangle heat map.
    plt.figure()
    d_plot(cur_j, {'table_name': 'SPHERE_20', 'attributes': ['a', 'i'], 'params_section': 'nibble-plot',
                   'params_prefix': 'nstai'})
    plt.subplots_adjust(wspace=0.3, left=0.06, right=0.98, bottom=0.15, top=0.85), plt.show()


def query_sigma_plots(cur_i):
    """ Display the plots associated with the query-sigma experiment.

    :param cur_i: Cursor to database containing result data.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=20)

    # Plot an empty bar chart for the legend.
    p = e_plot(cur_i, {'table_name': 'QUERY', 'x_attribute': 'Sigma1', 'y_attribute': 'SExistence',
                       'constrain_that': '(Sigma2 = Sigma1 OR Sigma2 = 0) '
                                         'AND (Sigma3 = Sigma1 OR Sigma3 = 0) '
                                         'AND ShiftDeviation=0 ',
                       'params_section': 'query-sigma-plot', 'params_prefix': 's1se', 'plot_type': 'BAR'})
    plt.clf(), plt.cla(), plt.close()

    fig = plt.figure()
    plt.subplot(121)  # Sigma1 vs. SExistence visualization.
    e_plot(cur_i, {'table_name': 'QUERY', 'x_attribute': 'Sigma1', 'y_attribute': 'SExistence',
                   'constrain_that': '(Sigma2 = Sigma1 OR Sigma2 = 0) '
                                     'AND (Sigma3 = Sigma1 OR Sigma3 = 0) '
                                     'AND ShiftDeviation=0 ',
                   'params_section': 'query-sigma-plot', 'params_prefix': 's1se', 'plot_type': 'LINE'})

    plt.subplot(122)  # Sigma1 vs. CandidateSetSize visualization.
    e_plot(cur_i, {'table_name': 'QUERY', 'x_attribute': 'Sigma1', 'y_attribute': 'CandidateSetSize',
                   'constrain_that': '(Sigma2 = Sigma1 OR Sigma2 = 0) '
                                     'AND (Sigma3 = Sigma1 OR Sigma3 = 0) '
                                     'AND ShiftDeviation=0 ',
                   'params_section': 'query-sigma-plot', 'params_prefix': 's1css', 'plot_type': 'LINE'})
    attach_figure_legend({'params_section': 'query-sigma-plot'}, fig, p)
    plt.subplots_adjust(wspace=0.3, left=0.06, right=0.98, bottom=0.15, top=0.85), plt.show()


def query_plots(cur_i):
    """ Display the plots associated with the query experiment.

    :param cur_i: Cursor to database containing result data.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=20)

    fig = plt.figure()
    plt.subplot(121)  # ShiftDeviation vs. SExistence visualization.
    e_plot(cur_i, {'table_name': 'QUERY', 'x_attribute': 'ShiftDeviation', 'y_attribute': 'SExistence',
                   'params_section': 'query-plot', 'params_prefix': 'sdse', 'plot_type': 'BAR'})

    plt.subplot(122)  # ShiftDeviation vs. CandidateSetSize visualization.
    p = e_plot(cur_i, {'table_name': 'QUERY', 'x_attribute': 'ShiftDeviation', 'y_attribute': 'CandidateSetSize',
                       'params_section': 'query-plot', 'params_prefix': 'sdcss', 'plot_type': 'BAR'})
    attach_figure_legend({'params_section': 'query-plot'}, fig, p)
    plt.subplots_adjust(wspace=0.3, left=0.06, right=0.98, bottom=0.15, top=0.85), plt.show()

    fig = plt.figure()
    p = e_plot(cur_i, {'table_name': 'QUERY', 'x_attribute': 'ShiftDeviation', 'y_attribute': 'RunningTime',
                       'params_section': 'query-plot', 'params_prefix': 'sdrt', 'plot_type': 'BAR'})
    attach_figure_legend({'params_section': 'query-plot'}, fig, p)
    plt.subplots_adjust(wspace=0.3, left=0.06, right=0.98, bottom=0.15, top=0.85), plt.show()


def reduction_plots(cur_i):
    """ Display the plots associated with the reduction experiment.

    :param cur_i: Cursor to database containing result data.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=20)

    # Plot an empty bar chart for the legend.
    p = e_plot(cur_i, {'table_name': 'REDUCTION', 'x_attribute': 'ShiftDeviation', 'y_attribute': 'PercentageCorrect',
                       'constrain_that': 'FalseStars = 0',
                       'params_section': 'reduction-plot',
                       'params_prefix': 'sdpc', 'plot_type': 'BAR'})
    plt.clf(), plt.cla(), plt.close()

    fig = plt.figure()
    plt.subplot(121)
    e_plot(cur_i, {'table_name': 'REDUCTION', 'x_attribute': 'ShiftDeviation', 'y_attribute': 'PercentageCorrect',
                   'constrain_that': 'FalseStars = 0 AND ShiftDeviation < 1.0e-1 ORDER BY rowid DESC LIMIT 10000',
                   'params_section': 'reduction-plot', 'params_prefix': 'sdpc', 'plot_type': 'BAR_NOERR'})

    plt.subplot(122)
    e_plot(cur_i, {'table_name': 'REDUCTION', 'x_attribute': 'ShiftDeviation', 'y_attribute': 'TimeToResult',
                   'constrain_that': 'FalseStars = 0 AND ShiftDeviation < 1.0e-1 ORDER BY rowid DESC LIMIT 10000',
                   'params_section': 'reduction-plot', 'params_prefix': 'sdttr', 'plot_type': 'BAR_NOERR'})
    attach_figure_legend({'params_section': 'reduction-plot'}, fig, p)
    plt.subplots_adjust(wspace=0.3, left=0.06, right=0.98, bottom=0.15, top=0.85), plt.show()

    # Plot an empty bar chart for the legend.
    p = e_plot(cur_i, {'table_name': 'REDUCTION', 'x_attribute': 'ShiftDeviation', 'y_attribute': 'PercentageCorrect',
                       'constrain_that': 'FalseStars = 0',
                       'params_section': 'reduction-plot',
                       'params_prefix': 'sdpc', 'plot_type': 'BAR'})
    plt.clf(), plt.cla(), plt.close()

    fig = plt.figure()
    plt.subplot(121)
    e_plot(cur_i, {'table_name': 'REDUCTION', 'x_attribute': 'FalseStars', 'y_attribute': 'PercentageCorrect',
                   'constrain_that': 'ABS(ShiftDeviation - 0.0) < 1.0e-17 ORDER BY rowid DESC LIMIT 10000',
                   'params_section': 'reduction-plot', 'params_prefix': 'fspc', 'plot_type': 'BAR_NOERR'})

    plt.subplot(122)
    e_plot(cur_i, {'table_name': 'REDUCTION', 'x_attribute': 'FalseStars', 'y_attribute': 'TimeToResult',
                   'constrain_that': 'ABS(ShiftDeviation - 0.0) < 1.0e-17 ORDER BY rowid DESC LIMIT 10000',
                   'params_section': 'reduction-plot', 'params_prefix': 'fsttr', 'plot_type': 'BAR_NOERR'})
    attach_figure_legend({'params_section': 'reduction-plot'}, fig, p)
    plt.subplots_adjust(wspace=0.3, left=0.06, right=0.98, bottom=0.15, top=0.85), plt.show()


def identification_plots(cur_i):
    """ Display the plots associated with the identification experiment.

    :param cur_i: Cursor to database containing result data.
    :return: None.
    """
    plt.rc('text', usetex=True), plt.rc('font', family='serif', size=20)

    # Plot an empty bar chart for the legend.
    p = e_plot(cur_i, {'table_name': 'IDENTIFICATION', 'x_attribute': 'ShiftDeviation',
                       'y_attribute': 'PercentageCorrect',
                       'constrain_that': 'FalseStars = 0 AND ShiftDeviation < 1.0e-3',
                       'params_section': 'identification-plot', 'params_prefix': 'sdpc', 'plot_type': 'BAR'})
    plt.clf(), plt.cla(), plt.close()

    fig = plt.figure()
    plt.subplot(121)
    e_plot(cur_i, {'table_name': 'IDENTIFICATION', 'x_attribute': 'ShiftDeviation',
                   'y_attribute': 'PercentageCorrect',
                   'constrain_that': 'FalseStars = 0 AND ShiftDeviation < 1.0e-1 LIMIT 10000',
                   'params_section': 'identification-plot', 'params_prefix': 'sdpc', 'plot_type': 'BAR_NOERR'})

    plt.subplot(122)
    e_plot(cur_i, {'table_name': 'IDENTIFICATION', 'x_attribute': 'ShiftDeviation',
                   'y_attribute': 'TimeToResult',
                   'constrain_that': 'FalseStars = 0 AND ShiftDeviation < 1.0e-1 LIMIT 10000',
                   'params_section': 'identification-plot', 'params_prefix': 'sdttr', 'plot_type': 'BAR_NOERR'})
    attach_figure_legend({'params_section': 'identification-plot'}, fig, p)
    plt.subplots_adjust(wspace=0.3, left=0.06, right=0.98, bottom=0.15, top=0.85), plt.show()

    # Plot an empty bar chart for the legend.
    p = e_plot(cur_i, {'table_name': 'IDENTIFICATION', 'x_attribute': 'ShiftDeviation',
                       'y_attribute': 'PercentageCorrect',
                       'constrain_that': 'FalseStars = 0 AND ShiftDeviation < 1.0e-3',
                       'params_section': 'identification-plot', 'params_prefix': 'sdpc', 'plot_type': 'BAR'})
    plt.clf(), plt.cla(), plt.close()

    fig = plt.figure()
    plt.subplot(121)
    e_plot(cur_i, {'table_name': 'IDENTIFICATION', 'x_attribute': 'FalseStars',
                   'y_attribute': 'PercentageCorrect',
                   'constrain_that': 'ABS(ShiftDeviation - 0.0) < 1.0e-17 LIMIT 10000',
                   'params_section': 'identification-plot', 'params_prefix': 'fspc', 'plot_type': 'BAR_NOERR'})

    plt.subplot(122)
    e_plot(cur_i, {'table_name': 'IDENTIFICATION', 'x_attribute': 'FalseStars',
                   'y_attribute': 'TimeToResult',
                   'constrain_that': 'ABS(ShiftDeviation - 0.0) < 1.0e-17 LIMIT 10000',
                   'params_section': 'identification-plot', 'params_prefix': 'fsttr', 'plot_type': 'BAR_NOERR'})

    attach_figure_legend({'params_section': 'identification-plot'}, fig, p)
    plt.subplots_adjust(wspace=0.3, left=0.06, right=0.98, bottom=0.15, top=0.85), plt.show()


if __name__ == '__main__':
    from sqlite3 import connect, Error as SQLError
    from os import path, getcwd, environ
    from sys import argv

    # Ensure that there exists between one and three arguments, and that it is in the appropriate space.
    arg_space = ['nibble', 'query-sigma', 'query', 'reduction', 'identification', 'overlay']
    if len(argv) < 1 or len(argv) > 4 or argv[1] not in arg_space:
        print('Usage: python3 plot_experiment.py [experiment-to-visualize] [location-of-database]'), exit(1)

    # Experiment data in lumberjack.db, or is specified by the user (second argument).
    conn = ''
    try:
        if len(argv) == 3 and not path.isabs(argv[2]):
            conn = connect(getcwd() + '/' + argv[2])
        elif len(argv) == 3:
            conn = connect(argv[2])
        else:
            conn = connect(environ['HOKU_PROJECT_PATH'] + '/data/lumberjack.db')
    except SQLError as e:
        print('SQL Error: ' + str(e)), exit(2)
    cur = conn.cursor()

    plots = [nibble_plots, query_sigma_plots, query_plots, reduction_plots, identification_plots, overlay_plots]
    plots[arg_space.index(argv[1])](cur)
