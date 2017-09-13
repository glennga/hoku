if "%~1"=="" del /s "%HOKU_PROJECT_PATH%\data\logs\*.csv"
if not "%~1" == "" del /s "%HOKU_PROJECT_PATH%\data\logs\%~1\*.csv"
