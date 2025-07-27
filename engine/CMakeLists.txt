cmake_minimum_required(VERSION 3.14)
project(Engine VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(COMPILE_WARNING_AS_ERROR ON)

# Create the engine library target
add_library(engine)

# Add the 'glfw' directory, which contains the definition of a 'glfw' target
add_subdirectory(glfw)

file(GLOB_RECURSE ENGINE_ENTITIES_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/entities/include/*.h"
)


file(GLOB_RECURSE ENGINE_ENTITIES_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/entities/src/*.cpp"
)

file(GLOB_RECURSE ENGINE_CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/core/src/*.cpp"
)

file(GLOB_RECURSE ENGINE_CORE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/core/include/*.h"
)

# Add source files to the engine target
target_sources(engine
    PRIVATE 
        ${ENGINE_CORE_SOURCES}
        ${ENGINE_ENTITIES_SOURCES}
    PUBLIC
        ${ENGINE_CORE_HEADERS}
        ${ENGINE_ENTITIES_HEADERS}
)

# Add include directories for engine core
target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/core/include
        ${CMAKE_CURRENT_SOURCE_DIR}/entities/include
        ${CMAKE_CURRENT_SOURCE_DIR}/entities/include/utils
)

# Link with entities library
target_link_libraries(engine 
    PRIVATE glfw
)

# Create source groups for nice organization in Visual Studio
source_group("Core\\Source Files" FILES ${ENGINE_CORE_SOURCES})
source_group("Core\\Header Files" FILES ${ENGINE_CORE_HEADERS})
#add entities as source group
source_group("Entities\\Header Files" FILES ${ENGINE_ENTITIES_HEADERS})
source_group("Entities\\Source Files" FILES ${ENGINE_ENTITIES_SOURCES})


# Enable testing at engine level
if(BUILD_TESTS)
    enable_testing()
endif()