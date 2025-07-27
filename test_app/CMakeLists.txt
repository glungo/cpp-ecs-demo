cmake_minimum_required(VERSION 3.14)
project(TestApp VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(COMPILE_WARNING_AS_ERROR ON)
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
