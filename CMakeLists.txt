cmake_minimum_required(VERSION 3.10.0)
project(tiny_renderer VERSION 0.1.0 LANGUAGES C CXX)

include_directories(include)

add_executable(hello part_0_hello/main.cpp)
add_executable(line part_1_line/main.cpp)
add_executable(triangle part_2_triangle/main.cpp)
add_executable(zbuffer part_3_zbuffer/main.cpp)
add_executable(camera part_4_camera/main.cpp)
add_executable(shading part_5_shading/main.cpp part_5_shading/my_gl.cpp)

file(GLOB P_6_SRC "part_6_texture/*.cpp")
add_executable(texture ${P_6_SRC})

file(GLOB P_7_SRC "part_7_tangent/*.cpp")
add_executable(tangent ${P_7_SRC})

file(GLOB P_8_SRC "part_8_shadow/*.cpp")
add_executable(shadow ${P_8_SRC})


add_executable(tiny_renderer tiny_renderer/main.cpp)
