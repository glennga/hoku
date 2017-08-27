@ECHO OFF

REM Export our project path environment variable using our current working directory.
SETX HOKU_PROJECT_PATH "%~dp0.."

REM Create our log, build, and binary folders.
MKDIR "%HOKU_PROJECT_PATH%\data\logs"
MKDIR "%HOKU_PROJECT_PATH%\data\logs\test"
MKDIR "%HOKU_PROJECT_PATH%\data\logs\tmp"
MKDIR "%HOKU_PROJECT_PATH%\data\logs\trial"
MKDIR "%HOKU_PROJECT_PATH%\build"
MKDIR "%HOKU_PROJECT_PATH%\bin"
