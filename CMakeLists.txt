
cmake_minimum_required(VERSION 3.21)

project(sfc)

option(SFC_ENABLE_TESTING "Enable SFC unit tests" ON)
add_subdirectory(src)
