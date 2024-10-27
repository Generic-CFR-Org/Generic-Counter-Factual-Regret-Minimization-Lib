//
// Created by Amir on 10/26/2024.
//

#include "info_set_tree_node.h"

#include <stdexcept>
#include <iostream>

#include "search_tree_node.h"
#include "tree_node_utils.h"
#include "float_normalization.h"

namespace generic_cfr
{
    uint64_t InfoSetTree::InfoSetTreeNode::InfoSetTreeNodeSize(uint64_t num_player_node_actions)
    {
        return kNumChildrenSizeInMem + num_player_node_actions *
            (kCurrActionProbSizeInMem + kCumulativeActionProbSizeInMem + kCumulativeActionRegretSizeInMem);
    }

    uint64_t InfoSetTree::InfoSetTreeNode::InfoSetTreeNodeSize(const Game::GameState& player_node)
    {
        if (!player_node.IsPlayerNode())
        {
            throw std::invalid_argument("GameState must be a player node");
        }
        return InfoSetTreeNodeSize(player_node.PlayerActions().size());
    }

    void InfoSetTree::InfoSetTreeNode::SetInfoSetNodeInMem(
        const Game::GameState& player_node, Byte* mem_addr)
    {
        if (!player_node.IsPlayerNode())
        {
            throw std::invalid_argument("GameState must be a player node");
        }
        NumActionsInt total_num_children = player_node.PlayerActions().size();
        SetValInMem<NumActionsInt>(mem_addr += sizeof(NumActionsInt), total_num_children);

        const float uniform_prob = 1.0f / static_cast<float>(total_num_children);
        const NormalizedProbabilityInt normalized_uniform_prob = normalize_probability(uniform_prob);

        Byte* curr_prob_ptr = mem_addr;
        Byte* cumulative_prob_ptr = curr_prob_ptr + total_num_children * kCurrActionProbSizeInMem;
        Byte* cumulative_regret_ptr = cumulative_prob_ptr + total_num_children * kCumulativeActionProbSizeInMem;
        for (int child_index = 0; child_index < total_num_children; ++child_index)
        {
            SetValInMem<NormalizedProbabilityInt>(curr_prob_ptr + child_index * kCurrActionProbSizeInMem, normalized_uniform_prob);
            SetValInMem<CumulativeProbInt>(curr_prob_ptr + child_index * kCumulativeActionProbSizeInMem, 0);
            SetValInMem<CumulativeRegretInt>(cumulative_regret_ptr + child_index * kCumulativeActionRegretSizeInMem, 0);
        }
    }

    InfoSetTree::InfoSetTreeNode::NumActionsInt
    InfoSetTree::InfoSetTreeNode::NumActions() const
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
        }
    }

    NormalizedProbabilityInt
    InfoSetTree::InfoSetTreeNode::CurrentActionProbability(uint64_t action_index) const
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
        }
    }

    InfoSetTree::InfoSetTreeNode::CumulativeProbInt
    InfoSetTree::InfoSetTreeNode::CumulativeActionProbability(uint64_t action_index) const
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
        }
    }

    InfoSetTree::InfoSetTreeNode::CumulativeRegretInt
    InfoSetTree::InfoSetTreeNode::CumulativeActionRegret(uint64_t action_index) const
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
        }
    }

    void InfoSetTree::InfoSetTreeNode::SetCurrentActionProbability(
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

    void InfoSetTree::InfoSetTreeNode::AddToCumulativeActionProbability(
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

    void InfoSetTree::InfoSetTreeNode::AddToCumulativeActionRegret(
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


    Byte *InfoSetTree::InfoSetTreeNode::GetCurrentActionProbabilityPtr(uint64_t action_index) const
    {
        NumActionsInt num_actions = NumActions();
        if (action_index >= num_actions)
        {
            throw std::invalid_argument("Action index out of range");
        }
        Byte* start_mem_ptr = const_cast<Byte*>(reinterpret_cast<const Byte*>(this));
        Byte* action_curr_prob_ptr = start_mem_ptr + kNumChildrenSizeInMem + action_index * kCurrActionProbSizeInMem;
        return action_curr_prob_ptr;
    }

    Byte *InfoSetTree::InfoSetTreeNode::GetCumulativeActionProbabilityPtr(uint64_t action_index) const
    {
        NumActionsInt num_actions = NumActions();
        if (action_index >= num_actions)
        {
            throw std::invalid_argument("Action index out of range");
        }
        Byte* start_mem_ptr = const_cast<Byte*>(reinterpret_cast<const Byte*>(this));
        Byte* action_cum_prob_ptr = start_mem_ptr + kNumChildrenSizeInMem
                + num_actions * kCurrActionProbSizeInMem + action_index * kCumulativeActionProbSizeInMem;
        return action_cum_prob_ptr;
    }


    Byte *InfoSetTree::InfoSetTreeNode::GetCumulativeActionRegretPtr(uint64_t action_index) const
    {
        NumActionsInt num_actions = NumActions();
        if (action_index >= num_actions)
        {
            throw std::invalid_argument("Action index out of range");
        }
        Byte* start_mem_ptr = const_cast<Byte*>(reinterpret_cast<const Byte*>(this));
        Byte* action_cum_regret_ptr = start_mem_ptr + kNumChildrenSizeInMem
                + num_actions * (kCurrActionProbSizeInMem + kCumulativeActionProbSizeInMem)
                + action_index * kCumulativeActionRegretSizeInMem;
        return action_cum_regret_ptr;
    }

}

