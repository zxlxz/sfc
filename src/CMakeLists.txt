cmake_minimum_required(VERSION 3.21)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "*.cc")

add_library(sfc ${SRC})

if(CMAKE_HOST_LINUX)
  target_link_options(sfc PUBLIC -rdynamic)
endif()

target_include_directories(sfc PUBLIC ${CMAKE_CURRENT_LIST_DIR})
