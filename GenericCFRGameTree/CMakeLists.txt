cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 20)

project(GenericCFRGameTree)

add_library(CFR_Lib STATIC src/cfr_search_tree.cpp src/cfr_search_tree.cpp src/pch.cpp)

target_include_directories(CFR_Lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

