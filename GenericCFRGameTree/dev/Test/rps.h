//
// Created by Amir on 10/27/2024.
//

#pragma once
#ifndef RPS_H
#define RPS_H

#endif //RPS_H
#include "cfr.h"

class RPS_Test {
public:

    enum ActionType {Rock, Paper, Scissors, None};
    class RPS_Action : public CFR::ClientGame::Action
    {
    public:

        ActionType action_;
        RPS_Action() : action_(None) {}
        RPS_Action(const ActionType& action) : action_(action) {}
        ~RPS_Action() override {}
    };

    class RPS_Player : public CFR::ClientGame::GameState
    {
    public:
        ActionType player_one_action_;
        ActionType player_two_action_;
        RPS_Player() : GameState(true, false)
        {
            player_one_action_ = None;
            player_two_action_ = None;
        }
        explicit RPS_Player(const ActionType& player_one_action) : GameState(true, false)
        {
            player_one_action_ = player_one_action;
            player_two_action_ = None;
        }
        RPS_Player(const ActionType& player_one_action, const ActionType& player_two_action)
        : GameState(false, false)
        {
            player_one_action_ = player_one_action;
            player_two_action_ = player_two_action;
        }

        uint8_t PlayerIndex() const override
        {
            if (player_one_action_ == None)
            {
                return 0;
            }
            return 1;
        }

        std::vector<std::shared_ptr<const CFR::ClientGame::Action>> PlayerActions() const override
        {
            const auto rock_ptr = std::make_shared<RPS_Action>(Rock);
            const auto paper_ptr = std::make_shared<RPS_Action>(Paper);
            const auto scissors_ptr = std::make_shared<RPS_Action>(Scissors);


            const std::shared_ptr<CFR::ClientGame::Action> rock = std::static_pointer_cast<CFR::ClientGame::Action>(rock_ptr);
            const std::shared_ptr<CFR::ClientGame::Action> paper = std::static_pointer_cast<CFR::ClientGame::Action>(paper_ptr);
            const std::shared_ptr<CFR::ClientGame::Action> scissors = std::static_pointer_cast<CFR::ClientGame::Action>(scissors_ptr);

            std::vector<std::shared_ptr<const CFR::ClientGame::Action>> actions;
            actions.push_back(rock);
            actions.push_back(paper);
            actions.push_back(scissors);

            return actions;
        }

        std::vector<std::pair<std::shared_ptr<const CFR::ClientGame::Action>, float>> ChanceActions() const override
        {
            return std::vector<std::pair<std::shared_ptr<const CFR::ClientGame::Action>, float>>();
        }

        std::shared_ptr<const GameState> ChildGameState(const CFR::ClientGame::Action &a) const override
        {
            const RPS_Action& action = dynamic_cast<const RPS_Action&>(a);
            if (player_one_action_ == None)
            {
                return std::make_shared<RPS_Player>(action.action_);
            }
            if (player_two_action_ == None)
            {
                return std::make_shared<RPS_Player>(player_one_action_, action.action_);
            }
            return std::make_shared<RPS_Player>();
        }

        float TerminalUtility(uint64_t player_index) const override
        {
            int multiplier = player_index == 0 ? 1 : -1;
            if (player_one_action_ == Rock && player_two_action_ == Paper)
            {
                return multiplier * -1;
            }
            if (player_one_action_ == Rock && player_two_action_ == Scissors)
            {
                return multiplier * 1;
            }
            if (player_one_action_ == Paper && player_two_action_ == Rock)
            {
                return multiplier * 1;
            }
            if (player_one_action_ == Paper && player_two_action_ == Scissors)
            {
                return multiplier * -1;
            }
            if (player_one_action_ == Scissors && player_two_action_ == Rock)
            {
                return multiplier * -1;
            }
            if (player_one_action_ == Scissors && player_two_action_ == Paper)
            {
                return multiplier * 1;
            }
            return 0;
        }

        std::string PlayerViewHash() const override
        {
            if (player_two_action_ == None)
            {
                return std::to_string(player_one_action_);
            }
            return std::to_string(player_two_action_);
        }
    };
};
