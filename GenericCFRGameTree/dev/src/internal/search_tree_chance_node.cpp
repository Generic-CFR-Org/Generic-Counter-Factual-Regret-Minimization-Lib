//
// Created by Amir on 10/26/2024.
//

#include <stdexcept>

#include "search_tree_node.h"
#include "tree_node_utils.h"
#include <iostream>

namespace generic_cfr
{
    uint64_t SearchTree::ChanceSearchNode::SearchTreeNodeSize() const
    {
        return kNumChildrenMemSize * 3 + kChildPtrMemSize + kProbabilityMemSize * GetTotalNumChildren();
    }

    SearchTree::SearchTreeNode::NumChildrenInt SearchTree::ChanceSearchNode::GetNumPlayerNodeChildren() const
    {
        try
        {
            const Byte* mem_ptr = reinterpret_cast<const Byte*>(this);
            return GetValFromMem<NumChildrenInt>(mem_ptr);
        }
        catch (std::out_of_range &e)
        {
            std::cerr << "Out of Range Error when attempting to"
                         " retrieve Number of Player Node Children for a Chance Node";
            std::cerr << e.what() << std::endl;
        }
    }

    SearchTree::SearchTreeNode::NumChildrenInt SearchTree::ChanceSearchNode::GetNumChanceNodeChildren() const
    {
        try
        {
            const Byte* mem_ptr = reinterpret_cast<const Byte*>(this);
            return GetValFromMem<NumChildrenInt>(mem_ptr + sizeof(uint8_t));
        }
        catch (std::out_of_range &e)
        {
            std::cerr << "Out of Range Error when attempting to"
                         " retrieve Number of Chance Node Children for a Chance Node";
            std::cerr << e.what() << std::endl;
        }
    }

    SearchTree::SearchTreeNode::NumChildrenInt SearchTree::ChanceSearchNode::GetNumTerminalNodeChildren() const
    {
        try
        {
            const Byte* mem_ptr = reinterpret_cast<const Byte*>(this);
            return GetValFromMem<NumChildrenInt>(mem_ptr + 2 * sizeof(uint8_t));
        }
        catch (std::out_of_range &e)
        {
            std::cerr << "Out of Range Error when attempting to"
                         " retrieve Number of Terminal Node Children for a Chance Node";
            std::cerr << e.what() << std::endl;
        }
    }

    SearchTree::SearchTreeNode::ChildNodePtr SearchTree::ChanceSearchNode::GetFirstChildAddr() const
    {
        try
        {
            const Byte* mem_ptr = reinterpret_cast<const Byte*>(this);
            return GetValFromMem<ChildNodePtr>(mem_ptr + 3 * sizeof(uint8_t));
        }
        catch (std::out_of_range &e)
        {
            std::cerr << "Out of Range Error when attempting to"
                        " retrieve First Child Address for a Chance Node";
            std::cerr << e.what() << std::endl;
        }
    }

    std::vector<NormalizedProbabilityInt> SearchTree::ChanceSearchNode::GetNormalizedProbabilities() const
    {
        std::vector<NormalizedProbabilityInt> probabilities;
        const int total_num_children = GetTotalNumChildren();
        const Byte* mem_ptr = reinterpret_cast<const Byte*>(this);
        for (int i = 0; i < total_num_children; i++)
        {
            try
            {
                uint8_t prob = GetValFromMem<NormalizedProbabilityInt>(mem_ptr + i * sizeof(NormalizedProbabilityInt));
                probabilities.push_back(prob);
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                        " retrieve Normalized Probability for a Chance Node";
                std::cerr << e.what() << std::endl;
            }
        }
        return probabilities;
    }

    //TODO: Implement GetRandomChildPtr() using RNG
    SearchTree::SearchTreeNode::ChildNodePtr SearchTree::ChanceSearchNode::GetRandomChildPtr() const
    {
        //TODO: Change return value to get random child pointer from probability distribution.
        return GetFirstChildAddr();
    }

}
