//
// Created by Amir on 10/28/2024.
//

#pragma once
#ifndef INTERNAL_H
#define INTERNAL_H
#include "external.h"
#include <limits>

#endif //INTERNAL_H

namespace Internal
{
    using Byte = CFR::Byte;
    using GameState = CFR::ClientGame::GameState;
    using Action = CFR::ClientGame::Action;
    using Game = CFR::ClientGame::Game;

    namespace Trees
    {
        using NormalizedProbabilityInt = uint8_t;
        using TerminalUtilityInt = uint8_t;

        using NumChildrenInt = uint8_t;

        using NumActionsInt = uint16_t;
        using CumulativeProbInt = uint32_t;
        using CumulativeRegretInt = uint32_t;
        using PlayerInt = uint8_t;

        constexpr NormalizedProbabilityInt kMaxChanceProbabilityVal = std::numeric_limits<NormalizedProbabilityInt>::max();
        constexpr TerminalUtilityInt kMaxTerminalUtilityVal = std::numeric_limits<TerminalUtilityInt>::max();

        constexpr uint8_t kNumPlayers = CFR::ClientGame::kNumPlayers;

        /**
        * @brief Size of a chance node's 'probability to reach child' in memory
        */
        static constexpr uint8_t kProbabilityMemSize = sizeof(NormalizedProbabilityInt);
        /**
         * @brief Size of a terminal node's utility in memory
         */
        static constexpr uint8_t kTerminalUtilitySize = sizeof(TerminalUtilityInt);

        /**
        * @brief Size of a node's 'number of children of some type' in memory
        */
        static constexpr uint8_t kNumChildrenMemSize = sizeof(NumChildrenInt);

        static constexpr uint8_t kNumChildrenSizeInMem = sizeof(NumActionsInt);
        static constexpr uint8_t kCurrActionProbSizeInMem = sizeof(NormalizedProbabilityInt);
        static constexpr uint8_t kCumulativeActionProbSizeInMem = sizeof(CumulativeProbInt);
        static constexpr uint8_t kCumulativeActionRegretSizeInMem = sizeof(CumulativeRegretInt);
        static constexpr uint8_t kPlayerSizeInMem = sizeof(PlayerInt);

    }
}
