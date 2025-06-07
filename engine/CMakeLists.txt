cmake_minimum_required(VERSION 3.14)
project(Engine VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(COMPILE_WARNING_AS_ERROR ON)

# Engine library source files
set(ENGINE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine.cpp
)

# Create engine static library
add_library(engine STATIC ${ENGINE_SOURCES})

# Set include directories
target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/..  # Add parent directory to include path
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link with entities library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../entities entities)
target_link_libraries(engine PUBLIC entities)

# Enable testing
if(BUILD_TESTS)
    enable_testing()
    
    # Add engine tests
    add_executable(test_engine_basic ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_engine_basic.cpp)
    target_link_libraries(test_engine_basic PRIVATE engine)
    add_test(NAME TestEngineBasic COMMAND test_engine_basic)
    
    # Store the test target in a list for the parent CMake to use
    set(ENGINE_TEST_TARGETS test_engine_basic)
    set_property(GLOBAL PROPERTY ENGINE_TEST_TARGETS ${ENGINE_TEST_TARGETS})
endif()