
cmake_minimum_required(VERSION 3.21)

project(sfc)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_compile_options(-Wall)

add_subdirectory(src)
add_subdirectory(test)
