//
// Created by Amir on 10/23/2024.
//
#pragma once
#ifndef FLOAT_NORMALIZATION_H
#define FLOAT_NORMALIZATION_H
#endif //FLOAT_NORMALIZATION_H
#include <random>
#include <vector>
#include "internal.h"

namespace Internal
{
    namespace Trees
    {
        /**
        * @brief Convert probability floating point value to normalized ChanceProbabilityInt value
        * @param probability Floating point probability value
        * @return vector of normalized ChanceProbabilityInt values
        */
        NormalizedProbabilityInt normalize_probability(float probability);

        /**
         * @brief Convert probability floating point values to normalized ChanceProbabilityInt values
         * @param probabilities Vector of floating point probability values
         * @return vector of normalized ChanceProbabilityInt values
         */
        std::vector<NormalizedProbabilityInt> normalize_probabilities(const std::vector<float>& probabilities);

        /**
         * @brief Convert utility floating point values to normalized TerminalUtilityInt values
         * @param utility_val
         * @param max_utility
         * @return TerminalUtilityInt value of utility, normalized to max_utility.
         */
        TerminalUtilityInt normalize_utility(const float utility_val, const float max_utility);
    }
}

