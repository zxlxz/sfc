
if (CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion")
endif()

add_subdirectory(sfc)
target_include_directories(sfc PUBLIC ${CMAKE_CURRENT_LIST_DIR})
