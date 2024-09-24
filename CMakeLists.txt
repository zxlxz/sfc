
cmake_minimum_required(VERSION 3.21)

project(sfc)

add_compile_options(-Wall -Wextra -Wconversion)
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_compile_options(-stdlib=libc++)
    add_link_options(-stdlib=libc++)
endif()

add_subdirectory(src)
add_subdirectory(test)
