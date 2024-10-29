//
// Created by Amir on 10/27/2024.
//

#include "client_game.h"
#include <algorithm>
#include <iostream>

namespace CFR
{
    namespace ClientGame
    {
        bool GameState::CompareByNodeType(const std::shared_ptr<const GameState> &a, const std::shared_ptr<const GameState> &b)
        {
            if (IsPlayerNode(a) && IsPlayerNode(b))
            {
                return false;
            }
            if (IsPlayerNode(a))
            {
                return true;
            }
            if (IsPlayerNode(b))
            {
                return false;
            }
            if (IsChanceNode(a) && IsChanceNode(b))
            {
                return false;
            }
            if (IsChanceNode(a))
            {
                return true;
            }
            if (IsChanceNode(b))
            {
                return false;
            }
            return false;
        }

        bool GameState::CompareByChildNodeType(
            const std::pair<std::shared_ptr<const GameState>, float> &a,
            const std::pair<std::shared_ptr<const GameState>, float> &b)
        {
            if (IsPlayerNode(a.first) && IsPlayerNode(b.first))
            {
                return false;
            }
            if (IsPlayerNode(a.first))
            {
                return true;
            }
            if (IsPlayerNode(b.first))
            {
                return false;
            }
            if (IsChanceNode(a.first) && IsChanceNode(b.first))
            {
                return false;
            }
            if (IsChanceNode(a.first))
            {
                return true;
            }
            if (IsChanceNode(b.first))
            {
                return false;
            }
            return false;
        }

        std::vector<std::shared_ptr<const GameState>> GameState::AllChildrenSorted() const
        {

            if (IsTerminalNode())
            {
                return std::vector<std::shared_ptr<const GameState>>();
            }
            std::vector<std::shared_ptr<const GameState>> ret;
            if (IsPlayerNode())
            {

                for (std::shared_ptr<const Action> a : PlayerActions())
                {
                    std::shared_ptr<const GameState> child = ChildGameState(*a);
                    ret.push_back(child);
                }
            }
            else if (IsChanceNode())
            {
                for (std::pair<std::shared_ptr<const Action>, float>& p : ChanceActions())
                {
                    std::shared_ptr<const GameState> child = ChildGameState(*p.first);
                    ret.push_back(child);
                }
            }
            std::sort(ret.begin(), ret.end(), CompareByNodeType);
            return ret;
        }

        ProbabilityList GameState::ChanceProbabilityList() const
        {
            if (!IsChanceNode())
            {
                return ProbabilityList();
            }
            ProbabilityList ret;
            for (std::pair<std::shared_ptr<const Action>, float>& p : ChanceActions())
            {
                ret.push_back(p.second);
            }
            return ret;
        }

        ProbabilityList GameState::SortedChanceProbabilityList() const
        {
            if (!IsChanceNode())
            {
                return ProbabilityList();
            }
            std::vector<std::pair<std::shared_ptr<const GameState>, float>> children_with_prob;
            ProbabilityList ret;
            for (std::pair<std::shared_ptr<const Action>, float> p : ChanceActions())
            {
                std::pair<std::shared_ptr<const GameState>, float> to_sort(ChildGameState(*p.first), p.second);
                children_with_prob.push_back(to_sort);
            }
            std::sort(children_with_prob.begin(), children_with_prob.end(), CompareByChildNodeType);
            for (std::pair<std::shared_ptr<const GameState>, float>& p : children_with_prob)
            {
                ret.push_back(p.second);
            }
            return ret;
        }

        uint64_t GameState::TotalNumChildren() const
        {
            if (IsPlayerNode())
            {
                return PlayerActions().size();
            }
            if (IsChanceNode())
            {
                return ChanceActions().size();
            }
            return 0;
        }

        GameStateTypeCnt GameState::NumChildrenOfEachType() const
        {
            if (IsTerminalNode())
            {
                return GameStateTypeCnt{0, 0, 0};
            }
            uint8_t player_node_cnt = 0;
            uint8_t chance_node_cnt = 0;
            uint8_t terminal_node_cnt = 0;
            if (IsPlayerNode())
            {
                for (std::shared_ptr<const Action> a : PlayerActions())
                {
                    const std::shared_ptr<const GameState> child = ChildGameState(*a);
                    if (child->IsPlayerNode()) { player_node_cnt++; }
                    else if (child->IsChanceNode()) { chance_node_cnt++; }
                    else {terminal_node_cnt++; }
                }
            }
            else if (IsChanceNode())
            {
                for (std::pair<std::shared_ptr<const Action>, float>& p : ChanceActions())
                {
                    const std::shared_ptr<const GameState> child = ChildGameState(*p.first);
                    if (child->IsPlayerNode()) { player_node_cnt++; }
                    else if (child->IsChanceNode()) { chance_node_cnt++; }
                    else {terminal_node_cnt++; }
                }
            }
            return GameStateTypeCnt(player_node_cnt, chance_node_cnt, terminal_node_cnt);
        }
    }
}
