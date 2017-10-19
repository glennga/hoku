""""
This file is used to produce plots to show the relationship between various parameters in the Query, Alignment, 
and Crown trials. We assume the Query, Alignment, and Crown trials use the following attributes:

Query:     IdentificationMethod,QuerySigma,ShiftSigma,CandidateSetSize,SExistence
Alignment: IdentificationMethod,MatchSigma,ShiftSigma,MBar,OptimalConfigRotation,
           NonOptimalConfigRotation,OptimalComponentError,NonOptimalComponentError
Crown:     ..................................

The first argument is the location of the log file. We can infer the type of trial from the length of the header.

Usage: visualize-trial [log-file-1] [log-file-2] [log-file-3] [log-file-4] 
"""""

import matplotlib.pyplot as plt
import numpy as np
import os, csv, sys

# I know, I know, bad practice, blah... Global constants and iterables defined for the current working plot.
POINTS_PER_VARIATION = 1000

LABEL_LIST = iter([])

LEGEND_LIST = iter([])

TITLE_LIST = iter([])

BOUNDS_LIST = iter([])


def query_trial_plot(log_sets):
    """ For every log in log_sets (which hold sets of tuples), display a plot comparing each.

    The following plots are displayed: QuerySigma vs. P(SExistence)
                                       QuerySigma vs. CandidateSetSize
                                       ShiftSigma vs. P(SExistence) w/ Optimal QuerySigma (Before Saturation)

    :param log_sets: List of lists representing the contents of the log file.
    :return: None.
    """

    # Plot #1: Query Sigma vs. P(SExistence).
    plt.figure()
    for log in log_sets:
        # Visualize trials w/o noise here. Sort the
        d = [tuple for tuple in log if tuple[2] == '0']
        sorted(d, key=lambda x: x[1])
        q_count = list(set([float(x[1]) for x in d]))
        s_list = [[] for _ in q_count]
        for i in range(0, len(q_count)):
            for j in range(0, POINTS_PER_VARIATION):
                s_list[i].append(int(d[i * POINTS_PER_VARIATION + j][4]))
        plt.bar(np.arange(len(q_count)), [np.average(x) for x in s_list],
                yerr=[np.std(s) for s in s_list], align='center')

    # Plot #1: QuerySigma vs. SExistence. Ignore the shift_sigma trials.
    d = [x for x in log_sets if x[2] == '0']
    sorted(d, key=lambda x: x[1])
    q_count, q_plot_count = list(set([float(x[1]) for x in d])), []
    s_count, s_list = [0 for _ in q_count], [[] for _ in q_count]
    for i in range(0, len(q_count)):
        for j in range(0, POINTS_PER_VARIATION):
            s_count[i] += int(d[i * POINTS_PER_VARIATION + j][4])
            s_list[i].append(int(d[i * POINTS_PER_VARIATION + j][4]))
    q_plot_count = q_count

    # Figure is a bar chart. Plot the error bars and set the axis.
    plt.figure()
    plt.bar(np.arange(len(q_count)), [float(i) / POINTS_PER_VARIATION for i in s_count],
            yerr=[np.std(s) for s in s_list], align='center')
    ax = plt.gca()
    ax.set_ylim([0, 1])
    for i in range(1, len(q_count) - 1, 2):
        q_plot_count[i] = ''
    q_plot_count[len(q_count) - 1] = ''
    plt.xticks(np.arange(len(q_plot_count)), q_plot_count)
    plt.xlabel('Query Sigma'), plt.ylabel('P(Correct Star Set in Candidate Set)')
    plt.title(next(titles)), plt.tight_layout(), plt.show()

    # Plot #2: ShiftSigma vs. SExistence. Restrict QuerySigma to the smallest recorded and largest recorded.
    d_s = [x for x in log_sets if x[1] == log_sets[0][1]]
    d_m = [x for x in log_sets if x[1] == log_sets[-1][1]]
    sorted(d_s, key=lambda x: x[2]), sorted(d_m, key=lambda x: x[2])

    # Figure is a 2-graph bar chart. Plot the error bars and set the axis.
    plt.figure()
    for k, d in enumerate([d_s, d_m]):
        s_count, s_plot_count = list(set([float(x[2]) for x in log_sets])), []
        se_count, se_list = [0 for _ in s_count], [[] for _ in s_count]
        for i in range(0, len(s_count)):
            for j in range(0, POINTS_PER_VARIATION):
                se_count[i] += int(d[i * POINTS_PER_VARIATION + j][4])
                se_list[i].append(int(d[i * POINTS_PER_VARIATION + j][4]))
        s_plot_count = s_count

        plt.subplot(1, 2, k + 1)
        plt.bar(np.arange(len(s_count)), [float(i) / POINTS_PER_VARIATION for i in se_count],
                yerr=[np.std(s) for s in se_list])
        ax = plt.gca()
        ax.set_ylim([0, 1])
        for i in range(len(s_count) - 1):
            if not i % 3 == 0:
                s_plot_count[i] = ''
        s_plot_count[len(s_count) - 1] = ''
        plt.xticks(np.arange(len(s_plot_count)), s_plot_count)
        plt.xlabel('Shift Sigma'), plt.ylabel('P(Correct Star Set in Candidate Set)'), plt.title(next(titles))
    plt.tight_layout(), plt.show()


def alignment_trial_plot(log_tuples, titles):
    """ Plots displayed: MatchSigma vs. ||R_q - R_qe|| for min and max MBar, ShiftSigma vs. ||R_q - R_qe|| for min
    and max MatchSigma. The titles are not entered so this has to be performed manually.

    :param log_tuples: List of lists representing the contents of the log file.
    :param titles: Iterator of strings representing each plot's title (in order).
    :return: None.
    """
    POINTS_PER_VARIATION = 100  # Constant defined in alignment.h.

    # Plot #1: MatchSigma vs. ||R_q - R_qe||. Ignore the shift_sigma trials. Take the highest and lowest magnitude.
    d_s = [x for x in log_tuples if x[3] == log_tuples[0][3] and x[2] == '0']
    d_m = [x for x in log_tuples if x[3] == log_tuples[-1][3] and x[2] == '0']
    sorted(d_s, key=lambda x: x[1]), sorted(d_m, key=lambda x: x[1])

    # Figure is a 2-graph bar chart. Plot the error bars and set the axis.
    plt.figure()
    for k, d in enumerate([d_s, d_m]):
        m_count, m_plot_count = list(set([float(x[1]) for x in log_tuples])), []
        r_list = [[] for _ in m_count]
        for i in range(0, len(m_count)):
            for j in range(0, POINTS_PER_VARIATION):
                r_list[i].append(np.abs(float(d[i * POINTS_PER_VARIATION + j][6])))
        m_plot_count = m_count

        plt.subplot(1, 2, k + 1)
        plt.bar(np.arange(len(m_count)), [np.average(r) for r in r_list], yerr=[np.std(r) for r in r_list])
        ax = plt.gca()
        ax.set_ylim([0, max([np.average(r) for r in r_list])])
        for i in range(len(m_count) - 1):
            if not i % 3 == 0:
                m_plot_count[i] = ''
        m_plot_count[len(m_count) - 1] = ''
        plt.xticks(np.arange(len(m_plot_count)), m_plot_count)
        plt.xlabel('Match Sigma'), plt.ylabel('||Original Vector - Estimated Vector||'), plt.title(next(titles))
    plt.tight_layout(), plt.show()

    # Plot #2: ShiftSigma vs. ||R_q - R_qe||. Hold m_bar constant (=6). Take the highest and lowest match sigma.
    d_s = [x for x in log_tuples if x[1] == log_tuples[0][1] and x[3] == '6']
    d_m = [x for x in log_tuples if x[1] == log_tuples[-1][1] and x[3] == '6']
    sorted(d_s, key=lambda x: x[1]), sorted(d_m, key=lambda x: x[1])

    # Figure is a 2-graph bar chart. Plot the error bars and set the axis.
    plt.figure()
    for k, d in enumerate([d_s, d_m]):
        s_count, s_plot_count = list(set([float(x[2]) for x in log_tuples])), []
        r_list = [[] for _ in s_count]
        for i in range(0, len(s_count)):
            for j in range(0, POINTS_PER_VARIATION):
                r_list[i].append(np.abs(float(d[i * POINTS_PER_VARIATION + j][6])))
        s_plot_count = s_count

        plt.subplot(1, 2, k + 1)
        plt.bar(np.arange(len(s_count)), [np.average(r) for r in r_list], yerr=[np.std(r) for r in r_list])
        ax = plt.gca()
        ax.set_ylim([0, max([np.average(r) for r in r_list])])
        for i in range(len(s_count) - 1):
            if not i % 3 == 0:
                s_plot_count[i] = ''
        s_plot_count[len(s_count) - 1] = ''
        plt.xticks(np.arange(len(s_plot_count)), s_plot_count)
        plt.xlabel('Shift Sigma'), plt.ylabel('||Original Vector - Estimated Vector||'), plt.title(next(titles))
    plt.tight_layout(), plt.show()


def visualize_trial(log_1, log_2, log_3, log_4):
    """ Source function, used to display a plot of the given exactly four log files.

    :param log_1 Location of the first log file to use.
    :param log_2 Location of the second log file to use.
    :param log_3 Location of the third log file to use.
    :param log_4 Location of the fourth log file to use.
    :return: None.
    """
    with open(log_1, 'r') as f_1, open(log_2, 'r') as f_2, open(log_3, 'r') as f_3, open(log_4, 'r') as f_4:
        csv_1, csv_2, csv_3, csv_4 = list(map(lambda f: csv.reader(f, delimeter=','), [f_1, f_2, f_3, f_4]))

        # Parse our header, and the rest of the logs.
        attributes = list(map(lambda c: next(c), [csv_1, csv_2, csv_3, csv_4]))
        logs = list(map(lambda c: np.array(np.array([tuple for tuple in c]))))

        # This is a log file for the query trials.
        if len(attributes) == 5:
            query_trial_plot(logs)

        # This is a log file for the alignment trials.
        elif len(attributes) == 8:
            alignment_trial_plot(logs)

        # This is a log file for the crown trials.
        else:
            crown_trial_plot(logs)


# Perform the trials!
if len(sys.argv) is not 5:
    print('Usage: visualize-trial [log-file-1] [log-file-2] [log-file-3] [log-file-4]')
else:
    visualize_trial(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
