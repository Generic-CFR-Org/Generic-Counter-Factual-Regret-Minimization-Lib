//
// Created by Amir on 10/28/2024.
//

#include "float_normalization.h"

namespace Internal
{
    namespace Trees
    {

        NormalizedProbabilityInt normalize_probability(float probability)
        {

            uint8_t normalized_prob = probability * kMaxChanceProbabilityVal;
            return normalized_prob;
        }

        std::vector<NormalizedProbabilityInt> normalize_probabilities(const std::vector<float>& probabilities)
        {
            std::vector<uint8_t> result;
            for (const float probability : probabilities)
            {
                result.push_back(normalize_probability(probability));
            }
            return result;
        }

        TerminalUtilityInt normalize_utility(const float utility_val, const float max_utility)
        {
            const uint8_t normalized_utility = static_cast<uint8_t>(kMaxTerminalUtilityVal * utility_val / max_utility);
            return normalized_utility;
        }
    }
}
