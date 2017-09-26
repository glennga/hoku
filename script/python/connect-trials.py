""""
This file is used to concatenate two trial logs together based on their set_n. 

The first argument should be the base-file, which holds trials starting from set_n = 1. The second argument is 
file-to-append. All trial data having a set_n = k are deleted, where k is the starting set_n of the file-to-append. 
There exists an optional third argument that will delete the second file if specified.

Trial files are assumed to be formatted with following attributes:
SetNumber,InputSize,IdentificationSize,MatchesFound,...,ComparisionCount\n

Usage: connect-trials [base-file] [file-to-append] [d]
"""""

import sys, csv, os

# The temporary file exists in the tmp folder. Clear this before proceeding (if exists).
tmp = os.environ['HOKU_PROJECT_PATH'] + '/data/logs/tmp/connect_tmp.csv'
try:
    os.remove(tmp)
except OSError:
    pass

# Open the base-file, the file to append, and a temporary file to write to.
with open(sys.argv[1], 'r+') as base_f, open(sys.argv[2], 'r+') as append_f, open(tmp, 'w+') as connect_f:
    base_ell, append_ell = list(csv.reader(base_f)), list(csv.reader(append_f))
    connect_w = csv.writer(connect_f, delimiter=',', lineterminator='\n')

    # Record the header (:
    connect_w.writerow(base_ell[0])

    # Remove all instances in base who's set_n is equal to set_n_app.
    set_n_app = int(append_ell[1][0])
    [(int(entry[0]) < set_n_app) and connect_w.writerow(entry) for entry in base_ell[1:]]

    # Write all rows in append to the temp file (ignoring the header).
    connect_w.writerows(append_ell[1:])

# Replace the base file with the temporary file.
os.remove(sys.argv[1])
os.rename(tmp, sys.argv[1])

# If desired, remove the append file.
if len(sys.argv) == 4 and sys.argv[3] is 'd':
    os.remove(sys.argv[2])

