#!/bin/bash

echo "export "HOKU_PROJECT_PATH"=\"$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../../"\" >> ~/.bashrc
echo HOKU_PROJECT_PATH=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../../ >> ~/.profile
echo HOKU_PROJECT_PATH=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../../ >> /etc/environment

set HOKU_PROJECT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../.."
mkdir "$HOKU_PROJECT_PATH/data/logs"
mkdir "$HOKU_PROJECT_PATH/data/logs/test"
mkdir "$HOKU_PROJECT_PATH/data/logs/tmp"
mkdir "$HOKU_PROJECT_PATH/data/logs/trial"
mkdir "$HOKU_PROJECT_PATH/build"
mkdir "$HOKU_PROJECT_PATH/bin"
mkdir "$HOKU_PROJECT_PATH/test/bin"
mkdir "$HOKU_PROJECT_PATH/doc/build"
