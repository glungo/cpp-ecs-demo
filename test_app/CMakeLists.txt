cmake_minimum_required(VERSION 3.14)
project(TestApp VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_definitions(ENTITIES_DEBUG)

# Source files for main executable
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

# Create the main executable
add_executable(test_app ${SOURCES})

# Link with the engine library (which already includes entities)
target_link_libraries(test_app PRIVATE engine)

target_include_directories(test_app PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Run tests for both entities and engine libraries
if (BUILD_TESTS)
    # Get list of all test targets
    get_property(ENTITIES_TEST_TARGETS GLOBAL PROPERTY ENTITIES_TEST_TARGETS)
    get_property(ENGINE_TEST_TARGETS GLOBAL PROPERTY ENGINE_TEST_TARGETS)
    set(TEST_TARGETS ${ENTITIES_TEST_TARGETS} ${ENGINE_TEST_TARGETS})
    
    set(address_sanitizer_flag "-fsanitize=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${address_sanitizer_flag}")
    
    # Configure CTest to show output
    set(CTEST_OUTPUT_ON_FAILURE ON)
    set(CTEST_VERBOSE ON)
    
    # Add a custom target that builds and runs tests with output
    add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
        DEPENDS ${TEST_TARGETS}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building and running tests with output..."
    )

    # Make tests run as part of the default build
    add_dependencies(test_app check)
endif()
