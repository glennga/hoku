#!/bin/bash

# Ensure that we have only 3 arguments passed.
if [[ "$#" -ne 3 ]]; then
    echo "Usage: find-sigma.sh [angle / dot / sphere / plane / pyramid / composite] [row blocks] [max spawn] "
    exit 1
fi

# Ensure that nibble.db exists before proceeding.
if [[ ! -f ${HOKU_PROJECT_PATH}/data/nibble.db ]]; then
    echo "'nibble.db' not found. Run the script 'script/generate-all-n.sh'."
    exit 1
fi

# Ensure that our method is valid.
if [[ "$1" != "angle" ]] && [[ "$1" != "dot" ]] && [[ "$1" != "sphere" ]] && [[ "$1" != "plane" ]] && \
   [[ "$1" != "composite" ]]; then
    echo "Method not valid. Use from: [angle / dot / sphere / plane / pyramid / composite]"
    exit 1
fi

# Ensure that the row block number is valid.
if ! [[ $2 =~ '^[0-9]+$' ]] && (( $2 <= 0 )); then
    echo "Blocks specified is not a positive integer."
    exit 1
fi

# Ensure that the maximum spawned processes number is valid.
if ! [[ $3 =~ '^[0-9]+$' ]] && (( $3 <= 0 )); then
    echo "Maximum process spawn number is not a positive integer."
    exit 1
fi

# We generate a random temporary folder to store our results before we assess the results of each.
TMP_RESULTS_DIR=${HOKU_PROJECT_PATH}/data/$(openssl rand -base64 12)
mkdir ${TMP_RESULTS_DIR}

# Determine our table name in nibble.db and the number of rows associated with this table.
TABLE_NAME=$(echo $1 | awk '{print toupper($0)}')
ROWS=$(sqlite3 ${HOKU_PROJECT_PATH}/data/nibble.db "SELECT COUNT(*) \
                                                    FROM ${TABLE_NAME}_20")

# Determine the first part of our query: the results we want to find.
if [[ "$1" = "angle" ]] || [[ "$1" = "pyramid" ]]; then
    QUERY_PART_ONE="SELECT MIN(ABS(D_THETA))"

elif [[ "$1" = "sphere" ]] || [[ "$1" = "plane" ]] || [[ "$1" = "composite" ]]; then
    QUERY_PART_ONE="SELECT MIN(ABS(D_A)), \
                    MIN(ABS(D_I))"

else
    QUERY_PART_ONE="SELECT MIN(ABS(D_THETA_1)), \
                    MIN(ABS(D_THETA_2)), \
                    MIN(ABS(D_PHI))"
fi

# Determine the second part of our query: the cartesian product of the table with itself, how do we find our result?
if [[ "$1" = "angle" ]] || [[ "$1" = "pyramid" ]]; then
    QUERY_PART_TWO="SELECT A.theta - B.theta AS D_THETA"

elif [[ "$1" = "sphere" ]] || [[ "$1" = "plane" ]] || [[ "$1" = "composite" ]]; then
    QUERY_PART_TWO="SELECT A.a - B.a AS D_A, \
                    A.i - B.i AS D_I"

else
    QUERY_PART_TWO="SELECT A.theta_1 - B.theta_1 AS D_THETA_1, \
                    A.theta_2 - B.theta_2 AS D_THETA_2, \
                    A.phi - B.phi AS D_PHI"
fi

# Determine the third part of our query: filter out any zero results we get.
if [[ "$1" = "angle" ]] || [[ "$1" = "pyramid" ]]; then
    QUERY_PART_THREE="D_THETA > 0"

elif [[ "$1" = "sphere" ]] || [[ "$1" = "plane" ]] || [[ "$1" = "composite" ]]; then
    QUERY_PART_THREE="D_A > 0 AND \
                      D_I > 0"

else
    QUERY_PART_THREE="D_THETA_1 > 0 AND \
                      D_THETA_2 > 0 AND \
                      D_PHI > 0"
fi

# Divide our query and perform accordingly. We output the results of each query to the temp directory.
DIVIDED_QUERY_ROWS=$(($ROWS / $2))
NUMBER_SPAWNED_PROCESSES=0
for r in `seq 1 ${DIVIDED_QUERY_ROWS}`; do
    LOWER_QUERY_BOUND=$(($(($r - 1)) * $2))
    UPPER_QUERY_BOUND=$(($r * $2))
    sqlite3 ${HOKU_PROJECT_PATH}/data/nibble.db "$QUERY_PART_ONE \
                                                 FROM ( \
                                                    $QUERY_PART_TWO \
                                                    FROM ${TABLE_NAME}_20 A, ${TABLE_NAME}_20 B
                                                    WHERE A.ROWID > $LOWER_QUERY_BOUND AND \
                                                    A.ROWID < $UPPER_QUERY_BOUND AND \
                                                    B.ROWID > $LOWER_QUERY_BOUND AND \
                                                    B.ROWID < $UPPER_QUERY_BOUND AND \
                                                    A.ROWID <> B.ROWID
                                                 )
                                                 WHERE $QUERY_PART_THREE;" >> ${TMP_RESULTS_DIR}/MIN-$1-${r}.result &

    # Ensure that we never spawn more than $3 processes.
    NUMBER_SPAWNED_PROCESSES=$((${NUMBER_SPAWNED_PROCESSES} + 1))
    if (( $NUMBER_SPAWNED_PROCESSES > $3 )); then
        wait
        NUMBER_SPAWNED_PROCESSES=0
    fi
done

# Wait for all spawned processes to finish.
wait

# Concatenate these results into one main file.
cat ${TMP_RESULTS_DIR}/* >> ${TMP_RESULTS_DIR}/MIN-$1-sum.result

# Determine the smallest values from here.
echo "sigma_1: $(cut -f1 -d"," ${TMP_RESULTS_DIR}/MIN-$1-sum.result | sort -n | head -1)"
echo "=============================="

if [[ "$1" = "sphere" ]] || [[ "$1" = "plane" ]] || [[ "$1" = "composite" ]] || [[ "$1" = "dot" ]]; then
    echo "sigma_2: $(cut -f2 -d"," ${TMP_RESULTS_DIR}/MIN-$1-sum.result | sort -n | head -1)"
    echo "=============================="
fi

if [[ "$1" = "dot" ]]; then
    echo "sigma_3: $(cut -f2 -d"," ${TMP_RESULTS_DIR}/MIN-$1-sum.result | sort -n | head -1)"
    echo "=============================="
fi

