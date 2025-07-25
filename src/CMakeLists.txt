cmake_minimum_required(VERSION 3.21)

# sfc
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "*.cc")
add_library(sfc ${SRC})
target_include_directories(sfc PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_compile_features(sfc PUBLIC cxx_std_23)

if(CMAKE_HOST_LINUX)
  target_link_options(sfc PUBLIC -rdynamic)
  set_target_properties(sfc PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# test
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "*.cxx")
add_executable(sfc_test ${SRC})
target_link_libraries(sfc_test sfc)
