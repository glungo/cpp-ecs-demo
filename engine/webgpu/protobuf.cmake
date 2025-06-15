include(FetchContent)

# Protocol Buffers version to use
set(PROTOBUF_VERSION "3.21.12" CACHE STRING "Protocol Buffers version to build")
set(PROTOBUF_DISABLE_RTTI ON CACHE BOOL "Build Protocol Buffers without RTTI")

# Function to fetch and build Protocol Buffers static libraries
function(fetch_protobuf)
  FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v${PROTOBUF_VERSION}
    GIT_SHALLOW ON
  )
  
  # Disable tests and examples to speed up the build
  set(protobuf_BUILD_TESTS OFF CACHE BOOL "Build tests" FORCE)
  set(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
  set(protobuf_WITH_ZLIB OFF CACHE BOOL "Build with zlib support" FORCE)
  set(protobuf_DISABLE_RTTI ${PROTOBUF_DISABLE_RTTI} CACHE BOOL "Remove runtime type information support" FORCE)
  
  # Force static linking
  set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
  
  FetchContent_MakeAvailable(protobuf)
endfunction()

# Call the function to fetch Protocol Buffers
fetch_protobuf()