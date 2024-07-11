cmake_minimum_required(VERSION 3.21)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cc")

add_library(sfc ${SRC})

if("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    target_compile_features(sfc PUBLIC cxx_std_20)
else()
    target_compile_features(sfc PUBLIC cxx_std_17)
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        target_compile_options(sfc PUBLIC -fconcepts)
    endif()
endif()

target_link_options(sfc PUBLIC -rdynamic)
target_include_directories(sfc PUBLIC ${CMAKE_CURRENT_LIST_DIR})
