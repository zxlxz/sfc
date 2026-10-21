cmake_minimum_required(VERSION 4.0)

project(sfc)

include(cmake/utils.cmake)
enable_testing()

add_subdirectory(src)

if (PROJECT_IS_TOP_LEVEL)
  add_subdirectory(tests)
endif()
