
aux_source_directory(. SRC)
add_executable(benchmark_log ${SRC})

target_link_libraries(benchmark_log sfc)
target_link_libraries(benchmark_log glog)
target_link_libraries(benchmark_log fmt)
