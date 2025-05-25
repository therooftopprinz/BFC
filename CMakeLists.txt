cmake_minimum_required(VERSION 3.10)

project(bfc)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Ofast")

add_library(bfc INTERFACE)
target_include_directories(bfc INTERFACE src)

FILE(GLOB bfc_tests_SRC
    tests/ut/*.cpp
    tests/e2e/*.cpp
    tests/gtestmain.cpp)

add_executable(bfc_tests ${bfc_tests_SRC} gtest/gmock-gtest-all.cc)
target_include_directories(bfc_tests PUBLIC gtest)
target_link_libraries(bfc_tests PRIVATE bfc pthread)
