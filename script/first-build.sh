#!/bin/bash

# TODO: Verify this script.
if ! grep -Fq HOKU_PROJECT_PATH ~/.bashrc
then
    echo Updating '.bashrc' with the HOKU_PROJECT_PATH environment variable.
    echo "export "HOKU_PROJECT_PATH"=\"$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../"\" >> ~/.bashrc
    source ~/.bashrc
else
    echo HOKU_PROJECT_PATH found in '.bashrc'.
fi

echo Creating 'build' and 'bin' directories.
HOKU_PROJECT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/.."
mkdir -p -m700 "$HOKU_PROJECT_PATH/build"
mkdir -p -m700 "$HOKU_PROJECT_PATH/bin"
