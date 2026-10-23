#include "mspa/psychology/mood.hpp"

#include <algorithm>

namespace mspa {

void MoodSystem::update(AgentPsychState& agent, float valence_delta, float activation_delta, PsychTime now) const noexcept {
    agent.mood.valence = std::clamp(agent.mood.valence + valence_delta, -1.0f, 1.0f);
    agent.mood.activation = std::clamp(agent.mood.activation + activation_delta, 0.0f, 1.0f);
    agent.mood.updated_at = now;
}

void MoodSystem::decay(AgentPsychState& agent, PsychTime now, float rate_per_tick) const noexcept {
    if (now < agent.mood.updated_at) return;
    const float amount = static_cast<float>(now.ticks() - agent.mood.updated_at.ticks()) * std::max(0.0f, rate_per_tick);
    if (agent.mood.valence > 0.0f) agent.mood.valence = std::max(0.0f, agent.mood.valence - amount);
    else agent.mood.valence = std::min(0.0f, agent.mood.valence + amount);
    agent.mood.activation = std::max(0.0f, agent.mood.activation - amount);
    agent.mood.updated_at = now;
}

void MoodSystem::reset(AgentPsychState& agent, PsychTime now) const noexcept {
    agent.mood = MoodState{};
    agent.mood.updated_at = now;
}

} // namespace mspa
