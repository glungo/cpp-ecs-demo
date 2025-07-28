#!/bin/bash

#clear console
clear

# Exit on any error
set -e

# Update submodules
git submodule update --init --recursive
# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure CMake using the root directory with WebGPU Dawn backend
cmake ..

# Build everything (including running tests)
cmake --build .