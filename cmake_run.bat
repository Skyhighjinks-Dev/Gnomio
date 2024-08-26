@echo off
setlocal

set BATCH_DIR=%~dp0
set BATCH_DIR=%BATCH_DIR:~0,-1%
echo Current batch file directory: %BATCH_DIR%

cd "%BATCH_DIR%"

set BUILD_PATH=%BATCH_DIR%\build

echo Build directory: %BUILD_PATH%

:: Check if the directory exists and remove it
if exist "%BUILD_PATH%" (
    rmdir /s /q "%BUILD_PATH%"
    echo Deleted %BUILD_PATH%
)
mkdir "%BUILD_PATH%"
echo Created %BUILD_PATH%

cd "%BUILD_PATH%"
echo Creating CMAKE config build
"%CMAKE_PATH%\cmake.exe" -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="C:/Users/amdro/vcpkg/scripts/buildsystems/vcpkg.cmake" "%BATCH_DIR%"
echo Finished.
echo Creating CMAKE project build
"%CMAKE_PATH%\cmake.exe" --build .
echo Finished

endlocal
