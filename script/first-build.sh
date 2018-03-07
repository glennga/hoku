#!/bin/bash

echo "export "HOKU_PROJECT_PATH"=\"$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../"\" >> ~/.bashrc
echo HOKU_PROJECT_PATH=\"$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../\" >> ~/.profile
echo HOKU_PROJECT_PATH=\"$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../\" >> /etc/environment

HOKU_PROJECT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/.."
export HOKU_PROJECT_PATH

mkdir -m777 "$HOKU_PROJECT_PATH/build"
mkdir -m777 "$HOKU_PROJECT_PATH/bin"
mkdir -m777 "$HOKU_PROJECT_PATH/test/bin"
mkdir -m777 "$HOKU_PROJECT_PATH/doc/build"
