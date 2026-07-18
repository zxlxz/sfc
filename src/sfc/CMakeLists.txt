project(sfc)

# sfc
add_library(sfc)
target_source_files(sfc alloc       "*.cc")
target_source_files(sfc app         "*.cc")
target_source_files(sfc collections "*.cc")
target_source_files(sfc core        "*.cc")
target_source_files(sfc env         "*.cc")
target_source_files(sfc ffi         "*.cc")
target_source_files(sfc fs          "*.cc")
target_source_files(sfc io          "*.cc")
target_source_files(sfc log         "*.cc")
target_source_files(sfc serde       "*.cc")
target_source_files(sfc sync        "*.cc")
target_source_files(sfc test        "*.cc")
target_source_files(sfc thread      "*.cc")
target_source_files(sfc time        "*.cc")

if (UNIX)
  target_source_files(sfc sys/posix   "*.cc")
elseif (WIN32)
  target_source_files(sfc sys/windows "*.cc")
endif()

target_compile_features(sfc PUBLIC cxx_std_23)
target_include_directories(sfc PUBLIC ${_SFC_INCLUDE})
target_compile_options(sfc PRIVATE -Wall -Wextra -Wconversion)

# sfc-test
add_executable(sfc-test)
target_source_files(sfc-test . "*.cxx")
target_compile_options(sfc-test PRIVATE -Wall -Wextra -Wconversion)
target_link_libraries(sfc-test sfc)

add_test(NAME sfc-test COMMAND sfc-test)
