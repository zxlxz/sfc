
cmake_minimum_required(VERSION 3.21)

project(sfc)

add_compile_options(-Wall -Wextra -Wconversion)
add_subdirectory(src)

if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(test)
endif()

option(ENABLE_BENCHMARK "benchmark")
if(ENABLE_BENCHMARK)
    add_subdirectory(benchmark)
endif()
