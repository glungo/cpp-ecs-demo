@echo off
rem Windows batch file for building cpp-ecs-demo
rem Usage: build-windows.bat [RUN_TESTS]
rem   RUN_TESTS: TRUE to build and run tests (default), FALSE to skip building and running tests

setlocal EnableDelayedExpansion

rem Default option - run tests
set RUN_TESTS=FALSE

rem Parse command line argument
if not "%~1"=="" (
    set RUN_TESTS=%~1
)

rem Set BUILD_TESTS based on RUN_TESTS
if /i "%RUN_TESTS%"=="TRUE" (
    set BUILD_TESTS=ON
) else (
    set BUILD_TESTS=OFF
)

echo Build configuration:
echo - RUN_TESTS=%RUN_TESTS%
echo - BUILD_TESTS=%BUILD_TESTS%

rem Clear console
cls
git submodule update --init --recursive
rem Create build directory if it doesn't exist
if not exist build mkdir build

rem Delete existing CMakeCache.txt to ensure clean configuration
if exist build\CMakeCache.txt (
    echo Cleaning previous CMake configuration...
    del build\CMakeCache.txt
)

cd build

rem Configure CMake using Visual Studio generator
echo Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -DBUILD_TESTS=%BUILD_TESTS%

rem Build everything in Debug configuration
echo Building project...
cmake --build . --config Debug

rem Run tests with the Debug configuration if enabled
if /i "%RUN_TESTS%"=="TRUE" (
    echo Running tests...
    ctest -C Debug --output-on-failure --verbose
) else (
    echo Skipping tests.
)

rem Return to original directory
cd ..

echo Build and tests completed successfully