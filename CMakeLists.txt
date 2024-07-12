
cmake_minimum_required(VERSION 3.21)

project(sfc)

add_compile_options(-Wall -Wextra -Wconversion)
add_subdirectory(src)
add_subdirectory(test)

if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(benchmark)
endif()
