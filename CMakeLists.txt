cmake_minimum_required(VERSION 3.0)

project(prototyp0)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") # HACK: usunąć gdy CMake 3.1 będzie dostępne

find_package(Boost 1.56 REQUIRED system coroutine)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(demux main.cxx)
target_link_libraries(demux ${Boost_LIBRARIES})