cmake_minimum_required(VERSION 3.21)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cxx")

add_executable(sfc_test ${SRC})

target_link_libraries(sfc_test sfc)
