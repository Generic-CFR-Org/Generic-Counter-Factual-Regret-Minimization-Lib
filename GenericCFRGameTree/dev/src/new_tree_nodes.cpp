//
// Created by Amir on 10/16/2024.
//
#include <new_tree_nodes.h>

namespace generic_cfr
{

    GameTree::GameTree(Game *client_game)
    {
        client_game_ = client_game;

        InitTrees();
    }


    void GameTree::InitTrees()
    {
        Game::GameState* root = client_game_->RootGameState();

        //Explore nodes using Depth First Search, adding metadata to
        //Search Tree and Info Set Tree classes for each node.

    }

    void GameTree::ExploreNode(const Game::GameState &node)
    {


        if (node.IsChanceNode())
        {
            const std::vector<std::pair<Game::Action, float>> chance_node_actions = node.ChanceActions();
            for (auto chance_node_action : chance_node_actions)
            {
                Game::GameState child_node = node.ChildGameState(chance_node_action.first);
                //TODO: Add function that determine size of arbitrary node.
                //TODO:
            }
        }
        else if (node.IsPlayerNode())
        {

        }
        else
        {

        }

    }




}