cmake_minimum_required(VERSION 2.8)
project(hello)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(galileo_hello ${SOURCE_FILES})