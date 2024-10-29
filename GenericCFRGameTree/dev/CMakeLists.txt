cmake_minimum_required(VERSION 3.29)

project(GenericCFRGameTreeDev VERSION 1.3)

add_library(CFR_Dev_Lib STATIC
        src/internal/SearchTree/search_tree_node.cpp
        src/internal/SearchTree/search_tree_player_node.cpp
        src/internal/SearchTree/search_tree_chance_node.cpp
        src/internal/SearchTree/search_tree_terminal_node.cpp
        src/internal/InfoSetTree/info_set_tree_node.cpp
        src/internal/InfoSetTree/info_set_tree.cpp
        src/internal/SearchTree/search_tree.cpp
        src/external/client_game.cpp
        src/external/cfr.cpp
        src/internal/Utils/float_normalization.cpp
)
target_include_directories(CFR_Dev_Lib PUBLIC ${PROJECT_SOURCE_DIR}/include/external)
target_include_directories(CFR_Dev_Lib PRIVATE ${PROJECT_SOURCE_DIR}/include/internal ${PROJECT_SOURCE_DIR}/include/internal/Utils)
target_include_directories(CFR_Dev_Lib PUBLIC ${PROJECT_SOURCE_DIR}/Test)

add_executable(CFR_Dev_Test Test/test_main.cpp)
add_dependencies(CFR_Dev_Test CFR_Dev_Lib)
target_link_libraries(CFR_Dev_Test PUBLIC CFR_Dev_Lib)
