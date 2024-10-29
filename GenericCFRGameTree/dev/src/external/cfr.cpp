//
// Created by Amir on 10/27/2024.
//

#include "cfr.h"

#include <ctime>

#include "float_normalization.h"
#include "info_set_tree.h"
#include "search_tree.h"

namespace CFR
{
    namespace Sim
    {
        class GameTreeImpl
        {
        public:

            Game* client_game_;

            using SearchTree = Internal::Trees::SearchTree;
            using InfoSetTree = Internal::Trees::InfoSetTree;

            using SearchTreeNode = Internal::Trees::SearchTreeNode;
            using PlayerSearchNode = Internal::Trees::PlayerSearchNode;
            using ChanceSearchNode = Internal::Trees::ChanceSearchNode;
            using TerminalSearchNode = Internal::Trees::TerminalSearchNode;
            using InfoSetTreeNode = Internal::Trees::InfoSetTreeNode;

            GameTreeImpl(Game *client_game);

            void RunCFR(int iterations);
            void RunMCCFR(int iterations);

            void RunCFRToNashDistance(float nash_distance);
            void RunMCCFRToNashDistance(float nash_distance);

            unsigned long long SearchTreeSize() const;
            unsigned long long InfoSetSize() const;

            uint64_t MCCFR_Helper(const PlayerSearchNode* player_search_node, uint8_t player, uint8_t norm_reach_prob_p1, uint8_t norm_reach_prob_p2);
            uint64_t MCCFR_Helper(const ChanceSearchNode* chance_search_node, uint8_t player, uint8_t norm_reach_prob_p1, uint8_t norm_reach_prob_p2);
            uint64_t MCCFR_Helper(const TerminalSearchNode* terminal_search_node, uint8_t player, uint8_t norm_reach_prob_p1, uint8_t norm_reach_prob_p2);

            std::shared_ptr<Internal::Trees::SearchTree> search_tree_;
            std::shared_ptr<Internal::Trees::InfoSetTree> info_set_tree_;

            Byte* search_tree_mem_ptr;
            Byte* info_set_tree_mem_ptr;

            void InitTrees();

        };

        GameTreeImpl::GameTreeImpl(Game *client_game)
        {
            client_game_ = client_game;
            search_tree_mem_ptr = nullptr;
            info_set_tree_mem_ptr = nullptr;
            srand(static_cast<unsigned>(time(nullptr)));
            InitTrees();
        }

        void GameTreeImpl::RunCFR(int iterations)
        {
            return;
        }

        void GameTreeImpl::RunMCCFR(int iterations)
        {
            return;
        }

        void GameTreeImpl::RunCFRToNashDistance(float nash_distance)
        {
            return;
        }

        void GameTreeImpl::RunMCCFRToNashDistance(float nash_distance)
        {
            return;
        }

        void GameTreeImpl::InitTrees()
        {
            info_set_tree_ = std::make_shared<Internal::Trees::InfoSetTree>(Internal::Trees::InfoSetTree());
            search_tree_ = static_cast<std::shared_ptr<SearchTree>>(new Internal::Trees::SearchTree(info_set_tree_.get()));
            search_tree_->ConstructBothTrees(*client_game_->RootGameState());
            search_tree_mem_ptr = search_tree_->SearchTreePtr();
            info_set_tree_mem_ptr = info_set_tree_->InfoSetPtr();
        }

        uint64_t GameTreeImpl::MCCFR_Helper(const Internal::Trees::PlayerSearchNode *player_node, uint8_t player, uint8_t norm_reach_prob_p1, uint8_t norm_reach_prob_p2)
        {
            std::vector<SearchTreeNode*> children = player_node->GetChildren();
            return 0;
        }

        uint64_t GameTreeImpl::MCCFR_Helper(const Internal::Trees::ChanceSearchNode *chance_node, uint8_t player, uint8_t norm_reach_prob_p1, uint8_t norm_reach_prob_p2)
        {
            std::vector<SearchTreeNode*> children = chance_node->GetChildren();
            uint16_t random_child_index = chance_node->GetRandomChildIndex();
            SearchTreeNode *child = children[random_child_index];
            if (random_child_index < chance_node->GetNumPlayerNodeChildren())
            {
                return MCCFR_Helper(dynamic_cast<PlayerSearchNode*>(child), player, norm_reach_prob_p1, norm_reach_prob_p2);
            }
            if (random_child_index < chance_node->GetNumPlayerNodeChildren() + chance_node->GetNumChanceNodeChildren())
            {
                return MCCFR_Helper(dynamic_cast<ChanceSearchNode*>(child), player, norm_reach_prob_p1, norm_reach_prob_p2);
            }
            return MCCFR_Helper(dynamic_cast<TerminalSearchNode*>(child), player, norm_reach_prob_p1, norm_reach_prob_p2);

        }

        uint64_t GameTreeImpl::MCCFR_Helper(const TerminalSearchNode *terminal_node, uint8_t player, uint8_t norm_reach_prob_p1, uint8_t norm_reach_prob_p2)
        {
            return terminal_node->GetNormalizedUtility(player);
        }

        unsigned long long GameTreeImpl::SearchTreeSize() const
        {
            return search_tree_->Size();
        }

        unsigned long long GameTreeImpl::InfoSetSize() const
        {
            return info_set_tree_->Size();
        }


        GameTree::GameTree(Game *client_game)
        {
            impl_ = new GameTreeImpl(client_game);
        }

        GameTree::~GameTree()
        {
            delete impl_;
        }

        void GameTree::RunCFR(int iterations)
        {
            impl_->RunCFR(iterations);
        }

        void GameTree::RunMCCFR(int iterations)
        {
            impl_->RunMCCFR(iterations);
        }

        void GameTree::RunCFRToNashDistance(float nash_distance)
        {
            impl_->RunCFRToNashDistance(nash_distance);
        }

        void GameTree::RunMCCFRToNashDistance(float nash_distance)
        {
            impl_->RunMCCFRToNashDistance(nash_distance);
        }

        unsigned long long GameTree::SearchTreeSize() const
        {
            return impl_->SearchTreeSize();
        }

        unsigned long long GameTree::InfoSetSize() const
        {
            return impl_->InfoSetSize();
        }
    }
}
