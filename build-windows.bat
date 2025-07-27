@echo off
rem Windows batch file for building cpp-ecs-demo

rem Clear console
cls

rem Create build directory if it doesn't exist
if not exist build mkdir build

rem Delete existing CMakeCache.txt to ensure clean configuration
if exist build\CMakeCache.txt (
    echo Cleaning previous CMake configuration...
    del build\CMakeCache.txt
)

cd build

rem Configure CMake using Visual Studio generator with the new test structure
echo Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -DBUILD_TESTS=ON

rem Build everything in Debug configuration
echo Building project...
cmake --build . --config Debug

rem Run tests with the Debug configuration
echo Running tests...
ctest -C Debug --output-on-failure --verbose

rem Return to original directory
cd ..

echo Build and tests completed successfully