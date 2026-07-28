cmake_minimum_required(VERSION 3.20)

project(sfc)

include(cmake/utils.cmake)

add_subdirectory(src)

if (PROJECT_IS_TOP_LEVEL)
  enable_testing()
  add_subdirectory(tests)
endif()
