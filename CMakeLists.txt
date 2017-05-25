cmake_minimum_required(VERSION 3.7)
project(Project_1_b)

set(CMAKE_CXX_STANDARD 11)
set(BOOST_ROOT "/Users/student/addlib/boost_1_57_0")

find_package(Boost 1.57.0)
set(Boost_LIB_PREFIX            "lib"       CACHE STRING "")
set(Boost_USE_MULTITHREADED     ON          CACHE BOOL "") # '-mt' flag
set(Boost_USE_STATIC_LIBS       ON          CACHE BOOL "")
set(Boost_USE_STATIC_RUNTIME    ON          CACHE BOOL "") # '-s' tag
set(Boost_USE_DEBUG_RUNTIME     ON          CACHE BOOL "") # '-g' tag
set(Boost_COMPILER              "-mgw49"    CACHE STRING "")
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()
include_directories(${Boost_INCLUDE_DIRS})
set(SOURCE_FILES main.cpp d_except.h d_random.h)
add_executable(Project_1 ${SOURCE_FILES})

