#!/bin/bash

if [ $# -eq 0 ]
then
    rm -r "$HOKU_PROJECT_PATH/data/logs/*.csv"
else
    rm -r "$HOKU_PROJECT_PATH/data/logs/$1/*.csv"
fi
