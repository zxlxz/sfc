
cmake_minimum_required(VERSION 3.21)

project(sfc)

option(SFC_ENABLE_TESTING "Enable SFC unit tests" ON)
option(SFC_ENABLE_ASAN    "Enable SFC ASAN support" OFF)

if(SFC_ENABLE_ASAN)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address)
endif()

add_subdirectory(src)
