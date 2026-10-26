#include "mspa/decision/hysteresis.hpp"
#include "mspa/psychology/personality.hpp"
#include <algorithm>

namespace mspa {
namespace {
constexpr float max_total_inertia = 0.35f;

struct InertiaParts {
    float continuity{0.0f};
    float guard{0.0f};
};

InertiaParts inertia_parts(const AgentPsychState& agent, const ActionOption& option) noexcept {
    const auto personality = PersonalitySystem{}.derive(agent.personality);
    const float continuity_full =
        0.2f * personality.switch_resistance +
        0.25f * std::clamp(option.progress, 0.0f, 1.0f) +
        0.25f * std::clamp(option.interruption_cost, 0.0f, 1.0f) +
        0.15f * std::clamp(option.context_stability, 0.0f, 1.0f);
    const float guard_full =
        0.05f +
        0.25f * personality.switch_resistance +
        0.2f * std::clamp(option.interruption_cost, 0.0f, 1.0f) +
        0.15f * std::clamp(option.progress, 0.0f, 1.0f);

    InertiaParts parts{0.5f * continuity_full, 0.5f * guard_full};
    const float total = parts.continuity + parts.guard;
    if (total > max_total_inertia) {
        const float scale = max_total_inertia / total;
        parts.continuity *= scale;
        parts.guard *= scale;
    }
    return parts;
}

float urgency_scale(float urgency) noexcept {
    return 1.0f - std::clamp(urgency, 0.0f, 1.0f);
}
} // namespace

float HysteresisPolicy::continuity_bonus(const AgentPsychState& agent, const ActionOption& option,
                                         ActionContinuityKey current, float urgency) const noexcept {
    if (!current.valid() || option.continuity != current) return 0.0f;
    return inertia_parts(agent, option).continuity * urgency_scale(urgency);
}

float HysteresisPolicy::switch_threshold(const AgentPsychState& agent, const ActionOption& option,
                                         float urgency) const noexcept {
    return inertia_parts(agent, option).guard * urgency_scale(urgency);
}
} // namespace mspa
