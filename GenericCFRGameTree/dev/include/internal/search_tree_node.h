//
// Created by Amir on 10/23/2024.
//
#pragma once
#ifndef SEARCH_TREE_NODE_H
#define SEARCH_TREE_NODE_H

#endif //SEARCH_TREE_NODE_H

#include "game_classes.h"
#include "new_tree_nodes.h"
#include <limits>
namespace generic_cfr
{

    using NormalizedProbabilityInt = uint8_t;
    using TerminalUtilityInt = uint8_t;

    constexpr NormalizedProbabilityInt kMaxChanceProbabilityVal = std::numeric_limits<NormalizedProbabilityInt>::max();
    constexpr TerminalUtilityInt kMaxTerminalUtilityVal = std::numeric_limits<TerminalUtilityInt>::max();

    /**
    * @brief Size of a chance node's 'probability to reach child' in memory
    */
    static constexpr uint8_t kProbabilityMemSize = sizeof(NormalizedProbabilityInt);
    /**
     * @brief Size of a terminal node's utility in memory
     */
    static constexpr uint8_t kTerminalUtilitySize = sizeof(TerminalUtilityInt);


    class SearchTree {

    private:
        std::unordered_map<int, long> search_tree_size_at_depth_;
        std::vector<long long> search_tree_offset_at_depth_;
        int search_tree_depth_;
        long long search_tree_size_;

        Byte* search_tree_;


    public:

        class SearchTreeNode;
        class PlayerSearchNode;
        class ChanceSearchNode;
        class TerminalSearchNode;

        SearchTree();

        long long AddNodeSizeAtDepth(int depth);
        void SetSearchNodes();

    };

    class SearchTree::SearchTreeNode
    {

    public:

        using NumChildrenInt = uint8_t;
        using ChildNodePtr = SearchTreeNode*;
        using InfoSetPtr = void*;

        /**
        * @brief Size of a node's 'number of children of some type' in memory
        */
        static constexpr uint8_t kNumChildrenMemSize = sizeof(NumChildrenInt);
        /**
         * @brief Size of a player node's first child's address in memory
         */
        static constexpr uint8_t kChildPtrMemSize = sizeof(ChildNodePtr);
        /**
         * @brief Size of a player node's associated Information Set address in memory
         */
        static constexpr uint8_t kInfoSetPtrMemSize = sizeof(InfoSetPtr);

        SearchTreeNode() {}

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
        static uint64_t SearchTreeChanceNodeSize(const Game::GameState &chance_node);
        static uint64_t SearchTreeTerminalNodeSize();

        static uint64_t SearchTreeNodeSize(const Game::GameState &state);

        //TODO: Replace void* with appropriate types
        /**
         * @brief Sets player search tree node at mem_addr.
         * @param player_node GameState representation of the player node
         * @param mem_addr Location in memory to set the Player Search Tree Node
         * @param first_child_addr Location of a Player Node's first child in the Search Tree.
         * @param info_set_addr Location of a PLayer Node's associated information set in the Information Set Tree.
         */
        static void SetPlayerSearchNodeInMem(const Game::GameState& player_node, Byte* mem_addr, ChildNodePtr first_child_addr, InfoSetPtr info_set_addr);

        /**
         * @brief Sets chance search tree node at mem_addr.
         * @param chance_node GameState representation of the chance node
         * @param mem_addr Location in memory to set the Chance Search Tree Node
         * @param first_child_addr Location of a Chance Node's first child in the Search Tree.
         */
        static void SetChanceSearchNodeInMem(const Game::GameState& chance_node, Byte* mem_addr, ChildNodePtr first_child_addr);

        /**
         * @brief Sets Terminal search tree node at mem_addr.
         * @param terminal_node GameState representation of the terminal node
         * @param mem_addr Location in memory to set the Terminal Search Tree Node
         * @param max_utility
         */
        static void SetTerminalSearchNodeInMem(const Game::GameState &terminal_node, Byte *mem_addr, float max_utility);

        /**
         * @returns Address of the current node's neighbour in the search tree (in ascending order)
         */
        SearchTreeNode* NextNodeAddr();

        /**
         * @returns Total number of children for a given search tree node.
         */
        int GetTotalNumChildren() const;

        /**
         * @returns Vector of the addresses of each of the current node's children.
         */
        std::vector<SearchTreeNode*> GetChildren() const;

    };

    class SearchTree::PlayerSearchNode : public SearchTreeNode
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


    class SearchTree::ChanceSearchNode : public SearchTreeNode
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
        ChildNodePtr GetRandomChildPtr() const;

    };

    class SearchTree::TerminalSearchNode : public SearchTreeNode
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
}