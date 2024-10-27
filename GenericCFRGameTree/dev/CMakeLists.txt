cmake_minimum_required(VERSION 3.29)

project(GenericCFRGameTreeDev VERSION 1.3)


add_library(CFR_Dev_Lib STATIC src/new_tree_nodes.cpp
        src/internal/search_tree_node.cpp
        src/internal/search_tree_player_node.cpp
        src/internal/search_tree_chance_node.cpp
        src/internal/search_tree_terminal_node.cpp
        src/internal/info_set_tree_node.cpp)
target_include_directories(CFR_Dev_Lib PUBLIC ${PROJECT_SOURCE_DIR}/include/external)
target_include_directories(CFR_Dev_Lib PRIVATE ${PROJECT_SOURCE_DIR}/include/internal)
