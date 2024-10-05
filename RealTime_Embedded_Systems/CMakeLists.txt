cmake_minimum_required(VERSION 3.10)

# Path to the toolchain for cross-compiling
set(CMAKE_TOOLCHAIN_FILE "../toolchain-aarch64.cmake")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set appropriate logging level
if(NOT DEFINED CURRENT_LOG_LEVEL)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-DCURRENT_LOG_LEVEL=LOG_LEVEL_DEBUG)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_definitions(-DCURRENT_LOG_LEVEL=LOG_LEVEL_INFO)
    endif()
endif()

project(finnhub_websocket)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Fetch the dependencies
include(FetchContent)

# ===== simdjson =====
FetchContent_Declare(
  simdjson
  GIT_REPOSITORY https://github.com/simdjson/simdjson.git
  GIT_SHALLOW TRUE
  GIT_TAG master
)
FetchContent_MakeAvailable(simdjson)

# ===== fmt (Header-only) =====
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master
)
FetchContent_MakeAvailable(fmt)
add_library(fmt_header_only INTERFACE)
target_include_directories(fmt_header_only INTERFACE ${fmt_SOURCE_DIR}/include)
target_compile_definitions(fmt_header_only INTERFACE FMT_HEADER_ONLY)

# Find the required libraries
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(libwebsockets REQUIRED)

# Find the source files
file(GLOB_RECURSE SOURCES_CPP "src/*.cpp")

# Include our headers in the src directory
include_directories("src/")

# Add the executable
add_executable(finnhub_websocket ${SOURCES_CPP})


target_link_libraries(finnhub_websocket PRIVATE 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    simdjson 
    websockets 
    fmt_header_only 
    Threads::Threads)
