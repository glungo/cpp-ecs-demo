cmake_minimum_required(VERSION 3.14)
project(Engine VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(COMPILE_WARNING_AS_ERROR ON)

# Create the engine library target
add_library(engine)

# Find Vulkan package
find_package(Vulkan REQUIRED)

# Add the 'glfw' directory, which contains the definition of a 'glfw' target
add_subdirectory(third_party/glfw)

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

file(GLOB_RECURSE ENGINE_GRAPHICS_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/graphics/include/*.h"
)

file(GLOB_RECURSE ENGINE_GRAPHICS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/graphics/src/*.cpp"
)

# Add source files to the engine target
target_sources(engine
    PRIVATE 
        ${ENGINE_CORE_SOURCES}
        ${ENGINE_ENTITIES_SOURCES}
        ${ENGINE_GRAPHICS_SOURCES}
    PUBLIC
        ${ENGINE_CORE_HEADERS}
        ${ENGINE_ENTITIES_HEADERS}
        ${ENGINE_GRAPHICS_HEADERS})

# Add include directories for engine core
target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/core/include
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/include
        ${CMAKE_CURRENT_SOURCE_DIR}/entities/include
        ${CMAKE_CURRENT_SOURCE_DIR}/entities/include/utils
        ${Vulkan_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends
)

# Link with entities library
target_link_libraries(engine 
    PUBLIC glfw
    PUBLIC Vulkan::Vulkan
)

# Check if we need to link filesystem library for C++17 std::filesystem
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(engine PUBLIC stdc++fs)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(engine PUBLIC c++fs)
endif()

# Create source groups for nice organization in Visual Studio
source_group("Core\\Source Files" FILES ${ENGINE_CORE_SOURCES})
source_group("Core\\Header Files" FILES ${ENGINE_CORE_HEADERS})
source_group("Graphics\\Source Files" FILES ${ENGINE_GRAPHICS_SOURCES})
source_group("Graphics\\Header Files" FILES ${ENGINE_GRAPHICS_HEADERS}) 
source_group("Entities\\Header Files" FILES ${ENGINE_ENTITIES_HEADERS})
source_group("Entities\\Source Files" FILES ${ENGINE_ENTITIES_SOURCES})

# Add ImGui (no native CMakeLists, create target manually)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)
file(GLOB IMGUI_SOURCES
    "${IMGUI_DIR}/*.cpp"
)
add_library(imgui STATIC ${IMGUI_SOURCES})
# ImGui public include dir
target_include_directories(imgui PUBLIC ${IMGUI_DIR})
# Define for backward compatibility if needed
# target_compile_definitions(imgui PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)

# For now rely on user of imgui to compile backends (e.g., Vulkan + GLFW). Optionally we can add them later.
# Link ImGui into engine
target_link_libraries(engine PUBLIC imgui)

# Enable testing at engine level
if(BUILD_TESTS)
    enable_testing()
endif()