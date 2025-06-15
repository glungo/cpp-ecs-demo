cmake_minimum_required(VERSION 3.14)
project(Engine VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(COMPILE_WARNING_AS_ERROR ON)

if (NOT EMSCRIPTEN)
    # Add the 'glfw' directory, which contains the definition of a 'glfw' target
    add_subdirectory(glfw)
else()
    # Create a mock 'glfw' target that just sets the `-sUSE_GLFW=3` link option:
    add_library(glfw INTERFACE)
    target_link_options(glfw INTERFACE -sUSE_GLFW=3)
endif()
# Add WebGPU as a dependency before creating the engine library
add_subdirectory(webgpu)

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
        ${CMAKE_BINARY_DIR}/_deps/webgpu-backend-wgpu-src/include  # Add WebGPU headers path
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link with entities library and WebGPU
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../entities entities)
target_link_libraries(engine PUBLIC entities)
# Add the 'glfw' target as a dependency of our App
target_link_libraries(engine PRIVATE webgpu glfw)
target_link_libraries(engine PUBLIC webgpu) # Changed PRIVATE to PUBLIC to propagate to executables

# Enable testing
if(BUILD_TESTS)
    enable_testing()
    
    # Add engine tests
    add_executable(test_engine_basic ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_engine_basic.cpp)
    target_link_libraries(test_engine_basic PRIVATE engine)
    
    # The application's binary must find wgpu.dll or libwgpu.so at runtime,
    # so we copy it next to the test executable
    target_copy_webgpu_binaries(test_engine_basic)
    
    add_test(NAME TestEngineBasic COMMAND test_engine_basic)
    
    # Store the test target in a list for the parent CMake to use
    set(ENGINE_TEST_TARGETS test_engine_basic)
    set_property(GLOBAL PROPERTY ENGINE_TEST_TARGETS ${ENGINE_TEST_TARGETS})
endif()