cmake_minimum_required(VERSION 3.10)

project(bfc)

set(CMAKE_CXX_STANDARD 17)

add_library(bfc INTERFACE)
target_include_directories(bfc INTERFACE src)

FILE(GLOB tests_SRC tests/*.cpp)
add_executable(tests ${tests_SRC} gtest/gmock-gtest-all.cc)
target_include_directories(tests PUBLIC gtest)
target_link_libraries(tests PRIVATE bfc pthread)
