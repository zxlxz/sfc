cmake_minimum_required(VERSION 3.21)

# sfc
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/**.cc")
add_library(sfc ${SRC})
target_include_directories(sfc PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_compile_features(sfc PUBLIC cxx_std_23)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(sfc PRIVATE -Wall -Wextra -Wpedantic)
endif()

# test
if(SFC_ENABLE_TESTING)
  file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/**.cxx")
  add_executable(sfc_test ${SRC})
  target_link_libraries(sfc_test sfc)
endif()
