""""
This file is used to produce plots to show the relationship between various parameters in the Query, Alignment, 
Reduction, SemiCrown, and Crown trials. We assume these trials use the following attributes:

Query:      IdentificationMethod,QuerySigma,ShiftSigma,CandidateSetSize,SExistence [5]
Alignment:  IdentificationMethod,MatchSigma,ShiftSigma,MBar,OptimalConfigRotation,
            NonOptimalConfigRotation,OptimalComponentError,NonOptimalComponentError [8]
Reduction:  IdentificationMethod,MatchSigma,QuerySigma,ShiftSigma,CameraSensitivity,ResultMatchesInput [6]
Semi-Crown: IdentificationMethod,MatchSigma,QuerySigma,ShiftSigma,CameraSensitivity,FalseStars,
            ComparisonCount,ComponentError [8]
Crown:      IdentificationMethod,MatchSigma,QuerySigma,ShiftSigma,CameraSensitivity,FalseStars,
            ComparisonCount,BenchmarkSetSize,ResultSetSize,PercentageCorrectInCleanResultSet [10]

The first argument is the location of the log file. We can infer the type of trial from the fifth element of the header.

Usage: visualize-trial [angle-log] [sphere-log] [plane-log] [pyramid-log] [coin-log]
"""""

import csv
import sys

import numpy as np
import visualize_base_trial as v

# Points per variation, defined in each trial's header file.
qu_ppv, al_ppv, r_ppv, sc_ppv, cr_ppv = 1000, 1000, 1000, 1000, 1000

# Plot parameters for query trials.
# noinspection PyRedeclaration
query_params = {"yll": iter([[0, 1.05], [0, 1.3]]),
                "xtl": iter([[r'$0$'] + [r'$10^{-7}$'] + [r'$10^{-6}$'] + [r'$10^{-5}$'] + [r'$10^{-4}$']
                             for _ in range(0, 2)]),
                "ll": iter([['Angle', 'Spherical Triangle', 'Planar Triangle', 'Pyramid', 'CoIn'] for _ in range(3)]),
                "xal": iter(['Noise (Degrees)', 'Noise (Degrees)']),
                "yal": iter(['P(Correct Star Set in Candidates)', r'$|$Candidate Set$|$'])}

# Plot parameters for alignment trials.
# noinspection PyRedeclaration
alignment_params = {"yll": iter([[0, 0.002], [0, 1]]),
                    "xtl": iter([['6.0', '6.25', '6.5', '6.75', '7.0']] +
                                [[r'$0$'] + [r'$10^{-6}$'] + [r'$10^{-5}$'] + [r'$10^{-4}$'] + [r'$10^{-3}$']]),
                    "ll": iter([['Angle', 'Spherical Triangle', 'Planar Triangle', 'Pyramid', 'CoIn']
                                for _ in range(2)]),
                    "xal": iter([r'Camera Sensitivity (m)', 'Noise (Degrees)']),
                    "yal": iter([r'Rotational Error' for _ in range(2)])}

# Plot parameters for reduction trials.
# noinspection PyRedeclaration
reduction_params = {"yll": iter([[0, 1.05], [0, 1]]),
                    "xtl": iter([[r'$0$'] + [r'$10^{-7}$'] + [r'$10^{-6}$'] + [r'$10^{-5}$'] + [r'$10^{-4}$']] +
                                [['6.0'] + ['6.25'] + ['6.5'] + ['6.75'] + ['7.0']]),
                    "ll": iter([['Angle', 'Spherical Triangle', 'Planar Triangle', 'Pyramid', 'CoIn']
                                for _ in range(2)]),
                    "xal": iter(['Noise (Degrees)'] + ['Camera Sensitivity (m)']),
                    "yal": iter(['P(Correct Star Set After Reduction)' for _ in range(2)])}

# Plot parameters for semi-crown trials.
semi_crown_params = {"yll": iter([[0, 1], [0, 1], [0, 1.3], [0, 40]]),
                     "xtl": iter([['0', '5', '10', '15', '20']] +
                                 [[r'$0$'] + [r'$10^{-7}$'] + [r'$10^{-6}$'] + [r'$10^{-5}$'] + [r'$10^{-4}$']] +
                                 [['0', '5', '10', '15', '20']] +
                                 [[r'$0$'] + [r'$10^{-7}$'] + [r'$10^{-6}$'] + [r'$10^{-5}$'] + [r'$10^{-4}$']]),
                     "ll": iter([['Angle', 'Spherical Triangle', 'Planar Triangle', 'Pyramid', 'CoIn']
                                 for _ in range(4)]),
                     "xal": iter(['False Stars'] + ['Noise (Degrees)'] + ['False Stars'] + ['Noise (Degrees)']),
                     "yal": iter(['Rotational Error'] + ['Rotational Error'] + ['Star Sets Exhausted'] +
                                 ['Star Sets Exhausted'])}

# Plot parameters for crown trials.
# noinspection PyRedeclaration
crown_params = {"yll": iter([]),
                "xtl": iter([]),
                "ll": iter([]),
                "xal": iter([]),
                "yal": iter([])}


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
        csvs = list(map(lambda f: csv.reader(f, delimiter=','), [f_1, f_2, f_3, f_4, f_5]))

        # with open(angle_log, 'r') as f_1, open(sphere_log, 'r') as f_2, open(pyramid_log, 'r') as f_4, \
        #         open(coin_log, 'r') as f_5:
        #     csvs = list(map(lambda f: csv.reader(f, delimiter=','), [f_1, f_2, f_4, f_5]))

        # Parse our header, and the rest of the logs.
        attributes = list(map(lambda c: next(c), csvs))
        logs = list(map(lambda c: np.array(np.array([tuple for tuple in c])), csvs))

        # Based on the header, determine what plots to produce.
        if all(map(lambda a: len(a) == 5, attributes)):
            v.query_trial_plot(qu_ppv, logs, query_params)
        elif all(map(lambda a: a[4] == 'OptimalConfigRotation', attributes)):
            v.alignment_trial_plot(al_ppv, logs, alignment_params)
        elif all(map(lambda a: len(a) == 6, attributes)):
            v.reduction_trial_plot(r_ppv, logs, reduction_params)
        elif all(map(lambda a: a[4] == 'CameraSensitivity' and len(a) == 8, attributes)):
            v.semi_crown_trial_plot(sc_ppv, logs, semi_crown_params)
        elif all(map(lambda a: len(a) == 10, attributes)):
            v.crown_trial_plot(cr_ppv, logs, crown_params)
        else:
            raise ValueError('Not a log file or corrupt header.')


# Perform the trials!
if len(sys.argv) is not 6:
    print('Usage: visualize-trial [angle-log] [sphere-log] [plane-log] [pyramid-log] [coin-log]')
else:
    visualize_trial(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
