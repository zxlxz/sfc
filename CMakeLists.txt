cmake_minimum_required(VERSION 4.0.0)

project(sfc)
set(SFC_ENABLE_TEST ${PROJECT_IS_TOP_LEVEL} CACHE BOOL "Enable SFC test")

add_subdirectory(src)
