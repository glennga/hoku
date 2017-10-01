""""
This file is used to produce plots to show the relationship between various parameters, and the comparison count & 
number of matches found.

We assume the following for each log file passed:
- Attributes are as follows: SetNumber,InputSize,IdentificationSize,MatchesFound,...,ComparisonCount
- The attributes in '...' are the areas of interest. We will plot these.
- The SetNumbers here correspond to the current BENCH table in Nibble.

The first argument is the log file to interpret. The second argument is the location to store the images produced.

Usage: visualize-trial [log-file] [location-of-images]
"""""

import matplotlib.pyplot as plt
import sqlite3 as sql
import numpy as np
import os, csv, sys


def plot_att_cc_mf(attributes, focus):
    """ For every attribute given, record a scatter plot of the attribute vs. the comparison count, and the
    attribute vs. the matches found.

    :param attributes: The header of the given log file.
    :param focus: All tuples from log file to derive plot from.
    :return: None.
    """
    for i in range(4, len(attributes) - 1):
        att, cc, mf = list(zip(*focus))[i], list(zip(*focus))[len(attributes) - 1], list(zip(*focus))[3]

        # Convert each list into floats. Zip the attributes with comparison count and matches found.
        att, cc, mf = list(map(lambda ell: list(map(float, ell)), [att, cc, mf]))
        att_cc, att_mf = list(zip(att, cc)), list(zip(att, mf))

        # Sort the given attribute into bins.
        cc_bins, mf_bins = [[[] for k in set(att)] for _ in range(2)]
        for k, a in enumerate(set(att)):
            cc_bins[k] = [j[1] for j in att_cc if j[0] == a]
            mf_bins[k] = [j[1] for j in att_mf if j[0] == a]

        # Determine sigma for error bars.
        cc_sigma = [np.std(cc_bins[k]) for k in range(len(set(att)))]
        mf_sigma = [np.std(mf_bins[k]) for k in range(len(set(att)))]

        # Determine mu to plot, as well as our attribute as a float.
        cc_mu = [np.mean(cc_bins[k]) for k in range(len(set(att)))]
        mf_mu = [np.mean(mf_bins[k]) for k in range(len(set(att)))]
        att_to_plot = [x for x in set(att)]

        # Plot the bins as a function of comparison count and matches found.
        plt.clf()
        for k, mu_sigma in enumerate([[cc_mu, cc_sigma, cc], [mf_mu, mf_sigma, mf]]):
            plt.subplot(1, 2, k + 1)
            plt.scatter(att_to_plot, mu_sigma[0])
            plt.errorbar(att_to_plot, mu_sigma[0], yerr=mu_sigma[1], linestyle='None', capsize=10)
            plt.axis([min(att) * 0.8, max(att) * 1.1, -max(mu_sigma[2]) * 1.1, max(mu_sigma[2]) * 1.1])
            plt.xlabel(attributes[i]), plt.ylabel(attributes[len(attributes) - 1 if k == 0 else 3])
            plt.title(attributes[i] + ' vs. ' + attributes[len(attributes) - 1 if k == 0 else 3])
        plt.show(block=True)


def grab_focus(cur, log_file, type):
    """ This function is meant to be run in order of: clean -> extra -> remove -> shift.

    :param cur: Connection to Nibble database with BENCH table.
    :param log_file: CSV reader that holds tuples to parse. Is an iterator, so requires the above order.
    :param type: Type of tuple to pull. Domain is ['clean', 'extra', 'remove', 'shift'].
    :return: List of tuples representing the current data.
    """
    focus = []

    if type == 'clean':
        [focus.append(next(log_file)) for _ in range(0, cur.execute('SELECT MAX(set_n) '
                                                                    'FROM BENCH '
                                                                    'WHERE e=0 AND r=0 AND s=0').fetchone()[0])]
    elif type == 'extra':
        [focus.append(next(log_file)) for _ in range(0, cur.execute('SELECT COUNT(*) '
                                                                    'FROM BENCH '
                                                                    'WHERE e!=0').fetchone()[0])]
    elif type == 'remove':
        [focus.append(next(log_file)) for _ in range(0, cur.execute('SELECT COUNT(*) '
                                                                    'FROM BENCH '
                                                                    'WHERE r!=0').fetchone()[0])]
    elif type == 'shift':
        [focus.append(next(log_file)) for _ in range(0, cur.execute('SELECT COUNT(*) '
                                                                    'FROM BENCH '
                                                                    'WHERE s!=0').fetchone()[0])]
    else:
        assert False

    return focus


# Open our file, and our connection to Nibble.
with open(sys.argv[1], 'r') as f:
    log_file = csv.reader(f, delimiter=',')
    con = sql.connect(os.environ['HOKU_PROJECT_PATH'] + '/data/nibble.db')
    cur = con.cursor()

    # Parse our header. Note that the first argument to count is indexed at 4.
    attributes = next(log_file)

    # Record our clean data-set. Generate plots for these.
    focus_clean = grab_focus(cur, log_file, 'clean')
    # plot_att_cc_mf(attributes, focus_clean)

    # Record our extra error data-set. Generate plots for these.
    focus_extra = grab_focus(cur, log_file, 'extra')
    # plot_att_es(attributes, focus_extra)

    # Record our remove error data-set. Generate plots for these.
    focus_remove = grab_focus(cur, log_file, 'remove')
    # plot_att_rs_rs(attributes, focus_remove)

    # Record our shift error data-set. Generate plots for these.
    focus_shift = grab_focus(cur, log_file, 'shift')
    # plot_att_ss_ss(attributes, focus_shift)
