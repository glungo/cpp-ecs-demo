#!/bin/bash

#clear console
clear

# Exit on any error
set -e

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure CMake using the root directory instead of test_app
cmake ..

# Build everything (including running tests)
cmake --build .