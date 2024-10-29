//
// Created by Amir on 10/26/2024.
//
#pragma once

#ifndef SEARCH_TREE_H
#define SEARCH_TREE_H

#pragma once
#include <unordered_map>
#include "internal.h"
#include "info_set_tree.h"
#include <cstdint>

namespace Internal
{
    namespace Trees
    {
        class SearchTree {

        public:

            /**
            * @brief Size of a chance node's 'probability to reach child' in memory
            */
            static constexpr uint8_t kProbabilityMemSize = sizeof(NormalizedProbabilityInt);
            /**
             * @brief Size of a terminal node's utility in memory
             */
            static constexpr uint8_t kTerminalUtilitySize = sizeof(TerminalUtilityInt);


            SearchTree();
            explicit SearchTree(InfoSetTree* info_set_tree_ptr);

            void ConstructBothTrees(const GameState& root_game_state);

            Byte* SearchTreePtr() const;
            InfoSetTree* InfoSetDataPtr() const;
            unsigned long long Size() const;


        private:
            std::vector<unsigned long long> search_tree_size_at_depth_;
            std::vector<unsigned long long> search_tree_offset_at_depth_;
            int search_tree_depth_;
            unsigned long long search_tree_size_;
            float max_utility_;
            InfoSetTree* info_set_tree_ptr_;

            Byte* search_tree_;

            void ExploreGameStateStageOne(const GameState& game_state, int curr_depth);
            void ExploreGameStateStageTwo(const GameState& game_state, int curr_depth);

            void AddGameStateAtDepth(const GameState& game_state, int depth);
            void AllocateSearchTreeMem();
            void SetGameStateInMem(const GameState& game_state, int depth);

            void AddPlayerNodeAtDepth(const GameState &player_node, int depth);
            void AddChanceNodeAtDepth(const GameState& chance_node, int depth);
            void AddTerminalNodeAtDepth(const GameState &terminal_node, int depth);
            void AddNodeSizeAtDepth(uint64_t node_size, int depth);

            void SetPlayerNodeAtDepth(const GameState& player_node, int depth);
            void SetChanceNodeAtDepth(const GameState& chance_node, int depth);
            void SetTerminalNodeAtDepth(const GameState& terminal_node, int depth);

            void UpdateMaxUtility(float utility);
            void UpdateOffsets();

        };

        class SearchTreeNode
        {
        public:
            virtual ~SearchTreeNode() = default;

            using ChildNodePtr = SearchTreeNode*;
            using InfoSetPtr = void*;

            /**
             * @brief Size of a player node's first child's address in memory
             */
            static constexpr uint8_t kChildPtrMemSize = sizeof(ChildNodePtr);
            /**
             * @brief Size of a player node's associated Information Set address in memory
             */
            static constexpr uint8_t kInfoSetPtrMemSize = sizeof(InfoSetPtr);

            SearchTreeNode() = default;

            /**
             * @returns size of SearchTreeNode stored in memory in bytes.
             */
            virtual uint64_t SearchTreeNodeSize() const = 0;

            /**
             * @returns number of children that are player search tree nodes.
             */
            virtual NumChildrenInt GetNumPlayerNodeChildren() const = 0;

            /**
             * @returns number of children that are chance search tree nodes.
             */
            virtual NumChildrenInt GetNumChanceNodeChildren() const = 0;

            /**
             * @returns number of children that are terminal search tree nodes.
             */
            virtual NumChildrenInt GetNumTerminalNodeChildren() const = 0;

            /**
             * @returns address of the node's first child located at the next depth of the search tree.
             */
            virtual ChildNodePtr GetFirstChildAddr() const = 0;

            static uint64_t SearchTreePlayerNodeSize();
            static uint64_t SearchTreeChanceNodeSize(const GameState &chance_node);
            static uint64_t SearchTreeTerminalNodeSize();

            static uint64_t SearchTreeNodeSize(const GameState &state);

            //TODO: Replace void* with appropriate types
            /**
             * @brief Sets player search tree node at mem_addr.
             * @param player_node GameState representation of the player node
             * @param mem_addr Location in memory to set the Player Search Tree Node
             * @param first_child_addr Location of a Player Node's first child in the Search Tree.
             * @param info_set_addr Location of a PLayer Node's associated information set in the Information Set Tree.
             */
            static void SetPlayerSearchNodeInMem(const GameState& player_node, Byte* mem_addr, Byte *first_child_addr, Byte *info_set_addr);

            /**
             * @brief Sets chance search tree node at mem_addr.
             * @param chance_node GameState representation of the chance node
             * @param mem_addr Location in memory to set the Chance Search Tree Node
             * @param first_child_addr Location of a Chance Node's first child in the Search Tree.
             */
            static void SetChanceSearchNodeInMem(const GameState& chance_node, Byte* mem_addr, Byte *first_child_addr);

            /**
             * @brief Sets Terminal search tree node at mem_addr.
             * @param terminal_node GameState representation of the terminal node
             * @param mem_addr Location in memory to set the Terminal Search Tree Node
             * @param max_utility
             */
            static void SetTerminalSearchNodeInMem(const GameState &terminal_node, Byte *mem_addr, float max_utility);

            /**
             * @returns total number of children.
             */
            NumChildrenInt GetTotalNumChildren() const;

            /**
             * @returns Address of the current node's neighbour in the search tree (in ascending order)
             */
            SearchTreeNode* NextNodeAddr();

            /**
             * @returns Vector of the addresses of each of the current node's children.
             */
            std::vector<SearchTreeNode*> GetChildren() const;

        };

        class PlayerSearchNode final : public SearchTreeNode
        {
        public:
            uint64_t SearchTreeNodeSize() const override;

            NumChildrenInt GetNumPlayerNodeChildren() const override;

            NumChildrenInt GetNumChanceNodeChildren() const override;

            NumChildrenInt GetNumTerminalNodeChildren() const override;

            ChildNodePtr GetFirstChildAddr() const override;

            /**
             * @returns The address of a Search Tree Player Node's associated Information Set
             */
            InfoSetPtr GetInfoSetPtr() const;

        };

        class ChanceSearchNode final : public SearchTreeNode
        {
        public:
            uint64_t SearchTreeNodeSize() const override;

            NumChildrenInt GetNumPlayerNodeChildren() const override;

            NumChildrenInt GetNumChanceNodeChildren() const override;

            NumChildrenInt GetNumTerminalNodeChildren() const override;

            ChildNodePtr GetFirstChildAddr() const override;

            /**
             * @returns vector of the chance node's normalized probability of reaching a child, for each child.
             */
            std::vector<NormalizedProbabilityInt> GetNormalizedProbabilities() const;

            /**
             * @returns the address of a randomly chosen child, based on a chance node's children probability distribution.
             */
            uint16_t GetRandomChildIndex() const;

        };

        class TerminalSearchNode final : public SearchTreeNode
        {
        public:
            uint64_t SearchTreeNodeSize() const override;

            NumChildrenInt GetNumPlayerNodeChildren() const override;

            NumChildrenInt GetNumChanceNodeChildren() const override;

            NumChildrenInt GetNumTerminalNodeChildren() const override;

            ChildNodePtr GetFirstChildAddr() const override;

            /**
             * @returns the terminal node's normalized probability.
             */
            TerminalUtilityInt GetNormalizedUtility(uint64_t player_index) const;

        };
        std::ostream& operator<<(std::ostream& os, const SearchTreeNode& search_tree_node);
    }
}

#endif //SEARCH_TREE_H
