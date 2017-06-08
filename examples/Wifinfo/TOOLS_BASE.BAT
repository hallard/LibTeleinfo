@ECHO OFF

SET BIN=arduino_debug
SET ACTION=--%1
SET MAIN_INO=Wifinfo.ino
SET OPTIONS=-v
SET PORT=--port %2
SET RESULTS=%3

%BIN% %ACTION% %MAIN_INO% %OPTIONS% %PORT% >> %RESULTS%

IF ERRORLEVEL 0 ECHO Success
IF ERRORLEVEL 1 ECHO %ACTION% failed
IF ERRORLEVEL 2 ECHO Sketch not found
IF ERRORLEVEL 3 ECHO Invalid (argument for) commandline option
IF ERRORLEVEL 4 ECHO Preference passed to --get-pref does not exist