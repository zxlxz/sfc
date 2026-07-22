cmake_minimum_required(VERSION 3.20)

project(sfc)

include(cmake/utils.cmake)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
