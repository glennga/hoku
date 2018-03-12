""""
This file is used to hold the plot parameter dictionary. This is to be used with plot_experiment.py, and determines 
how the plots are visually represented.
"""""

params = {
    'split-n': 5,  # Number of splits to perform for each member in X space.
    'query-plot': {
        'll': ['Angle', 'Dot', 'Sphere', 'Plane', 'Pyramid'],  # Legend list.

        # Sigma1 vs. CandidateSetSize plot parameters.
        's1css-yll': [0, 35],  # Y-axis limits.
        's1css-xal': r'$\sigma_1 = \sigma_2 = \ldots = \sigma_n$',  # X axis label.
        's1css-yal': r'$|$Candidate Set$|$',  # Y axis label.
        's1css-lxa': 1,  # Set the X axis as logarithmic.

        # Sigma1 vs. SExistence plot parameters.
        's1se-yll': [0, 1.05],  # Y-axis limits.
        's1se-xal': r'$\sigma_1 = \sigma_2 = \ldots = \sigma_n$',  # X axis label.
        's1se-yal': 'P(Correct Stars in Candidate Set)',  # Y axis label.
        's1se-lxa': 1,  # Set the X axis as logarithmic.

        # Shift Deviation vs. SExistence plot parameters.
        'sdse-yll': [0, 1.05],  # Y-axis limits.
        'sdse-xtl': [r'$0$', r'$10^{-4}$', r'$10^{-3}$', r'$10^{-2}$', r'$10^{-1}$'],  # X-axis tick labels.
        'sdse-xal': 'Deviation of Noise (degrees)',  # X-axis label.
        'sdse-yal': 'P(Correct Stars in Candidate Set)',  # Y-axis label.

        # Shift Deviation vs. CandidateSetSize plot parameters.
        'sdcss-yll': [0, 13],  # Y-axis limits.
        'sdcss-xtl': [r'$0$', r'$10^{-4}$', r'$10^{-3}$', r'$10^{-2}$',
                      r'$10^{-1}$'],  # X-axis tick labels.
        'sdcss-xal': 'Deviation of Noise (degrees)',  # X-axis label.
        'sdcss-yal': r'$|$Candidate Set$|$'  # Y-axis label.
    },

    'reduction-plot': {
        'll': ['Angle', 'Dot', 'Sphere', 'Plane', 'Pyramid', 'Composite'],  # Legend list.

    },

    'identification-plot': {
        'll': ['Angle', 'Dot', 'Sphere', 'Plane', 'Pyramid', 'Composite'],  # Legend list.

    }
}

