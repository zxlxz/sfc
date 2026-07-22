project(sfc)

# sfc
add_library(sfc)
target_source_files(sfc . *.cc)

if (NOT (LINUX OR APPLE))
  target_sources_exclude(sfc "posix/.*")
endif()
if(NOT WIN32)
  target_sources_exclude(sfc "windows/.*")
endif()

target_sources(sfc PRIVATE ${_SFC_SRC})
target_compile_features(sfc PUBLIC cxx_std_23)
target_include_directories(sfc PUBLIC ${_SFC_INCLUDE})

# sfc-test
add_executable(sfc-test)
target_source_files(sfc-test . "*.cxx")
target_link_libraries(sfc-test sfc)
add_test(NAME sfc-test COMMAND sfc-test)
