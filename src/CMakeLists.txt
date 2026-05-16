cmake_minimum_required(VERSION 4.0.0)

# sfc
add_library(sfc)
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "**.cc")
target_sources(sfc PRIVATE ${SRC})
target_compile_features(sfc PUBLIC cxx_std_23)
target_include_directories(sfc PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# warning
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(sfc PRIVATE -Wall -Wextra -Wpedantic)
endif()

# option: lto
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  set_target_properties(sfc PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
endif()

# test (enable only if project is top level)
if (SFC_ENABLE_TEST)
  add_executable(sfc_test)
  target_link_libraries(sfc_test sfc)
  file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "**.cxx")
  target_sources(sfc_test PRIVATE ${SRC})
endif()
