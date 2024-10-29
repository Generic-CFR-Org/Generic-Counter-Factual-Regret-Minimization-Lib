//
// Created by Amir on 10/26/2024.
//

#include <stdexcept>
#include "search_tree.h"
#include "tree_node_utils.h"
#include <iostream>
#include <string>

namespace Internal
{
    namespace Trees
    {
        //TODO: Generalize to N number of players
        uint64_t TerminalSearchNode::SearchTreeNodeSize() const { return kTerminalUtilitySize * kNumPlayers; }

        NumChildrenInt TerminalSearchNode::GetNumPlayerNodeChildren() const { return 0; }

        NumChildrenInt TerminalSearchNode::GetNumChanceNodeChildren() const { return 0; }

        NumChildrenInt TerminalSearchNode::GetNumTerminalNodeChildren() const { return 0; }

        SearchTreeNode::ChildNodePtr TerminalSearchNode::GetFirstChildAddr() const { return nullptr; }

        //TODO: Generalize to N number of players
        TerminalUtilityInt TerminalSearchNode::GetNormalizedUtility(const uint64_t player_index) const
        {
            if (player_index >= kNumPlayers)
            {
                std::string error_msg = "Cannot get utility for player #" + (player_index + 1);
                error_msg += " there are only " + kNumPlayers;
                error_msg += " players.";
                throw std::invalid_argument(error_msg);
            }
            try
            {
                const Byte* mem_ptr = reinterpret_cast<const Byte*>(this);
                return GetValFromMem<TerminalUtilityInt>(mem_ptr + player_index);
            }
            catch (const std::out_of_range &e)
            {
                std::cerr << "Out of Range Error when attempting to"
                " retrieve Normalized Utility for a Terminal Node";
                std::cerr << e.what() << std::endl;
                throw;
            }
        }
    }
}
