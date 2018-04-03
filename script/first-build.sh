#!/bin/bash

# TODO: Verify this script.
echo "export "HOKU_PROJECT_PATH"=\"$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../"\" >> ~/.bashrc
HOKU_PROJECT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/.."
mkdir -m700 "$HOKU_PROJECT_PATH/build"
mkdir -m700 "$HOKU_PROJECT_PATH/bin"
