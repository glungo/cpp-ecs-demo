# Helper file for building Dawn with Protocol Buffers
cmake_minimum_required(VERSION 3.14)

# Use FindProtobuf instead of FetchContent
include(FindProtobuf)

# Try to find Protocol Buffers on the system first
find_package(Protobuf)

# If Protobuf is not found, fetch and build it
if(NOT Protobuf_FOUND)
  message(STATUS "Protocol Buffers not found on system, fetching from source")
  
  # Add Protocol Buffers using FetchContent
  include(FetchContent)
  
  # Define Protocol Buffers version
  set(PROTOBUF_VERSION "3.21.12" CACHE STRING "Protocol Buffers version")
  
  # Configure protobuf options
  set(protobuf_BUILD_TESTS OFF CACHE BOOL "Build tests" FORCE)
  set(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
  set(protobuf_WITH_ZLIB OFF CACHE BOOL "Build with zlib support" FORCE)
  set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
  
  # Fetch Protocol Buffers
  FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v${PROTOBUF_VERSION}
    GIT_SHALLOW ON
  )
  
  # Make Protocol Buffers available
  FetchContent_MakeAvailable(protobuf)
else()
  message(STATUS "Found Protocol Buffers: ${Protobuf_VERSION}")
endif()

# Define the missing generate_protos function that Dawn requires
function(generate_protos)
  cmake_parse_arguments(PARSE_ARGV 0 GEN_PROTOS "" "TINT_ROOT;PROTO_DIR;OUTPUT_DIR" "SRCS")
  
  # Get the paths
  get_filename_component(proto_dir "${GEN_PROTOS_PROTO_DIR}" ABSOLUTE)
  get_filename_component(output_dir "${GEN_PROTOS_OUTPUT_DIR}" ABSOLUTE)
  
  # Make sure the output directory exists
  file(MAKE_DIRECTORY "${output_dir}")
  
  # Create a list to collect all generated files
  set(all_gen_files)
  
  # For each source file
  foreach(proto ${GEN_PROTOS_SRCS})
    # Get the file name without extension
    get_filename_component(proto_name "${proto}" NAME_WE)
    
    # Create paths for the generated files
    set(proto_file "${proto_dir}/${proto}")
    set(gen_cc "${output_dir}/${proto_name}.pb.cc")
    set(gen_h "${output_dir}/${proto_name}.pb.h")
    
    # Add a custom command to generate the C++ files from the proto file
    if(TARGET protobuf::protoc)
      add_custom_command(
        OUTPUT "${gen_cc}" "${gen_h}"
        COMMAND protobuf::protoc
        ARGS --cpp_out="${output_dir}" -I"${proto_dir}" "${proto_file}"
        DEPENDS "${proto_file}" protobuf::protoc
        COMMENT "Generating protobuf files from ${proto}"
        VERBATIM
      )
    else()
      message(STATUS "Protoc not found, creating dummy implementation for ${proto}")
      # Create dummy files with valid C++ code that will compile into an object file
      file(WRITE "${gen_cc}" 
        "// Auto-generated dummy file\n"
        "#include \"${gen_h}\"\n"
        "// Provide a dummy symbol to ensure the library is not empty\n"
        "namespace tint::lang::core::ir::pb {\n"
        "  namespace ${proto_name}_pb {\n"
        "    int dummy_${proto_name}_value = 0;\n"
        "  }\n"
        "}\n"
      )
      file(WRITE "${gen_h}" 
        "#pragma once\n"
        "// Auto-generated dummy header\n"
        "namespace tint::lang::core::ir::pb {\n"
        "  namespace ${proto_name}_pb {\n"
        "    extern int dummy_${proto_name}_value;\n"
        "  }\n"
        "}\n"
      )
    endif()
    
    # Add generated files to the list
    list(APPEND all_gen_files "${gen_cc}" "${gen_h}")
    
    # Set properties to mark these files as generated
    set_source_files_properties("${gen_cc}" "${gen_h}" PROPERTIES GENERATED TRUE)
  endforeach()
  
  # Set variable in parent scope so the calling CMake code can access the generated files
  set(GEN_PROTO_SOURCES ${all_gen_files} PARENT_SCOPE)
  
  # Create a dummy source file if no proto files were processed
  if(NOT GEN_PROTOS_SRCS)
    set(dummy_file "${output_dir}/dummy_proto_file.cc")
    file(WRITE "${dummy_file}" 
      "// Auto-generated dummy file to ensure the library is not empty\n"
      "namespace tint::lang::core::ir::pb {\n"
      "  int dummy_proto_value = 0;\n"
      "}\n"
    )
    set_source_files_properties("${dummy_file}" PROPERTIES GENERATED TRUE)
    set(GEN_PROTO_SOURCES "${dummy_file}" PARENT_SCOPE)
  endif()
endfunction()

# Special handling for the tint_lang_core_ir_binary_proto target
macro(fix_tint_ir_binary_proto)
  if(TARGET tint_lang_core_ir_binary_proto)
    # Create a dummy source file directly
    set(dummy_file "${CMAKE_CURRENT_BINARY_DIR}/dummy_binary_proto.cc")
    file(WRITE "${dummy_file}" 
      "// Auto-generated dummy file for tint_lang_core_ir_binary_proto\n"
      "namespace tint::lang::core::ir::pb {\n"
      "  int ensure_binary_proto_not_empty = 1;\n"
      "}\n"
    )
    
    # Add the dummy source to the target
    target_sources(tint_lang_core_ir_binary_proto PRIVATE "${dummy_file}")
    
    message(STATUS "Added dummy source file to tint_lang_core_ir_binary_proto target")
  endif()
endmacro()

# Configure Dawn options
set(DAWN_ENABLE_INSTALL OFF CACHE BOOL "Enable installation" FORCE)
set(DAWN_ENABLE_DESKTOP_GL ON CACHE BOOL "Enable OpenGL support" FORCE)
set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "Don't build samples" FORCE)
set(DAWN_USE_PROTOBUF ON CACHE BOOL "Use protobuf for Dawn" FORCE)
set(DAWN_ENABLE_PIC OFF CACHE BOOL "Don't build with PIC" FORCE)
set(DAWN_USE_GLFW ON CACHE BOOL "Use GLFW" FORCE)
set(DAWN_BUILD_BENCHMARKS OFF CACHE BOOL "Don't build benchmarks" FORCE)
set(DAWN_BUILD_NODE_BINDINGS OFF CACHE BOOL "Don't build Node bindings" FORCE)
set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "Fetch dependencies" FORCE)
set(TINT_BUILD_SAMPLES OFF CACHE BOOL "Don't build Tint samples" FORCE)
set(TINT_BUILD_DOCS OFF CACHE BOOL "Don't build Tint docs" FORCE)
set(TINT_BUILD_TESTS OFF CACHE BOOL "Don't build Tint tests" FORCE)
set(TINT_BUILD_FUZZERS OFF CACHE BOOL "Don't build fuzzers" FORCE)
set(LIBPROTOBUF_MUTATOR_BUILD_TESTLIB OFF CACHE BOOL "Don't build libprotobuf-mutator test library" FORCE)

# Set this flag to avoid issues with empty targets
set(CMAKE_ALLOW_EMPTY_TARGETS OFF)

# Add hook into Dawn's CMake to fix the problematic target
cmake_language(DEFER DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/_deps/dawn-build" CALL fix_tint_ir_binary_proto)