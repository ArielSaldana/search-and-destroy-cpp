cmake_minimum_required(VERSION 3.25)
project(search_and_destroy_cpp)

set(CMAKE_CXX_STANDARD 23)

# Add the 'include' directory to the include file search path
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(search_and_destroy_cpp src/main.cpp src/sha256.cpp src/search.cpp)
