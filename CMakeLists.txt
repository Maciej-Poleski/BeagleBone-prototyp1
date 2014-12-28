cmake_minimum_required(VERSION 3.0)

project(prototyp1)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") # HACK: usunąć gdy CMake 3.1 będzie dostępne

find_package(Threads REQUIRED) # HACK: to chyba bug w CMake 3.0 ...

find_package(Boost 1.55 REQUIRED system coroutine context)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(demux src/main.cxx src/vuart.cxx)
target_link_libraries(demux ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})