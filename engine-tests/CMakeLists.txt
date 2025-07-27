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
    ${CMAKE_SOURCE_DIR}/engine/include
    ${CMAKE_SOURCE_DIR}/entities/include
    ${CMAKE_SOURCE_DIR}/entities # This is needed for the "include/entity.h" paths
)

# Source files for the test suite
set(TEST_SOURCES
    src/test_main.cpp
    src/engine/engine_test_wrapper.cpp
    src/entities/entity_test_wrappers.cpp
)

# Create the combined test executable
add_executable(engine_tests ${TEST_SOURCES})
target_link_libraries(engine_tests PRIVATE engine)
target_include_directories(engine_tests PRIVATE ${COMMON_INCLUDE_DIRS})

# Add tests that can be run through CTest
add_test(NAME engine_basic COMMAND engine_tests "Engine Basic Tests")
add_test(NAME entity_creation COMMAND engine_tests "Entity Creation")
add_test(NAME component_creation COMMAND engine_tests "Component Creation")
add_test(NAME pool_creation COMMAND engine_tests "Pool Creation")
add_test(NAME archetype_creation COMMAND engine_tests "Archetype Creation")
add_test(NAME archetype_component_access COMMAND engine_tests "Archetype Component Access")
add_test(NAME archetype_entity_iteration COMMAND engine_tests "Archetype Entity Iteration")
add_test(NAME basic_entity_creation COMMAND engine_tests "Basic Entity Creation")
add_test(NAME entity_pool_overflow COMMAND engine_tests "Entity Pool Overflow")
add_test(NAME entity_reuse COMMAND engine_tests "Entity Reuse")
add_test(NAME job_create COMMAND engine_tests "Job Create")
add_test(NAME job_parallel_execution COMMAND engine_tests "Job Parallel Execution")
add_test(NAME job_cache_refresh COMMAND engine_tests "Job Cache Refresh")
add_test(NAME job_multiple COMMAND engine_tests "Job Multiple")
add_test(NAME job_scheduler COMMAND engine_tests "Job Scheduler")
# System test is currently disabled
# add_test(NAME system COMMAND engine_tests "System Tests")

# Set environment variables for all tests
set_tests_properties(
    engine_basic
    entity_creation
    component_creation
    pool_creation
    archetype_creation
    archetype_component_access
    archetype_entity_iteration
    basic_entity_creation
    entity_pool_overflow
    entity_reuse
    job_create
    job_parallel_execution
    job_cache_refresh
    job_multiple
    job_scheduler
    PROPERTIES ENVIRONMENT "CTEST_OUTPUT_ON_FAILURE=1"
)

# Add an option to run all tests at once
add_test(NAME all_tests COMMAND engine_tests)
set_tests_properties(all_tests PROPERTIES ENVIRONMENT "CTEST_OUTPUT_ON_FAILURE=1")
