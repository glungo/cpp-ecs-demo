cmake_minimum_required(VERSION 3.14)
project(EntityComponentSystem VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_TESTS ON)

# Enable testing at the top level
enable_testing()

# Add the entities library
add_subdirectory(../entities entities)

# Source files for main executable
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

# Create the main executable
add_executable(test_app ${SOURCES})

# Link the entities library to the main executable
target_link_libraries(test_app PRIVATE entities)

target_include_directories(test_app PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

# Run tests for the entities library
if (BUILD_TESTS)
    # Get list of all test targets from entities
    get_property(TEST_TARGETS GLOBAL PROPERTY ENTITIES_TEST_TARGETS)
    
    # Add a custom target that builds and runs tests
    add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS ${TEST_TARGETS}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building and running tests..."
    )

    # Make tests run as part of the default build
    add_dependencies(test_app check)
endif()
