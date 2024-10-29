//
// Created by Amir on 10/26/2024.
//

#include <stdexcept>

#include "search_tree.h"
#include "tree_node_utils.h"
#include <iostream>

namespace Internal
{
    namespace Trees
    {
        uint64_t ChanceSearchNode::SearchTreeNodeSize() const
        {
            return kNumChildrenMemSize * 3 + kChildPtrMemSize + kProbabilityMemSize * GetTotalNumChildren();
        }

        NumChildrenInt ChanceSearchNode::GetNumPlayerNodeChildren() const
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
                throw;
            }
        }

        NumChildrenInt ChanceSearchNode::GetNumChanceNodeChildren() const
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
                throw;
            }
        }

        NumChildrenInt ChanceSearchNode::GetNumTerminalNodeChildren() const
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
                throw;
            }
        }

        SearchTreeNode::ChildNodePtr ChanceSearchNode::GetFirstChildAddr() const
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
                throw;
            }
        }

        std::vector<NormalizedProbabilityInt> ChanceSearchNode::GetNormalizedProbabilities() const
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
        uint16_t ChanceSearchNode::GetRandomChildIndex() const
        {
            //TODO: Change return value to get random child pointer from probability distribution.
            NormalizedProbabilityInt random = rand() % kMaxChanceProbabilityVal;
            std::vector<SearchTreeNode*> children = GetChildren();
            std::vector<NormalizedProbabilityInt> probabilities = GetNormalizedProbabilities();
            auto upper = std::upper_bound(probabilities.begin(), probabilities.end(), random);
            unsigned int index = std::distance(probabilities.begin(), upper);
            return index;
        }
    }
}
