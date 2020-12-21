cmake_minimum_required(VERSION 3.10)

project(bfc)

set(CMAKE_CXX_STANDARD 17)

add_library(bfc INTERFACE)
target_include_directories(bfc INTERFACE src)

add_library(gtest STATIC gtest/gmock-gtest-all.cc)
target_include_directories(gtest PUBLIC gtest)

FILE(GLOB tests_SRC tests/*.cpp)
add_executable(tests ${tests_SRC})
target_link_libraries(tests PRIVATE bfc gtest pthread)
