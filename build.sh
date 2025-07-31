#!/bin/bash
# Usage: ./build.sh [RUN_TESTS]
#   RUN_TESTS: TRUE to build and run tests (default), FALSE to skip building and running tests

# Default option - run tests
RUN_TESTS=FALSE

# Parse command line argument
if [ $# -eq 1 ]; then
    RUN_TESTS=$1
fi

# Set BUILD_TESTS based on RUN_TESTS
if [[ "${RUN_TESTS}" == "TRUE" ]]; then
    BUILD_TESTS=ON
else
    BUILD_TESTS=OFF
fi

echo "Build configuration:"
echo "- RUN_TESTS=${RUN_TESTS}"
echo "- BUILD_TESTS=${BUILD_TESTS}"

# Clear console
clear

# Exit on any error
set -e

# Update submodules
git submodule update --init --recursive
# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure CMake
echo "Configuring CMake..."
cmake .. -DBUILD_TESTS=${BUILD_TESTS}

# Build everything
echo "Building project..."
cmake --build .

# Run tests if enabled
if [[ "${RUN_TESTS}" == "TRUE" ]]; then
    echo "Running tests..."
    ctest --output-on-failure --verbose
else
    echo "Skipping tests."
fi

# Return to original directory
cd ..