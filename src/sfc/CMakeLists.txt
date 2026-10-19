
# sfc
add_library(sfc)
target_compile_features(sfc PUBLIC cxx_std_23)
target_compile_options(sfc PRIVATE -Wall -Wextra -Wconversion)
target_include_directories(sfc PUBLIC ${_SFC_PATH})

target_source_files(sfc  . "*.cc")
target_source_exclude(sfc "sys/.*")

if (LINUX OR APPLE)
  target_source_files(sfc  sys/posix "*.cc")
endif()

if(WIN32)
  target_source_files(sfc  sys/windows "*.cc")
endif()

# sfc-test
add_executable(sfc-test)
target_source_files(sfc-test  . "*.cxx")
target_link_libraries(sfc-test sfc)
add_test(NAME sfc-test COMMAND sfc-test)
