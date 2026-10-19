
if (CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()


set(_SFC_PATH ${CMAKE_CURRENT_LIST_DIR})
add_subdirectory(sfc)
