cmake_minimum_required(VERSION 3.10.0)
project(ecs-demo VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -fsanitize=address")
enable_testing()
add_subdirectory(src/entities)
add_executable(ecs-demo src/main.cpp)
target_link_libraries(ecs-demo PRIVATE entities)

# Add custom target to run tests
add_custom_target(run_tests ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS entity_manager_tests archetype_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests..."
)

