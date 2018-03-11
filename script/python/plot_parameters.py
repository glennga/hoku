""""
This file is used to hold the plot parameter dictionary. This is to be used with plot_experiment.py, and determines 
how the plots are visually represented.
"""""

params = {
    'split-n' : 10,  # Number of splits to perform for each member in X space.
    'query-plot' : {
        # Shift Deviation vs. SExistence plot parameters.
        'sdse-yll' : [0, 1.05],  # Y-axis limits.
        'sdse-xtl' : [r'$0', r'10^{-4}', r'10^{-3}', r'10^{-2}', r'10^{-1}'],  # X-axis tick labels.
        'sdse-xal' : 'Deviation of Noise (degrees)',  # X-axis label.
        'sdse-yal' : 'P(Correct Stars in Query)',  # Y-axis label.

        # Shift Deviation vs. CandidateSetSize plot parameters.
        'sdcss-yll' : [0, 1.3],  # Y-axis limits.
        'sdcss-xtl' : [r'$0', r'10^{-4}', r'10^{-3}', r'10^{-2}', r'10^{-1}'],  # X-axis tick labels.
        'sdcss-xal' : 'Deviation of Noise (degrees)',  # X-axis label.
        'sdcss-yal' : r'$|$Candidate Set$|$'  # Y-axis label.
    },

    'reduction-plot' : {

    },

    'identification-plot' : {

    }
}
