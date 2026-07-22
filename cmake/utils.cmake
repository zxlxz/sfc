

function(list_source_files VARIABLE DIR)
  set(_PATTERNS ${ARGN})
  set(_SOURCES)

  foreach(PATTERN ${_PATTERNS})
    file(GLOB_RECURSE MATCHED_SOURCES CONFIGURE_DEPENDS "${DIR}/${PATTERN}")
    list(APPEND _SOURCES ${MATCHED_SOURCES})
  endforeach()

  set(${VARIABLE} ${_SOURCES} PARENT_SCOPE)
endfunction()


function(target_source_files TARGET DIR)
  list_source_files(_SOURCES ${DIR} ${ARGN})
  target_sources(${TARGET} PRIVATE ${_SOURCES})
endfunction()

function(target_sources_exclude TARGET PATTERN)
  get_target_property(_SOURCES ${TARGET} SOURCES)
  list(FILTER _SOURCES EXCLUDE REGEX ${PATTERN})
  set_target_properties(${TARGET} PROPERTIES SOURCES "${_SOURCES}")
endfunction()
