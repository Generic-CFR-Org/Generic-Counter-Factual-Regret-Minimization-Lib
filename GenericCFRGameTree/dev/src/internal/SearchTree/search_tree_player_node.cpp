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
        uint64_t PlayerSearchNode::SearchTreeNodeSize() const
        {
            return kNumChildrenMemSize * 3 + kInfoSetPtrMemSize + kInfoSetPtrMemSize;
        }

        NumChildrenInt PlayerSearchNode::GetNumPlayerNodeChildren() const
        {
            try
            {
                const Byte* mem_ptr = reinterpret_cast<const Byte*>(this);
                return GetValFromMem<NumChildrenInt>(mem_ptr);
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                             " retrieve Number of Player Node Children for a Player Node";
                std::cerr << e.what() << std::endl;
                throw;
            }
        }

        NumChildrenInt PlayerSearchNode::GetNumChanceNodeChildren() const
        {
            try
            {
                const Byte* mem_ptr = reinterpret_cast<const Byte*>(this);
                return GetValFromMem<NumChildrenInt>(mem_ptr + sizeof(uint8_t));
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                             " retrieve Number of Chance Node Children for a Player Node";
                std::cerr << e.what() << std::endl;
                throw;
            }
        }

        NumChildrenInt PlayerSearchNode::GetNumTerminalNodeChildren() const
        {
            try
            {
                const Byte* mem_ptr = reinterpret_cast<const Byte*>(this);
                return GetValFromMem<NumChildrenInt>(mem_ptr + 2 * sizeof(uint8_t));
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                             " retrieve Number of Terminal Node Children for a Player Node";
                std::cerr << e.what() << std::endl;
                throw;
            }
        }

        SearchTreeNode::ChildNodePtr PlayerSearchNode::GetFirstChildAddr() const
        {
            try
            {
                const Byte* mem_ptr = reinterpret_cast<const Byte*>(this);
                return GetValFromMem<ChildNodePtr>(mem_ptr + 3 * sizeof(uint8_t));
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                            " retrieve First Child Address for a Player Node";
                std::cerr << e.what() << std::endl;
                throw;
            }
        }

        SearchTreeNode::InfoSetPtr PlayerSearchNode::GetInfoSetPtr() const
        {
            try
            {
                const Byte* mem_ptr = reinterpret_cast<const Byte*>(this);
                return GetValFromMem<InfoSetPtr>(mem_ptr + 3 * kNumChildrenMemSize + kChildPtrMemSize);
            }
            catch (std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                            " retrieve Info Set Address for a Player Node";
                std::cerr << e.what() << std::endl;
                throw;
            }
        }
    }
}
