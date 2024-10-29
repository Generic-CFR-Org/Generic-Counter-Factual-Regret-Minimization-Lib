//
// Created by Amir on 10/26/2024.
//

#include "info_set_tree.h"
#include <iostream>

namespace Internal
{
    namespace Trees
    {
        InfoSetTree::InfoSetTree()
        {
            info_set_tree_size_ = 0;
            bytes_set_ = 0;
            info_set_tree_ = nullptr;
        }

        Byte *InfoSetTree::InfoSetPtr() const
        {
            return info_set_tree_;
        }

        void InfoSetTree::AddInfoSetToTree(const std::string& info_set_hash, const uint16_t num_actions, PlayerInt player)
        {
            if (info_set_offset_map_.find(info_set_hash) == info_set_offset_map_.end())
            {
                info_set_hash_LIFO_.push_back(info_set_hash);
                info_set_size_map_[info_set_hash] = num_actions;
                info_set_player_map_[info_set_hash] = player;
                info_set_tree_size_ += InfoSetTreeNode::InfoSetTreeNodeSize(num_actions);
            }
        }

        Byte *InfoSetTree::InfoSetNodeAddress(const std::string& info_set_hash)
        {
            return info_set_tree_ + info_set_offset_map_[info_set_hash];
        }

        void InfoSetTree::ConstructInfoSetTree()
        {
            AllocateInfoSetMem();
            SetAllInfoSetsInMem();
        }

        unsigned long long InfoSetTree::Size() const
        {
            return info_set_tree_size_;
        }


        void InfoSetTree::AllocateInfoSetMem()
        {
            try
            {
                info_set_tree_ = new Byte[info_set_tree_size_];
            }
            catch (std::bad_alloc &e)
            {
                std::cerr << "Exception allocating memory for info set tree." << e.what() << std::endl;
            }
        }

        void InfoSetTree::SetInfoSetNodeInMem(uint16_t num_actions, PlayerInt player)
        {
            Byte* info_set_mem_ptr = info_set_tree_ + bytes_set_;
            InfoSetTreeNode::SetInfoSetNodeInMem(num_actions, info_set_mem_ptr, player);
            bytes_set_ += InfoSetTreeNode::InfoSetTreeNodeSize(num_actions);
        }
        void InfoSetTree::SetAllInfoSetsInMem()
        {
            for (std::string& info_set_hash : info_set_hash_LIFO_)
            {
                info_set_offset_map_[info_set_hash] = bytes_set_;

                SetInfoSetNodeInMem(info_set_size_map_[info_set_hash], info_set_player_map_[info_set_hash]);
            }
        }
    }
}
