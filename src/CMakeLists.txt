cmake_minimum_required(VERSION 3.21)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cc")

add_library(sfc ${SRC})
target_compile_features(sfc PUBLIC cxx_std_20)

target_link_options(sfc PUBLIC -rdynamic)
target_include_directories(sfc PUBLIC ${CMAKE_CURRENT_LIST_DIR})
