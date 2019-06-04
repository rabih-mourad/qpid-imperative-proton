@ECHO OFF
setlocal

set BUILD_DIR=build-dir

if exist %BUILD_DIR% rd /s /q %BUILD_DIR%
mkdir %BUILD_DIR%
cd %BUILD_DIR%
cmake .. -G "Visual Studio 15 2017 Win64"


endlocal