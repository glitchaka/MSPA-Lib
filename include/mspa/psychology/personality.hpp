#pragma once

#include "mspa/base/result.hpp"
#include "mspa/model/agent_state.hpp"

namespace mspa {

struct PersonalityDerived {
    float belief_rigidity{0.0f};
    float goal_persistence{0.0f};
    float switch_resistance{0.0f};
};

class PersonalitySystem {
public:
    Result<void> normalize(Personality& personality) const;
    [[nodiscard]] PersonalityDerived derive(const Personality& personality) const noexcept;
};

} // namespace mspa
