cmake_minimum_required(VERSION 3.22.1)

project(CircularBuffer-lab-8)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib)

enable_testing()
add_subdirectory(tests)