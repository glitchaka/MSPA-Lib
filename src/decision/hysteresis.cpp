#include "mspa/decision/hysteresis.hpp"

#include <algorithm>

namespace mspa {

float HysteresisPolicy::continuity_bonus(const AgentPsychState& agent,
                                         const ActionOption& option,
                                         ActionContinuityKey current) const noexcept {
    if (!current.valid() || option.continuity != current) return 0.0f;
    const float persistence = std::clamp(agent.personality.persistence, 0.0f, 1.0f);
    const float progress = std::clamp(option.progress, 0.0f, 1.0f);
    return 0.25f * persistence + 0.25f * progress;
}

} // namespace mspa
