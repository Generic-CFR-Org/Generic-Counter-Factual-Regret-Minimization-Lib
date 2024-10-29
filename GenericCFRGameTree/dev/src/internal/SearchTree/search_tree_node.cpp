//
// Created by Amir on 10/23/2024.
//

#include <stdexcept>
#include <string>
#include "../../../include/internal/search_tree.h"

#include <iostream>

#include "tree_node_utils.h"
#include "float_normalization.h"

namespace Internal
{
    namespace Trees
    {
        uint64_t SearchTreeNode::SearchTreePlayerNodeSize()
        {
            return kNumChildrenMemSize * 3 + kChildPtrMemSize + kInfoSetPtrMemSize;
        }

        uint64_t SearchTreeNode::SearchTreeChanceNodeSize(const GameState &chance_node)
        {
            return kNumChildrenMemSize * 3 + kChildPtrMemSize + kProbabilityMemSize * chance_node.TotalNumChildren();
        }

        uint64_t SearchTreeNode::SearchTreeTerminalNodeSize()
        {
            return kNumPlayers * kTerminalUtilitySize;
        }

        uint64_t SearchTreeNode::SearchTreeNodeSize(const GameState &game_state)
        {
            if (game_state.IsPlayerNode())
            {
                return SearchTreePlayerNodeSize();
            }
            if (game_state.IsChanceNode())
            {
                return SearchTreeChanceNodeSize(game_state);
            }
            if (game_state.IsTerminalNode())
            {
                return SearchTreeTerminalNodeSize();
            }
            throw std::invalid_argument("Game state is not a Player, Chance, or Terminal Node");
        }

        NumChildrenInt SearchTreeNode::GetTotalNumChildren() const
        {
            return GetNumPlayerNodeChildren() + GetNumChanceNodeChildren() + GetNumTerminalNodeChildren();
        }


        SearchTreeNode *SearchTreeNode::NextNodeAddr()
        {
            /** @brief Convert to void pointer for correct pointer arithmetic */
            Byte* curr_addr = reinterpret_cast<Byte*>(this);
            return reinterpret_cast<SearchTreeNode*>(curr_addr + SearchTreeNodeSize());
        }



        std::vector<SearchTreeNode*> SearchTreeNode::GetChildren() const
        {
            {
                void *curr_child = GetFirstChildAddr();
                std::vector<SearchTreeNode*> children;
                /** @brief Adds all player search tree nodes to children*/
                for (int i_player_child = 0; i_player_child < GetNumPlayerNodeChildren(); i_player_child++)
                {
                    /** @brief casts curr_child to a player search_node pointer
                     * to add to the children vector and find the next node*/
                    auto* player_search_node_child = static_cast<PlayerSearchNode*>(curr_child);
                    children.push_back(player_search_node_child);
                    try
                    {
                        curr_child = player_search_node_child->NextNodeAddr();
                    }
                    catch (std::invalid_argument &e)
                    {
                        std::string node_ptr_as_string = std::to_string(reinterpret_cast<unsigned long long>(reinterpret_cast<void **>(player_search_node_child)));
                        std::cerr << node_ptr_as_string << " is not a valid SearchTreeNode Pointer: " << e.what() << std::endl;
                    }
                }
                /** @brief Adds all chance search tree nodes to children*/
                for (int i_chance_child = 0; i_chance_child < GetNumChanceNodeChildren(); i_chance_child++)
                {
                    /** @brief casts curr_child to a chance search_node pointer
                     * to add to the children vector and find the next node*/
                    auto* chance_search_node_child = static_cast<ChanceSearchNode *>(curr_child);
                    children.push_back(chance_search_node_child);
                    try
                    {
                        curr_child = chance_search_node_child->NextNodeAddr();
                    }
                    catch (std::invalid_argument &e)
                    {
                        std::string node_ptr_as_string = std::to_string(reinterpret_cast<unsigned long long>(reinterpret_cast<void **>(chance_search_node_child)));
                        std::cerr << node_ptr_as_string << " is not a valid SearchTreeNode Pointer: " << e.what() << std::endl;
                    }
                }
                /** @brief Adds all terminal search tree nodes to children*/
                for (int i_terminal_child = 0; i_terminal_child < GetNumTerminalNodeChildren(); i_terminal_child++)
                {
                    /** @brief casts curr_child to a terminal search node pointer
                     * to add to the children vector and find the next node*/
                    auto* terminal_search_node_child = static_cast<TerminalSearchNode*>(curr_child);
                    children.push_back(terminal_search_node_child);
                    try
                    {
                        curr_child = terminal_search_node_child->NextNodeAddr();
                    }
                    catch (std::invalid_argument &e)
                    {
                        std::string node_ptr_as_string = std::to_string(reinterpret_cast<unsigned long long>(reinterpret_cast<void **>(terminal_search_node_child)));
                        std::cerr << node_ptr_as_string << " is not a valid SearchTreeNode Pointer: " << e.what() << std::endl;
                    }
                }
                return children;
            }
        }

        void SearchTreeNode::SetPlayerSearchNodeInMem(
            const GameState& player_node, Byte *mem_addr,
            Byte *first_child_addr, Byte *info_set_addr)
        {
            CFR::ClientGame::GameStateTypeCnt children_type_cnt = player_node.NumChildrenOfEachType();
            try
            {
                SetValInMem<NumChildrenInt>(mem_addr += kNumChildrenMemSize, std::get<0>(children_type_cnt));
                SetValInMem<NumChildrenInt>(mem_addr += kNumChildrenMemSize, std::get<1>(children_type_cnt));
                SetValInMem<NumChildrenInt>(mem_addr += kNumChildrenMemSize, std::get<2>(children_type_cnt));
                SetValInMem<ChildNodePtr>(mem_addr += kChildPtrMemSize, reinterpret_cast<ChildNodePtr>(first_child_addr));
                SetValInMem<InfoSetPtr>(mem_addr += kInfoSetPtrMemSize, reinterpret_cast<InfoSetPtr>(info_set_addr));
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Bounds Error when setting Player Search Node in memory: " << e.what() << std::endl;
            }
        }

        void SearchTreeNode::SetChanceSearchNodeInMem(
            const GameState &chance_node,
            Byte *mem_addr, Byte *first_child_addr)
        {
            CFR::ClientGame::GameStateTypeCnt children_type_cnt = chance_node.NumChildrenOfEachType();
            try
            {
                SetValInMem<NumChildrenInt>(mem_addr += kNumChildrenMemSize, std::get<0>(children_type_cnt));
                SetValInMem<NumChildrenInt>(mem_addr += kNumChildrenMemSize, std::get<1>(children_type_cnt));
                SetValInMem<NumChildrenInt>(mem_addr += kNumChildrenMemSize, std::get<2>(children_type_cnt));
                SetValInMem<ChildNodePtr>(mem_addr += kChildPtrMemSize, reinterpret_cast<ChildNodePtr>(first_child_addr));

                CFR::ClientGame::ProbabilityList chance_node_prob_list = chance_node.SortedChanceProbabilityList();
                const std::vector<NormalizedProbabilityInt> normalized_prob_list = normalize_probabilities(chance_node_prob_list);
                for (const NormalizedProbabilityInt normalized_prob : normalized_prob_list)
                {
                    SetValInMem<NormalizedProbabilityInt>(mem_addr += kProbabilityMemSize, normalized_prob);
                }
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Bounds Error when setting Chance Search Node in memory: " << e.what() << std::endl;
            }
        }


        void SearchTreeNode::SetTerminalSearchNodeInMem(
            const GameState &terminal_node, Byte *mem_addr, const float max_utility)
        {
            try
            {
                for (int i_player = 0; i_player < kNumPlayers; i_player++)
                {
                    SetValInMem<TerminalUtilityInt>(mem_addr += kTerminalUtilitySize,
                    normalize_utility(terminal_node.TerminalUtility(i_player), max_utility));
                }
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Bounds Error when setting Terminal Search Node in memory: " << e.what() << std::endl;
            }

        }

        std::ostream& operator<<(std::ostream& os, const PlayerSearchNode& player_search_node)
        {
            os << "Search Tree Player node:" << std::endl;
            os << "Num Player Node Children: " << player_search_node.GetNumPlayerNodeChildren() << std::endl;
            os << "Num Chance Node Children: " << player_search_node.GetNumChanceNodeChildren() << std::endl;
            os << "Num Terminal Node Children: " << player_search_node.GetNumTerminalNodeChildren() << "\n" << std::endl;
            return os;
        }

        std::ostream& operator<<(std::ostream& os, const ChanceSearchNode& chance_search_node)
        {
            os << "Search Tree Chance Node:" << std::endl;
            os << "Num Player Node Children: " << chance_search_node.GetNumPlayerNodeChildren() << std::endl;
            os << "Num Chance Node Children: " << chance_search_node.GetNumChanceNodeChildren() << std::endl;
            os << "Num Terminal Node Children: " << chance_search_node.GetNumTerminalNodeChildren() << std::endl;
            os << "Normalized Probabilities: [ ";
            for (uint8_t normalized_float : chance_search_node.GetNormalizedProbabilities())
            {
                os << normalized_float << ", ";
            }
            os << "]\n" << std::endl;
            return os;
        }

        std::ostream& operator<<(std::ostream& os, const TerminalSearchNode& terminal_search_node)
        {
            os << "Search Tree Terminal Node:" << std::endl;
            os << "Utilities: { ";
            for (int i_player = 0; i_player < kNumPlayers; i_player++)
            {
                os << "player " << i_player << ": ";
                os << terminal_search_node.GetNormalizedUtility(i_player) << ", ";
            }
            return os;
        }
    }
}