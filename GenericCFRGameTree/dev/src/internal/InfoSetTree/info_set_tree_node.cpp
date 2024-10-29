//
// Created by Amir on 10/26/2024.
//

#include <stdexcept>
#include <iostream>

#include "tree_node_utils.h"
#include "float_normalization.h"
#include "info_set_tree.h"

namespace Internal
{
    namespace Trees
    {
        uint64_t InfoSetTreeNode::InfoSetTreeNodeSize(uint64_t num_player_node_actions)
        {
            return kNumChildrenSizeInMem + kPlayerSizeInMem + num_player_node_actions *
                (kCurrActionProbSizeInMem + kCumulativeActionProbSizeInMem + kCumulativeActionRegretSizeInMem);
        }

        uint64_t InfoSetTreeNode::InfoSetTreeNodeSize(const GameState& player_node)
        {
            if (!player_node.IsPlayerNode())
            {
                throw std::invalid_argument("GameState must be a player node");
            }
            return InfoSetTreeNodeSize(player_node.PlayerActions().size());
        }

        void InfoSetTreeNode::SetInfoSetNodeInMem(
            uint64_t num_actions, Byte* mem_addr, PlayerInt player)
        {


            SetValInMem<NumActionsInt>(mem_addr += sizeof(NumActionsInt), num_actions);
            SetValInMem<PlayerInt>(mem_addr += sizeof(PlayerInt), player);

            const float uniform_prob = 1.0f / static_cast<float>(num_actions);
            const NormalizedProbabilityInt normalized_uniform_prob = normalize_probability(uniform_prob);

            Byte* curr_prob_ptr = mem_addr;
            Byte* cumulative_prob_ptr = curr_prob_ptr + num_actions * kCurrActionProbSizeInMem;
            Byte* cumulative_regret_ptr = cumulative_prob_ptr + num_actions * kCumulativeActionProbSizeInMem;
            for (int child_index = 0; child_index < num_actions; ++child_index)
            {
                SetValInMem<NormalizedProbabilityInt>(curr_prob_ptr + child_index * kCurrActionProbSizeInMem, normalized_uniform_prob);
                SetValInMem<CumulativeProbInt>(curr_prob_ptr + child_index * kCumulativeActionProbSizeInMem, 0);
                SetValInMem<CumulativeRegretInt>(cumulative_regret_ptr + child_index * kCumulativeActionRegretSizeInMem, 0);
            }
        }

        NumActionsInt InfoSetTreeNode::NumActions() const
        {
            try
            {
                const Byte* mem_ptr = reinterpret_cast<const Byte*>(this);
                return GetValFromMem<NumActionsInt>(mem_ptr);
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                             " retrieve Number of Actions for an Information Set Node";
                std::cerr << e.what() << std::endl;
                throw;
            }
        }

        PlayerInt InfoSetTreeNode::Player() const
        {
            try
            {
                const Byte* mem_ptr = reinterpret_cast<const Byte*>(this);
                return GetValFromMem<PlayerInt>(mem_ptr + kNumChildrenSizeInMem);
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                             " retrieve Player for an Information Set Node";
                std::cerr << e.what() << std::endl;
                throw;
            }
        }


        NormalizedProbabilityInt InfoSetTreeNode::CurrentActionProbability(uint64_t action_index) const
        {
            NumActionsInt num_actions = NumActions();
            if (action_index >= num_actions)
            {
                throw std::invalid_argument("Action index out of range");
            }
            try
            {
                const Byte* action_curr_prob_ptr = GetCurrentActionProbabilityPtr(action_index);
                return GetValFromMem<NumActionsInt>(action_curr_prob_ptr);
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                             " retrieve Current Action Probability for an Information Set Node"
                             " at index: " << action_index << " : ";
                std::cerr << e.what() << std::endl;
                throw;
            }
        }

        CumulativeProbInt InfoSetTreeNode::CumulativeActionProbability(uint64_t action_index) const
        {
            try
            {
                const Byte* action_cum_prob_ptr = GetCumulativeActionProbabilityPtr(action_index);
                return GetValFromMem<NumActionsInt>(action_cum_prob_ptr);
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                             " retrieve Cumulative Action Probability for an Information Set Node"
                             " at index: " << action_index << " : ";
                std::cerr << e.what() << std::endl;
                throw;
            }
        }

        CumulativeRegretInt InfoSetTreeNode::CumulativeActionRegret(uint64_t action_index) const
        {
            try
            {

                const Byte* action_cum_regret_ptr = GetCumulativeActionRegretPtr(action_index);
                return GetValFromMem<NumActionsInt>(action_cum_regret_ptr);
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                             " retrieve Cumulative Action Regret for an Information Set Node"
                             " at index: " << action_index << " : ";
                std::cerr << e.what() << std::endl;
                throw;
            }
        }

        void InfoSetTreeNode::SetCurrentActionProbability(
            const uint64_t action_index, const NormalizedProbabilityInt action_probability) const
        {
            try
            {
                Byte* action_curr_prob_ptr = GetCurrentActionProbabilityPtr(action_index);
                SetValInMem<NumActionsInt>(action_curr_prob_ptr, action_probability);
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                             " Set Current Action Probability for an Information Set Node"
                             " at index: " << action_index
                            << " with Probability: " << action_probability << " : ";
                std::cerr << e.what() << std::endl;
            }
        }

        void InfoSetTreeNode::AddToCumulativeActionProbability(
            const uint64_t action_index, const CumulativeProbInt action_probability) const
        {
            try
            {
                Byte* action_curr_prob_ptr = GetCurrentActionProbabilityPtr(action_index);
                AddValInMem<NumActionsInt>(action_curr_prob_ptr, action_probability);
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                             " Add Cumulative Action Probability for an Information Set Node"
                             " at index: " << action_index
                            << " with Probability: " << action_probability << " : ";
                std::cerr << e.what() << std::endl;
            }
        }

        void InfoSetTreeNode::AddToCumulativeActionRegret(
            const uint64_t action_index, const CumulativeRegretInt action_regret) const
        {
            try
            {
                Byte* action_cum_regret_ptr = GetCumulativeActionRegretPtr(action_index);
                AddValInMem<NumActionsInt>(action_cum_regret_ptr, action_regret);
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                             " Add Cumulative Action Regret for an Information Set Node"
                             " at index: " << action_index
                            << " with regret: " << action_regret << " : ";
                std::cerr << e.what() << std::endl;
            }
        }


        Byte *InfoSetTreeNode::GetCurrentActionProbabilityPtr(uint64_t action_index) const
        {
            NumActionsInt num_actions = NumActions();
            if (action_index >= num_actions)
            {
                throw std::invalid_argument("Action index out of range");
            }
            Byte* start_mem_ptr = const_cast<Byte*>(reinterpret_cast<const Byte*>(this));
            Byte* action_curr_prob_ptr = start_mem_ptr + kNumChildrenSizeInMem + kPlayerSizeInMem + action_index * kCurrActionProbSizeInMem;
            return action_curr_prob_ptr;
        }

        Byte *InfoSetTreeNode::GetCumulativeActionProbabilityPtr(uint64_t action_index) const
        {
            NumActionsInt num_actions = NumActions();
            if (action_index >= num_actions)
            {
                throw std::invalid_argument("Action index out of range");
            }
            Byte* start_mem_ptr = const_cast<Byte*>(reinterpret_cast<const Byte*>(this));
            Byte* action_cum_prob_ptr = start_mem_ptr + kNumChildrenSizeInMem + kPlayerSizeInMem
                    + num_actions * kCurrActionProbSizeInMem + action_index * kCumulativeActionProbSizeInMem;
            return action_cum_prob_ptr;
        }


        Byte *InfoSetTreeNode::GetCumulativeActionRegretPtr(uint64_t action_index) const
        {
            NumActionsInt num_actions = NumActions();
            if (action_index >= num_actions)
            {
                throw std::invalid_argument("Action index out of range");
            }
            Byte* start_mem_ptr = const_cast<Byte*>(reinterpret_cast<const Byte*>(this));
            Byte* action_cum_regret_ptr = start_mem_ptr + kNumChildrenSizeInMem + kPlayerSizeInMem
                    + num_actions * (kCurrActionProbSizeInMem + kCumulativeActionProbSizeInMem)
                    + action_index * kCumulativeActionRegretSizeInMem;
            return action_cum_regret_ptr;
        }
    }
}


