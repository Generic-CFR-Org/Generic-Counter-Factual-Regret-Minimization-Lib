//
// Created by Amir on 9/7/2024.
//

#ifndef NEW_TREE_NODES_H
#define NEW_TREE_NODES_H

#endif //NEW_TREE_NODES_H

#include "pch.h"
#include <vector>
#include <memory>
#include <string>

namespace generic_cfr {

    typedef unsigned char Byte;

    //Forward Declarations.
    class SearchTree;
    class InfoSetTree;

    template<class ClientGame>
    class GameTree {
    public:
        ClientGame client_game_;

        int max_tree_depth_;

        SearchTree* search_tree_;
        InfoSetTree* info_set_tree_;
    };

    class SearchTree {

        std::vector<long long> search_tree_offset_at_depth_;
        long long search_tree_block_size_;
        long long search_tree_size_;

        std::vector<Byte> search_tree_;

    };

    class InfoSetTree {

        std::vector<long long> info_set_tree_offset_at_depth_;
        long long info_set_tree_block_size_;
        long long info_set_tree_size_;

        std::vector<Byte> info_set_tree_;
    };
}
