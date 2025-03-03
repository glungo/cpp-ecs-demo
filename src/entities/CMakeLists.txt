project(ecs-entities VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(entities STATIC
    entities.cpp
    entity.h
    world.h
    component.h
)
target_include_directories(entities PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add tests
enable_testing()
add_executable(world_tests tests/world_tests.cpp)
target_link_libraries(world_tests PRIVATE entities)
add_test(NAME WorldTests COMMAND world_tests) 

add_executable(archetype_tests tests/archetype_tests.cpp)
target_link_libraries(archetype_tests PRIVATE entities)
add_test(NAME ArchetypeTests COMMAND archetype_tests) 

add_executable(component_tests tests/component_tests.cpp)
target_link_libraries(component_tests PRIVATE entities)
add_test(NAME ComponentTests COMMAND component_tests) 

add_executable(entity_manager_tests tests/entity_manager_tests.cpp)
target_link_libraries(entity_manager_tests PRIVATE entities)
add_test(NAME EntityManagerTests COMMAND entity_manager_tests) 