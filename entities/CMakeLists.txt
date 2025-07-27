# Create INTERFACE library since we only have headers
add_library(entities INTERFACE)
 
target_include_directories(entities INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set compiler warnings as errors
set(COMPILE_WARNING_AS_ERROR ON)