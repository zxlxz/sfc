
if (CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

option(SFC_TEST "Build sfc-test" ON)
if (SFC_TEST)
  enable_testing()
endif()

function(target_source_files TARGET DIR)
  set(_PATTERNS ${ARGN})

  set(_SOURCES)
  foreach(PATTERN ${_PATTERNS})
    file(GLOB_RECURSE MATCHED_SOURCES CONFIGURE_DEPENDS "${DIR}/${PATTERN}")
    list(APPEND _SOURCES ${MATCHED_SOURCES})
  endforeach()

  target_sources(${TARGET} PRIVATE ${_SOURCES})
endfunction()
