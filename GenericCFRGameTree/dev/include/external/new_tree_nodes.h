//
// Created by Amir on 9/7/2024.
//
#pragma once
#ifndef NEW_TREE_NODES_H
#define NEW_TREE_NODES_H
#include "game_classes.h"

#endif //NEW_TREE_NODES_H

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

namespace generic_cfr
{

    typedef unsigned char Byte;

    //Forward Declarations.
    class SearchTree;
    class InfoSetTree;


    class GameTree {
    public:
        Game* client_game_;

        ~GameTree();
        explicit GameTree(Game* client_game);

        void RunCFR(int iterations);
        void RunMCCFR(int iterations);

        void RunCFRToNashDistance(float nash_distance);
        void RunMCCFRToNashDistance(float nash_distance);

        void StartTreeNavigation();

    private:
        void InitTrees();
        void ExploreNode(const Game::GameState& node);

        SearchTree* search_tree_;
        InfoSetTree* info_set_tree_;
    };

}
