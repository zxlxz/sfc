project(sfc)

# sfc
add_library(sfc)
target_source_files(sfc . "*.cc")
target_compile_features(sfc PUBLIC cxx_std_23)
target_include_directories(sfc PUBLIC ${_SFC_INCLUDE})
target_compile_options(sfc PRIVATE -Wall -Wextra -Wconversion)

# sfc-test
add_executable(sfc-test)
target_source_files(sfc-test . "*.cxx")
target_compile_options(sfc-test PRIVATE -Wall -Wextra -Wconversion)
target_link_libraries(sfc-test sfc)

add_test(NAME sfc-test COMMAND sfc-test)
