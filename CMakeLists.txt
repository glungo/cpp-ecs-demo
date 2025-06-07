cmake_minimum_required(VERSION 3.14)
project(EntityComponentSystem VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard globally
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable testing at the root level
enable_testing()

# Define build options
option(BUILD_TESTS "Build tests" ON)

# Add the engine library first
add_subdirectory(engine)

# Then add the test_app which will use the engine
add_subdirectory(test_app)