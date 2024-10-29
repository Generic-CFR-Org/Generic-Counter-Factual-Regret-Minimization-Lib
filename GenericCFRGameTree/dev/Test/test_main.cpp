//
// Created by Amir on 10/27/2024.
//
#include "rps.h"
#include <cfr.h>
#include <iostream>
#include <ostream>

int main(int argc, char* argv[])
{
    RPS_Test::RPS_Player* p = new RPS_Test::RPS_Player();
    CFR::ClientGame::Game* game = new CFR::ClientGame::Game(p);
    CFR::Sim::GameTree* tree = new CFR::Sim::GameTree(game);
    std::cout << tree->SearchTreeSize() << std::endl;
    std::cout << tree->InfoSetSize() << std::endl;

}
