cmake_minimum_required(VERSION 3.25)
project(search_and_destroy_cpp)

set(CMAKE_CXX_STANDARD 23)

add_executable(search_and_destroy_cpp main.cpp sha256.cpp sha256.h sha2.cpp sha2.h picosha.h)
