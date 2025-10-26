cmake_minimum_required(VERSION 3.10)
project(httplib_boost_beast)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost
find_package(Boost 1.70 REQUIRED COMPONENTS system)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${Boost_INCLUDE_DIRS})

# Link libraries
link_libraries(${Boost_LIBRARIES} pthread)

# Examples
add_executable(http_server examples/http_server.cpp)
add_executable(http_client examples/http_client.cpp)
add_executable(ws_server examples/ws_server.cpp)
add_executable(ws_client examples/ws_client.cpp)

# Tests
# The cpp-httplib test suite is available under the tests/ directory.
# Refer to tests/CMakeLists.txt for building the upstream tests.

# Installation
install(FILES include/httplib.h DESTINATION include)
