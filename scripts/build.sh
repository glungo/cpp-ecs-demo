#!/bin/bash

# Exit on any error
set -e

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure CMake using test_app's CMakeLists.txt
cmake ../test_app

# Build everything (including running tests)
cmake --build . 