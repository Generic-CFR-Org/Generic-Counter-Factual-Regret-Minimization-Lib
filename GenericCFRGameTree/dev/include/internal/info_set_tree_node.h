//
// Created by Amir on 10/26/2024.
//

#ifndef INFO_SET_TREE_NODE_H
#define INFO_SET_TREE_NODE_H
#include "search_tree_node.h"

#endif //INFO_SET_TREE_NODE_H

#include "game_classes.h"
#include "new_tree_nodes.h"

namespace generic_cfr
{

    class InfoSetTree {

    private:
        std::unordered_map<int, long> regret_tree_size_at_depth_;
        std::vector<long long> info_set_tree_offset_at_depth_;
        int info_set_tree_depth_;
        long long info_set_tree_block_size_;
        long long info_set_tree_size_;

        std::vector<Byte> info_set_tree_;

    public:


        class InfoSetTreeNode;

        InfoSetTree();

        void AddInfoSetTable(Game::GameState&, int tree_depth);
        int TableIndex(Game::GameState&, int tree_depth);
        void SetInfoSetTables();

    };

    class InfoSetTree::InfoSetTreeNode
    {
        //TODO: Add Locking Mechanism for Thread-Safe Concurrency
    public:

        using NumActionsInt = uint16_t;
        using CumulativeProbInt = uint32_t;
        using CumulativeRegretInt = uint32_t;

        static constexpr uint8_t kNumChildrenSizeInMem = sizeof(NumActionsInt);
        static constexpr uint8_t kCurrActionProbSizeInMem = sizeof(NormalizedProbabilityInt);
        static constexpr uint8_t kCumulativeActionProbSizeInMem = sizeof(CumulativeProbInt);
        static constexpr uint8_t kCumulativeActionRegretSizeInMem = sizeof(CumulativeRegretInt);

        InfoSetTreeNode();

        static uint64_t InfoSetTreeNodeSize(const Game::GameState& &player_node);
        static uint64_t InfoSetTreeNodeSize(uint64_t num_player_node_actions);
        //TODO: Add SIMD version of SetInfoSetNodeInMem
        static void SetInfoSetNodeInMem(const Game::GameState& &player_node, Byte* mem_addr);

        NumActionsInt NumActions() const;

        NormalizedProbabilityInt CurrentActionProbability(uint64_t action_index) const;
        CumulativeProbInt CumulativeActionProbability(uint64_t action_index) const;
        CumulativeRegretInt CumulativeActionRegret(uint64_t action_index) const;

        void SetCurrentActionProbability(uint64_t action_index, NormalizedProbabilityInt action_probability) const;
        void AddToCumulativeActionProbability(uint64_t action_index, CumulativeProbInt action_probability) const;
        void AddToCumulativeActionRegret(uint64_t action_index, CumulativeRegretInt action_regret) const;

        //TODO: Add SIMD version of SetCurrentActionProbability
        //TODO: Add SIMD version of AddToCumulativeActionProbability
        //TODO: Add SIMD version of AddToCumulativeActionRegret

    private:

        Byte* GetCurrentActionProbabilityPtr(uint64_t action_index) const;
        Byte* GetCumulativeActionProbabilityPtr(uint64_t action_index) const;
        Byte* GetCumulativeActionRegretPtr(uint64_t action_index) const;

    };
}