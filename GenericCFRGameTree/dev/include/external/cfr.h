//
// Created by Amir on 9/7/2024.
//
#pragma once
#ifndef NEW_TREE_NODES_H
#define NEW_TREE_NODES_H
#endif //NEW_TREE_NODES_H

#include "external.h"

namespace CFR
{
    namespace Sim
    {
        using Byte = unsigned char;

        //Forward Declaration of hidden implementation class.
        class GameTreeImpl;

        class GameTree {
        public:

            ~GameTree();
            explicit GameTree(Game* client_game);

            void RunCFR(int iterations);
            void RunMCCFR(int iterations);

            void RunCFRToNashDistance(float nash_distance);
            void RunMCCFRToNashDistance(float nash_distance);

            unsigned long long SearchTreeSize() const;
            unsigned long long InfoSetSize() const;

        private:
            void InitTrees();
            GameTreeImpl* impl_;
        };
    }
}
