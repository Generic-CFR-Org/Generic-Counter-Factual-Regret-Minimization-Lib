//
// Created by Amir on 10/26/2024.
//
#pragma once

#ifndef INFO_SET_TREE_H
#define INFO_SET_TREE_H

#include <unordered_map>
#include "internal.h"

namespace Internal
{
    namespace Trees
    {
        class InfoSetTree {
        public:
            InfoSetTree();

            Byte* InfoSetPtr() const;
            void AddInfoSetToTree(const std::string& info_set_hash, uint16_t num_actions, PlayerInt player);
            Byte* InfoSetNodeAddress(const std::string& info_set_hash);
            void ConstructInfoSetTree();
            unsigned long long Size() const;

        private:

            std::vector<std::string> info_set_hash_LIFO_;
            std::unordered_map<std::string, unsigned long long> info_set_size_map_;
            std::unordered_map<std::string, uint8_t> info_set_player_map_;
            std::unordered_map<std::string, unsigned long long> info_set_offset_map_;
            unsigned long long info_set_tree_size_;
            unsigned long long bytes_set_;

            Byte* info_set_tree_;

            void AllocateInfoSetMem();
            void SetInfoSetNodeInMem(uint16_t num_actions, PlayerInt player);
            void SetAllInfoSetsInMem();
        };

        class InfoSetTreeNode {
        //TODO: Add Locking Mechanism for Thread-Safe Concurrency
        public:
            static uint64_t InfoSetTreeNodeSize(const GameState& player_node);
            static uint64_t InfoSetTreeNodeSize(uint64_t num_player_node_actions);
            //TODO: Add SIMD version of SetInfoSetNodeInMem
            static void SetInfoSetNodeInMem(uint64_t num_actions, Byte* mem_addr, uint8_t player);

            NumActionsInt NumActions() const;
            PlayerInt Player() const;

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
}

#endif //INFO_SET_TREE_H
