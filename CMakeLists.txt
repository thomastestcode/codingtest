# Specify the minimum version for CMake

cmake_minimum_required(VERSION 3.12)

# Project's name

project(hello)
# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -pthread")

# The following folder will be included
include_directories("${PROJECT_SOURCE_DIR}")
add_executable(ProducerConsumer ${PROJECT_SOURCE_DIR}/ProducerConsumer.cpp)

