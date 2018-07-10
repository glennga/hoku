#!/bin/bash

echo $(dirname $0)

if [ "$#" -ne 1 ] || [ -d "$1" ]; then
    echo "Usage: check-progress.sh [lumberjack table]"
    exit 1
fi

sqlite3 $HOKU_PROJECT_PATH/data/lumberjack.db -header -column "SELECT COUNT(*),IdentificationMethod FROM $1 GROUP BY IdentificationMethod"
