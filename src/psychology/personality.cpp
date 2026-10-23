#include "mspa/psychology/personality.hpp"

#include <algorithm>

namespace mspa {

Result<void> PersonalitySystem::normalize(Personality& p) const {
    p.sociability = std::clamp(p.sociability, 0.0f, 1.0f);
    p.stability = std::clamp(p.stability, 0.0f, 1.0f);
    p.empathy = std::clamp(p.empathy, 0.0f, 1.0f);
    p.persistence = std::clamp(p.persistence, 0.0f, 1.0f);
    p.curiosity = std::clamp(p.curiosity, 0.0f, 1.0f);
    p.boldness = std::clamp(p.boldness, 0.0f, 1.0f);
    return {};
}

PersonalityDerived PersonalitySystem::derive(const Personality& p) const noexcept {
    PersonalityDerived result{};
    result.belief_rigidity = std::clamp(0.65f * p.persistence + 0.35f * (1.0f - p.curiosity), 0.0f, 1.0f);
    result.goal_persistence = std::clamp(0.8f * p.persistence + 0.2f * p.stability, 0.0f, 1.0f);
    result.switch_resistance = std::clamp(0.7f * p.persistence + 0.3f * p.stability, 0.0f, 1.0f);
    return result;
}

} // namespace mspa
