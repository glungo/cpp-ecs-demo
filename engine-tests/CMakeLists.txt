cmake_minimum_required(VERSION 3.14)
project(EngineTests VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable testing
enable_testing()

# Common include directories for all tests
set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/entities/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/entities
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/entities/include/utils
)

# Store test executables to add as dependencies
set(ALL_TEST_EXECUTABLES "")

# Function to create a test executable
function(add_engine_test TEST_NAME TEST_SOURCE)
    get_filename_component(TEST_NAME_ONLY ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME_ONLY} ${TEST_SOURCE})
    set_target_properties(${TEST_NAME_ONLY} PROPERTIES 
        FOLDER "Tests"  # Group tests in a separate folder in the IDE
    )
    target_link_libraries(${TEST_NAME_ONLY} PRIVATE engine)
    target_include_directories(${TEST_NAME_ONLY} PRIVATE ${COMMON_INCLUDE_DIRS})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME_ONLY})
    set_tests_properties(${TEST_NAME} PROPERTIES ENVIRONMENT "CTEST_OUTPUT_ON_FAILURE=1")
    
    # Add this test to our list of all test executables
    list(APPEND ALL_TEST_EXECUTABLES ${TEST_NAME_ONLY})
    set(ALL_TEST_EXECUTABLES ${ALL_TEST_EXECUTABLES} PARENT_SCOPE)
endfunction()

# Find all test source files in the engine directory
file(GLOB ENGINE_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/test_*.cpp")
foreach(TEST_SOURCE ${ENGINE_TEST_SOURCES})
    get_filename_component(TEST_NAME_WE ${TEST_SOURCE} NAME_WE)
    string(REPLACE "test_" "" TEST_NAME ${TEST_NAME_WE})
    add_engine_test(engine_${TEST_NAME} ${TEST_SOURCE})
endforeach()

# Find all test source files in the entities directory
file(GLOB ENTITIES_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/test_*.cpp")
foreach(TEST_SOURCE ${ENTITIES_TEST_SOURCES})
    get_filename_component(TEST_NAME_WE ${TEST_SOURCE} NAME_WE)
    string(REPLACE "test_" "" TEST_NAME ${TEST_NAME_WE})
    add_engine_test(entity_${TEST_NAME} ${TEST_SOURCE})
endforeach()

# Debugging - Print all test executables
message(STATUS "Test executables: ${ALL_TEST_EXECUTABLES}")

# Create a custom target just to build all tests without running them
add_custom_target(build_all_tests
    DEPENDS ${ALL_TEST_EXECUTABLES}
    COMMENT "Building all test executables"
)

# Create a custom target to build and run all tests
add_custom_target(build_and_run_tests
    COMMAND ${CMAKE_CTEST_COMMAND}
    DEPENDS ${ALL_TEST_EXECUTABLES}
    COMMENT "Building and running all tests"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
)