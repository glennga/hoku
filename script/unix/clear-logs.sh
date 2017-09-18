#!/bin/bash

if [ $# -eq 0 ]
then
    rm -r "$HOKU_PROJECT_PATH/data/logs/*"
else
    rm -r "$HOKU_PROJECT_PATH/data/logs/$1/*"
fi
