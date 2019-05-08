#!/bin/bash

HOKU_PROJECT_PATH="$(dirname "$0")/../"

# Ensure that GenerateN exists before proceeding.
if [[ ! -f $HOKU_PROJECT_PATH/bin/GenerateN ]]; then
    echo "'GenerateN' not found. Build the file 'src/generate-n.cpp'."
    exit 1
fi

echo "Building HIP table."
$HOKU_PROJECT_PATH/bin/GenerateN hip

echo "Building ANGLE table."
$HOKU_PROJECT_PATH/bin/GenerateN angle

echo "Building SPHERE table."
$HOKU_PROJECT_PATH/bin/GenerateN sphere

echo "Building PLANE table."
$HOKU_PROJECT_PATH/bin/GenerateN plane

echo "Building PYRAMID table."
$HOKU_PROJECT_PATH/bin/GenerateN pyramid

echo "Building COMPOSITE table."
$HOKU_PROJECT_PATH/bin/GenerateN composite
