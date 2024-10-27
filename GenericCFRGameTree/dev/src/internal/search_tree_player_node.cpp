//
// Created by Amir on 10/26/2024.
//

#include <stdexcept>
#include "search_tree_node.h"
#include "tree_node_utils.h"
#include <iostream>

namespace generic_cfr
{

    uint64_t SearchTree::PlayerSearchNode::SearchTreeNodeSize() const
    {
        return kNumChildrenMemSize * 3 + kInfoSetPtrMemSize + kInfoSetPtrMemSize;
    }

    SearchTree::SearchTreeNode::NumChildrenInt SearchTree::PlayerSearchNode::GetNumPlayerNodeChildren() const
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
        }
    }

    SearchTree::SearchTreeNode::NumChildrenInt SearchTree::PlayerSearchNode::GetNumChanceNodeChildren() const
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
        }
    }

    SearchTree::SearchTreeNode::NumChildrenInt SearchTree::PlayerSearchNode::GetNumTerminalNodeChildren() const
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
        }
    }

    SearchTree::SearchTreeNode::ChildNodePtr SearchTree::PlayerSearchNode::GetFirstChildAddr() const
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
        }
    }

    SearchTree::SearchTreeNode::InfoSetPtr SearchTree::PlayerSearchNode::GetInfoSetPtr() const
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
        }
    }
}
