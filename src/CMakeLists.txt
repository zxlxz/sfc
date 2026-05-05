cmake_minimum_required(VERSION 4.0.0)

# sfc
add_library(sfc)
target_compile_features(sfc PUBLIC cxx_std_23)
set_target_properties(sfc PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)

if (MSVC)
  target_compile_options(sfc PRIVATE /W4)
else()
  target_compile_options(sfc PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(sfc PUBLIC ${CMAKE_CURRENT_LIST_DIR})
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "**.cc")
target_sources(sfc PRIVATE ${SRC})

# test
if(SFC_ENABLE_TEST)
  add_executable(sfc_test)
  target_link_libraries(sfc_test sfc)
  file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "**.cxx")
  target_sources(sfc_test PRIVATE ${SRC})
endif()
