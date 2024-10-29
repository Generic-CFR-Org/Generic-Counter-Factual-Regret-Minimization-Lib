//
// Created by Amir on 10/26/2024.
//

#include "search_tree.h"

#include <iostream>


#include <numeric>
#include <ostream>
#include <algorithm>

#include "info_set_tree.h"



namespace Internal
{
    namespace Trees
    {
        SearchTree::SearchTree()
        {
            search_tree_depth_ = 0;
            search_tree_size_ = 0;
            max_utility_ = 0.0f;
            info_set_tree_ptr_ = new InfoSetTree();
            search_tree_ = nullptr;
        }

        SearchTree::SearchTree(InfoSetTree* info_set_tree_ptr)
        {
            search_tree_depth_ = 0;
            search_tree_size_ = 0;
            max_utility_ = 0.0f;
            info_set_tree_ptr_ = info_set_tree_ptr;
            search_tree_ = nullptr;
        }

        void SearchTree::ConstructBothTrees(const GameState &root_game_state)
        {
            ExploreGameStateStageOne(root_game_state, 0);
            AllocateSearchTreeMem();
            UpdateOffsets();
            info_set_tree_ptr_->ConstructInfoSetTree();
            ExploreGameStateStageTwo(root_game_state, 0);
        }

        Byte *SearchTree::SearchTreePtr() const
        {
            return search_tree_;
        }

        InfoSetTree *SearchTree::InfoSetDataPtr() const
        {
            return info_set_tree_ptr_;
        }

        unsigned long long SearchTree::Size() const
        {
            return search_tree_size_;
        }





        void SearchTree::ExploreGameStateStageOne(const GameState &game_state, int curr_depth)
        {

            if (search_tree_size_at_depth_.size() <= curr_depth)
            {
                search_tree_size_at_depth_.push_back(0);
                search_tree_depth_ += 1;
            }
            if (game_state.IsTerminalNode())
            {
                AddGameStateAtDepth(game_state, curr_depth);
                return;
            }
            std::vector<std::shared_ptr<const GameState>> sorted_children = game_state.AllChildrenSorted();
            search_tree_depth_ = std::max(search_tree_depth_, curr_depth + 1);
            for (std::shared_ptr<const GameState> child : sorted_children)
            {
                ExploreGameStateStageOne(*child, curr_depth + 1);
            }
            AddGameStateAtDepth(game_state, curr_depth);
        }

        void SearchTree::ExploreGameStateStageTwo(const GameState &game_state, int curr_depth)
        {

            if (game_state.IsTerminalNode())
            {
                SetGameStateInMem(game_state, curr_depth);
                return;
            }
            std::vector<std::shared_ptr<const GameState>> sorted_children = game_state.AllChildrenSorted();
            search_tree_depth_ = std::max(search_tree_depth_, curr_depth + 1);
            for (std::shared_ptr<const GameState> child : sorted_children)
            {
                ExploreGameStateStageOne(*child, curr_depth + 1);
            }
            SetGameStateInMem(game_state, curr_depth);
        }

        void SearchTree::AddGameStateAtDepth(const GameState &game_state, const int depth)
        {
            if (game_state.IsPlayerNode())
            {
                AddPlayerNodeAtDepth(game_state, depth);
            }
            else if (game_state.IsChanceNode())
            {
                AddChanceNodeAtDepth(game_state, depth);
            }
            else
            {
                AddTerminalNodeAtDepth(game_state, depth);
            }
        }

        void SearchTree::AllocateSearchTreeMem()
        {
            search_tree_size_ = std::accumulate(search_tree_size_at_depth_.begin(), search_tree_size_at_depth_.end(), 0ULL);
            try
            {
                search_tree_ = new Byte[search_tree_size_];
            }
            catch (std::bad_alloc &e)
            {
                std::cerr << "Exception allocating memory for search tree." << e.what() << std::endl;
            }
        }

        void SearchTree::SetGameStateInMem(const GameState &game_state, int depth)
        {
            if (game_state.IsPlayerNode())
            {
                SetPlayerNodeAtDepth(game_state, depth);
            }
            else if (game_state.IsChanceNode())
            {
                SetChanceNodeAtDepth(game_state, depth);
            }
            else
            {
                SetTerminalNodeAtDepth(game_state, depth);
            }
        }


        //TODO: Add info set implementation
        void SearchTree::AddPlayerNodeAtDepth(const GameState &player_node, const int depth)
        {
            std::string player_view_hash = player_node.PlayerViewHash();
            NumActionsInt num_actions = player_node.TotalNumChildren();
            PlayerInt player = player_node.PlayerIndex();
            info_set_tree_ptr_->AddInfoSetToTree(player_view_hash, num_actions, player);
            AddNodeSizeAtDepth(SearchTreeNode::SearchTreePlayerNodeSize(), depth);
        }
        void SearchTree::AddChanceNodeAtDepth(const GameState &chance_node, const int depth)
        {
            AddNodeSizeAtDepth(SearchTreeNode::SearchTreeChanceNodeSize(chance_node), depth);
        }
        void SearchTree::AddTerminalNodeAtDepth(const GameState &terminal_node, const int depth)
        {
            for (int i_player = 0; i_player < kNumPlayers; i_player++)
            {
                UpdateMaxUtility(terminal_node.TerminalUtility(i_player));
            }
            AddNodeSizeAtDepth(SearchTreeNode::SearchTreeTerminalNodeSize(), depth);
        }

        void SearchTree::AddNodeSizeAtDepth(uint64_t node_size, int depth)
        {
            search_tree_size_at_depth_[depth] += node_size;
        }

        void SearchTree::SetPlayerNodeAtDepth(const GameState &player_node, int depth)
        {
            if (depth == search_tree_depth_ - 1)
            {
                throw std::invalid_argument("Cannot set player node at final depth of search tree");
            }
            if (depth >= search_tree_depth_)
            {
                throw std::invalid_argument("Depth out of bounds");
            }
            Byte* curr_node_ptr = search_tree_ + search_tree_offset_at_depth_[depth];
            Byte* first_child_ptr = search_tree_ + search_tree_offset_at_depth_[depth + 1];
            Byte* info_set_ptr = info_set_tree_ptr_->InfoSetNodeAddress(player_node.PlayerViewHash());
            SearchTreeNode::SetPlayerSearchNodeInMem(player_node, curr_node_ptr, first_child_ptr, info_set_ptr);
            search_tree_offset_at_depth_[depth] += SearchTreeNode::SearchTreePlayerNodeSize();
        }

        void SearchTree::SetChanceNodeAtDepth(const GameState &chance_node, int depth)
        {
            if (depth == search_tree_depth_ - 1)
            {
                throw std::invalid_argument("Cannot set chance node at final depth of search tree");
            }
            if (depth >= search_tree_depth_)
            {
                throw std::invalid_argument("Depth out of bounds");
            }
            Byte* first_child_ptr = search_tree_ + search_tree_offset_at_depth_[depth + 1];
            Byte* curr_node_ptr = search_tree_ + search_tree_offset_at_depth_[depth];
            SearchTreeNode::SetChanceSearchNodeInMem(chance_node, curr_node_ptr, first_child_ptr);
            search_tree_offset_at_depth_[depth] += SearchTreeNode::SearchTreeChanceNodeSize(chance_node);
        }

        void SearchTree::SetTerminalNodeAtDepth(const GameState &terminal_node, int depth)
        {
            if (depth >= search_tree_depth_)
            {
                throw std::invalid_argument("Depth out of bounds");
            }
            Byte* curr_node_ptr = search_tree_ + search_tree_offset_at_depth_[depth];
            SearchTreeNode::SetTerminalSearchNodeInMem(terminal_node, curr_node_ptr, max_utility_);
            search_tree_offset_at_depth_[depth] += SearchTreeNode::SearchTreeTerminalNodeSize();
        }

        void SearchTree::UpdateOffsets()
        {
            unsigned long long curr_offset = 0;
            for (const unsigned long long size_at_depth : search_tree_size_at_depth_)
            {
                search_tree_offset_at_depth_.push_back(curr_offset);
                curr_offset += size_at_depth;
            }
        }

        void SearchTree::UpdateMaxUtility(float utility)
        {
            max_utility_ = std::max(max_utility_, utility);
        }
    }
}
