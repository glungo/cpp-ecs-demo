cmake_minimum_required(VERSION 3.14)
project(Engine VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(COMPILE_WARNING_AS_ERROR ON)


# Add the 'glfw' directory, which contains the definition of a 'glfw' target
add_subdirectory(glfw)

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
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link with entities library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../entities entities)
target_link_libraries(engine PUBLIC entities)

target_include_directories(engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../entities/include
)
# Add the 'glfw' target as a dependency of our App
target_link_libraries(engine PRIVATE glfw)

# Enable testing at engine level
if(BUILD_TESTS)
    enable_testing()
endif()