cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 20)

project(GenericTreeTest)

add_subdirectory(src)

add_executable(GenericTreeTest src/main.cpp)