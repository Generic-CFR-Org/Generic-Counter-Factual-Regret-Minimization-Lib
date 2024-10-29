//
// Created by Amir on 9/7/2024.
//
#pragma once
#ifndef GAME_CLASSES_H
#define GAME_CLASSES_H
#include <memory>

#endif //GAME_CLASSES_H

#include <vector>
#include <string>
#include <tuple>


namespace CFR {

    namespace ClientGame
    {
        constexpr uint8_t kNumPlayers = 2;
        using GameStateTypeCnt = std::tuple<uint8_t, uint8_t, uint8_t>;
        using ProbabilityList = std::vector<float>;

        class Action
        {
        public:
            virtual ~Action() = default;
        };

        class GameState {
        protected:
            bool is_player_node_;
            bool is_chance_node_;
            bool is_player_one_;

        public:
            virtual ~GameState() = default;

            GameState() :
                is_player_node_(false), is_chance_node_(false),
                is_player_one_(false) {}

            GameState(const bool is_player_node, const bool is_chance_node) :
                is_player_node_(is_player_node), is_chance_node_(is_chance_node),
                is_player_one_(false) {}

            GameState( const bool is_player_node, const bool is_chance_node,
                const bool is_player_one) :
                is_player_node_(is_player_node), is_chance_node_(is_chance_node),
                is_player_one_(is_player_one) {}

            bool IsPlayerNode() const { return is_player_node_; }
            bool IsChanceNode() const { return is_chance_node_; }
            bool IsTerminalNode() const { return !is_player_node_ && !is_chance_node_; }

            static bool IsPlayerNode(const std::shared_ptr<const GameState> &p) { return p->IsPlayerNode(); }
            static bool IsChanceNode(const std::shared_ptr<const GameState> &c) { return c->IsChanceNode(); }
            static bool IsTerminalNode(const std::shared_ptr<const GameState> &t) { return t->IsTerminalNode(); }

            static bool CompareByNodeType(const std::shared_ptr<const GameState> &a, const std::shared_ptr<const GameState> &b);

            static bool GameState::CompareByChildNodeType(
                const std::pair<std::shared_ptr<const GameState>, float> &a,
                const std::pair<std::shared_ptr<const GameState>, float> &b);

            std::vector<std::shared_ptr<const GameState>> AllChildrenSorted() const;
            ProbabilityList ChanceProbabilityList() const;
            ProbabilityList SortedChanceProbabilityList() const;

            GameStateTypeCnt NumChildrenOfEachType() const;
            uint64_t TotalNumChildren() const;

            virtual std::vector<std::shared_ptr<const Action>> PlayerActions() const = 0;
            virtual std::vector<std::pair<std::shared_ptr<const Action>, float>> ChanceActions() const = 0;
            virtual std::shared_ptr<const GameState> ChildGameState(const Action& a) const = 0;
            virtual float TerminalUtility(uint64_t player_index) const = 0;
            virtual uint8_t PlayerIndex() const = 0;

            /*virtual std::string GameStateHash() const = 0;*/
            virtual std::string PlayerViewHash() const = 0;

            /*virtual bool operator==(const GameState &ref) const = 0;*/
        };

        class Game {
        public:
            Game() : root_game_state_(nullptr) {}
            explicit Game(GameState* root_game_state) : root_game_state_(root_game_state) {}
            std::shared_ptr<GameState> RootGameState() const { return root_game_state_; }

        private:
            std::shared_ptr<GameState> root_game_state_;
        };
    }
}