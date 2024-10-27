//
// Created by Amir on 10/23/2024.
//
#pragma once
#ifndef FLOAT_NORMALIZATION_H
#define FLOAT_NORMALIZATION_H
#include "search_tree_node.h"
#include <random>
#endif //FLOAT_NORMALIZATION_H

#include <vector>

namespace generic_cfr
{
    /**
    * @brief Convert probability floating point value to normalized ChanceProbabilityInt value
    * @param probability Floating point probability value
    * @return vector of normalized ChanceProbabilityInt values
    */
    inline NormalizedProbabilityInt normalize_probability(float probability)
    {

        uint8_t normalized_prob = probability * kMaxChanceProbabilityVal;
        return normalized_prob;
    }

    /**
     * @brief Convert probability floating point values to normalized ChanceProbabilityInt values
     * @param probabilities Vector of floating point probability values
     * @return vector of normalized ChanceProbabilityInt values
     */
    inline std::vector<NormalizedProbabilityInt> normalize_probabilities(const std::vector<float>& probabilities)
    {
        std::vector<uint8_t> result;
        for (const float probability : probabilities)
        {
            result.push_back(normalize_probability(probability));
        }
        return result;
    }

    /**
     * @brief Convert utility floating point values to normalized TerminalUtilityInt values
     * @param utility_val
     * @param max_utility
     * @return TerminalUtilityInt value of utility, normalized to max_utility.
     */
    inline TerminalUtilityInt normalize_utility(const float utility_val, const float max_utility)
    {
        const uint8_t normalized_utility = static_cast<uint8_t>(kMaxTerminalUtilityVal * utility_val / max_utility);
        return normalized_utility;
    }
}

