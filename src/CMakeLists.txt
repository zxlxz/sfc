cmake_minimum_required(VERSION 4.0.0)

project(sfc)

# warning
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)
endif()

# sfc
add_library(sfc)
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "**.cc")
target_sources(sfc PRIVATE ${SRC})
target_compile_features(sfc PUBLIC cxx_std_23)
target_include_directories(sfc PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# sfc-test
add_executable(sfc-test)
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "**.cxx")
target_sources(sfc-test PRIVATE ${SRC})
target_link_libraries(sfc-test sfc)
