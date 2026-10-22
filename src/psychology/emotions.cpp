#include "mspa/psychology/emotions.hpp"

#include <algorithm>

namespace mspa {

void EmotionSystem::apply(AgentPsychState& agent, const AppraisalVector& appraisal, PsychTime now) const {
    if (appraisal.relevance <= 0.0f) return;
    EmotionState emotion{};
    emotion.kind = ConceptId(appraisal.consequence >= 0.0f ? 1 : 2);
    emotion.intensity = std::clamp(appraisal.relevance * (0.5f + 0.5f * std::abs(appraisal.consequence)), 0.0f, 1.0f);
    emotion.started_at = now;
    agent.emotions.push_back(emotion);
}

void EmotionSystem::decay(AgentPsychState& agent, PsychTime now) const {
    for (auto& emotion : agent.emotions) {
        if (now >= emotion.started_at) {
            const auto elapsed = now.ticks() - emotion.started_at.ticks();
            const float decay = static_cast<float>(elapsed) * 0.0001f;
            emotion.intensity = std::max(0.0f, emotion.intensity - decay);
        }
    }
    agent.emotions.erase(
        std::remove_if(agent.emotions.begin(), agent.emotions.end(), [](const EmotionState& e) { return e.intensity <= 0.0f; }),
        agent.emotions.end());
}

} // namespace mspa
