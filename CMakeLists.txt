
cmake_minimum_required(VERSION 3.21)

project(sfc)

option(SFC_ENABLE_ASAN "Enable SFC ASAN Tests" OFF)
option(SFC_ENABLE_TEST "Enable SFC UNIT Tests" ${PROJECT_IS_TOP_LEVEL})

if(SFC_ENABLE_ASAN)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address)
endif()

add_subdirectory(src)
