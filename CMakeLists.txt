cmake_minimum_required(VERSION 3.10.0)
project(tiny_renderer VERSION 0.1.0 LANGUAGES C CXX)

include_directories(include)

add_executable(hello part_0_hello/main.cpp)
add_executable(line part_1_line/main.cpp)
add_executable(triangle part_2_triangle/main.cpp)

