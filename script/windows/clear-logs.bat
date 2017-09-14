if "%~1"=="" del /s "%HOKU_PROJECT_PATH%\data\logs\*"
if not "%~1" == "" del /s "%HOKU_PROJECT_PATH%\data\logs\%~1\*"
