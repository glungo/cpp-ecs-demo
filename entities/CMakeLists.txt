# Create INTERFACE library since we only have headers
add_library(entities INTERFACE)
 
target_include_directories(entities INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Create a property to store test targets
set_property(GLOBAL PROPERTY ENTITIES_TEST_TARGETS "")
set(address_sanitizer_flag "-fsanitize=address")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${address_sanitizer_flag}")
# Only build tests if BUILD_TESTS is ON
if(BUILD_TESTS)
    enable_testing()
    # Function to add a test
    function(add_entities_test test_name)
        add_executable(${test_name} 
            tests/${test_name}.cpp
        )
        target_link_libraries(${test_name} PRIVATE entities)
        target_include_directories(${test_name} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/..
        )
        # Configure test to show output
        add_test(
            NAME ${test_name} 
            COMMAND ${test_name}
        )
        set_tests_properties(${test_name} PROPERTIES
            ENVIRONMENT "CTEST_OUTPUT_ON_FAILURE=1"
        )
        
        # Add the test target to our list
        get_property(TEST_TARGETS GLOBAL PROPERTY ENTITIES_TEST_TARGETS)
        list(APPEND TEST_TARGETS ${test_name})
        set_property(GLOBAL PROPERTY ENTITIES_TEST_TARGETS "${TEST_TARGETS}")
    endfunction()

    # Add all tests
    add_entities_test(test_entity_creation)
    add_entities_test(test_component_creation)
    add_entities_test(test_pool_creation)
    add_entities_test(test_archetype_creation)
    add_entities_test(test_archetype_component_access)
    add_entities_test(test_archetype_entity_iteration)
    add_entities_test(test_basic_entity_creation)
    add_entities_test(test_entity_pool_overflow)
    add_entities_test(test_entity_reuse)
    add_entities_test(test_job_create)
    add_entities_test(test_job_parallel_execution)
    add_entities_test(test_job_cache_refresh)
    add_entities_test(test_job_multiple)
    add_entities_test(test_job_scheduler)
    #add_entities_test(test_system)
endif()