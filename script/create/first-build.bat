@echo off

echo Setting HOKU_PROJECT_PATH enviroment variable.
set HOKU_PROJECT_PATH=%~dp0..

echo Creating 'build' and 'bin' directories.
mkdir "%HOKU_PROJECT_PATH%"\build
mkdir "%HOKU_PROJECT_PATH%"\bin
