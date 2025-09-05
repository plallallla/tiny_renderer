cmake_minimum_required(VERSION 3.10.0)
project(tiny_renderer VERSION 0.1.0 LANGUAGES C CXX)

include_directories(include)

add_executable(hello hello_part_0/main.cpp)
add_executable(line line_part_1/main.cpp)

